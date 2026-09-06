#include "forge_lsp.h"
#include "peg_engine.h"
#include "header_engine.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace alphabet {
namespace forge {

namespace {

std::string escape_json_str(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

void send_lsp_response(const std::string& body) {
    std::cout << "Content-Length: " << body.length() << "\r\n\r\n" << body << std::flush;
}

} // namespace

void ForgeLanguageServer::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        // Strip carriage return
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.rfind("Content-Length:", 0) == 0) {
            size_t colon = line.find(':');
            if (colon == std::string::npos) continue;
            size_t length = std::stoul(line.substr(colon + 1));

            // Skip until empty line \r\n
            while (std::getline(std::cin, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (line.empty()) break;
            }

            // Read exact payload
            std::string payload;
            payload.resize(length);
            std::cin.read(&payload[0], length);

            // Handle basic messages
            if (payload.find("\"method\":\"initialize\"") != std::string::npos) {
                // Find id
                std::string id = "1";
                size_t id_pos = payload.find("\"id\":");
                if (id_pos != std::string::npos) {
                    size_t start = id_pos + 5;
                    size_t end = payload.find_first_of(",}", start);
                    id = payload.substr(start, end - start);
                }

                std::ostringstream resp;
                resp << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":{"
                     << "\"capabilities\":{"
                     << "\"textDocumentSync\":1,"
                     << "\"completionProvider\":{\"resolveProvider\":false}"
                     << "}}}";
                send_lsp_response(resp.str());
            } 
            else if (payload.find("\"method\":\"textDocument/didOpen\"") != std::string::npos ||
                     payload.find("\"method\":\"textDocument/didChange\"") != std::string::npos) {
                // Extract URI
                std::string uri;
                size_t uri_pos = payload.find("\"uri\":\"");
                if (uri_pos != std::string::npos) {
                    size_t start = uri_pos + 7;
                    size_t end = payload.find('"', start);
                    uri = payload.substr(start, end - start);
                }

                // Extract text
                std::string text;
                size_t text_pos = payload.find("\"text\":\"");
                if (text_pos != std::string::npos) {
                    size_t start = text_pos + 8;
                    size_t end = payload.find("\"}}", start);
                    if (end == std::string::npos) end = payload.find("\"}", start);
                    if (end != std::string::npos) {
                        text = payload.substr(start, end - start);
                    }
                }

                // Validate with PegEngine
                HeaderParseResult hdr = HeaderEngine::parse(text, spec_.header, spec_.name);
                std::ostringstream diag;
                diag << "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{"
                     << "\"uri\":\"" << uri << "\",\"diagnostics\":[";

                if (!hdr.valid) {
                    diag << "{\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":0,\"character\":5}},"
                         << "\"severity\":1,\"message\":\"" << escape_json_str(hdr.error) << "\"}";
                } else {
                    PegEngine peg(spec_);
                    ParseResult res = peg.parse(hdr.stripped_source, uri);
                    if (!res.success) {
                        for (size_t i = 0; i < res.errors.size(); ++i) {
                            if (i > 0) diag << ",";
                            int l = std::max(0, res.errors[i].line - 1);
                            int c = std::max(0, res.errors[i].column - 1);
                            diag << "{\"range\":{\"start\":{\"line\":" << l << ",\"character\":" << c << "},"
                                 << "\"end\":{\"line\":" << l << ",\"character\":" << (c + 1) << "}},"
                                 << "\"severity\":1,\"message\":\"" << escape_json_str(res.errors[i].message) << "\"}";
                        }
                    }
                }
                diag << "]}}";
                send_lsp_response(diag.str());
            }
            else if (payload.find("\"method\":\"textDocument/completion\"") != std::string::npos) {
                std::string id = "1";
                size_t id_pos = payload.find("\"id\":");
                if (id_pos != std::string::npos) {
                    size_t start = id_pos + 5;
                    size_t end = payload.find_first_of(",}", start);
                    id = payload.substr(start, end - start);
                }

                std::ostringstream resp;
                resp << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":{\"isIncomplete\":false,\"items\":[";
                size_t count = 0;
                for (const auto& kv : spec_.tokens.keywords) {
                    if (count > 0) resp << ",";
                    resp << "{\"label\":\"" << escape_json_str(kv.first) << "\","
                         << "\"kind\":14,"
                         << "\"detail\":\"" << spec_.name << " keyword (" << escape_json_str(kv.second) << ")\"}";
                    count++;
                }
                resp << "]}}";
                send_lsp_response(resp.str());
            }
            else if (payload.find("\"method\":\"shutdown\"") != std::string::npos) {
                std::string id = "1";
                size_t id_pos = payload.find("\"id\":");
                if (id_pos != std::string::npos) {
                    size_t start = id_pos + 5;
                    size_t end = payload.find_first_of(",}", start);
                    id = payload.substr(start, end - start);
                }
                send_lsp_response("{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"result\":null}");
            }
            else if (payload.find("\"method\":\"exit\"") != std::string::npos) {
                break;
            }
        }
    }
}

} // namespace forge
} // namespace alphabet
