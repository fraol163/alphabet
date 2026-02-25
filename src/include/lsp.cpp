#include "lsp.h"
#include <iostream>
#include <regex>

namespace alphabet {
namespace lsp {

LanguageServer::LanguageServer() {
    initialize_completions();
}

void LanguageServer::initialize_completions() {
    keywords_ = {
        {"i", "keyword", "if statement", "Conditional logic"},
        {"e", "keyword", "else statement", "Alternative path"},
        {"l", "keyword", "loop statement", "Repetition"},
        {"b", "keyword", "break", "Exit loop"},
        {"k", "keyword", "continue", "Next iteration"},
        {"r", "keyword", "return", "Return from function"},
        {"t", "keyword", "try", "Exception handling"},
        {"h", "keyword", "handle", "Catch exception"},
        {"c", "keyword", "class", "Define class"},
        {"a", "keyword", "abstract", "Abstract class"},
        {"j", "keyword", "interface", "Interface definition"},
        {"n", "keyword", "new", "Instantiate object"},
        {"s", "keyword", "static", "Static member"},
        {"v", "keyword", "public", "Public visibility"},
        {"p", "keyword", "private", "Private visibility"},
        {"m", "keyword", "method", "Function definition"},
        {"z", "keyword", "system", "System library"},
    };

    types_ = {
        {"1", "type", "i8", "8-bit integer"},
        {"2", "type", "i16", "16-bit integer"},
        {"3", "type", "i32", "32-bit integer"},
        {"4", "type", "i64", "64-bit integer"},
        {"5", "type", "int", "Generic integer"},
        {"6", "type", "f32", "32-bit float"},
        {"7", "type", "f64", "64-bit float"},
        {"8", "type", "float", "Generic float"},
        {"9", "type", "dec", "Decimal"},
        {"10", "type", "cpx", "Complex number"},
        {"11", "type", "bool", "Boolean"},
        {"12", "type", "str", "String"},
        {"13", "type", "list", "Array"},
        {"14", "type", "map", "Hash map"},
    };
}

std::string LanguageServer::process_message(const std::string& json) {
    std::string method = extract_json_string(json, "method");
    int id = extract_json_int(json, "id", -1);
    
    if (method == "initialize") {
        return handle_initialize(id, json);
    } else if (method == "textDocument/didOpen") {
        return handle_did_open(json);
    } else if (method == "textDocument/didChange") {
        return handle_did_change(json);
    } else if (method == "textDocument/completion") {
        return handle_completion(id, json);
    } else if (method == "textDocument/hover") {
        return handle_hover(id, json);
    } else if (method == "shutdown") {
        return create_response(id, "null");
    } else if (method == "exit") {
        return "";
    }
    
    return create_error(id, -32601, "Method not found: " + method);
}

std::vector<Diagnostic> LanguageServer::get_diagnostics(const std::string& /*uri*/, const std::string& content) {
    std::vector<Diagnostic> diagnostics;

    if (content.find("#alphabet<") == std::string::npos) {
        Diagnostic diag;
        diag.range.start.line = 0;
        diag.range.start.character = 0;
        diag.range.end.line = 0;
        diag.range.end.character = 0;
        diag.severity = 1;
        diag.message = "Missing magic header '#alphabet<lang>' on line 1";
        diag.code = "MISSING_HEADER";
        diagnostics.push_back(diag);
    }

    return diagnostics;
}

std::vector<CompletionItem> LanguageServer::get_completions(const std::string& /*uri*/, Position /*position*/) {
    std::vector<CompletionItem> items;
    items.insert(items.end(), keywords_.begin(), keywords_.end());
    items.insert(items.end(), types_.begin(), types_.end());
    return items;
}

void LanguageServer::run() {
    std::string line;
    std::string buffer;
    int content_length = 0;
    bool reading_headers = true;

    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (reading_headers) {
            if (line.empty()) {
                reading_headers = false;
                if (content_length > 0 && buffer.size() < static_cast<size_t>(content_length)) {
                    buffer.resize(content_length);
                    std::cin.read(&buffer[0], content_length - buffer.size());
                }
                std::string response = process_message(buffer);
                if (!response.empty()) {
                    send_response(response);
                }
                buffer.clear();
                content_length = 0;
            } else if (line.find("Content-Length:") == 0) {
                content_length = std::stoi(line.substr(17));
            }
        }
    }
}

std::string LanguageServer::create_response(int id, const std::string& result) {
    std::ostringstream oss;
    oss << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":" << result << "}";
    return oss.str();
}

std::string LanguageServer::create_error(int id, int code, const std::string& message) {
    std::ostringstream oss;
    oss << "{\"jsonrpc\":\"2.0\",\"id\":" << id
        << ",\"error\":{\"code\":" << code << ",\"message\":\"" << message << "\"}}";
    return oss.str();
}

void LanguageServer::send_response(const std::string& json) {
    std::cout << "Content-Length: " << json.size() << "\r\n\r\n" << json << std::flush;
}

std::string LanguageServer::handle_initialize(int id, const std::string& /*params*/) {
    std::ostringstream oss;
    oss << R"({
        "capabilities": {
            "textDocumentSync": 1,
            "completionProvider": {
                "resolveProvider": true,
                "triggerCharacters": [".", "(", " "]
            },
            "hoverProvider": true,
            "diagnosticProvider": {
                "interFileDependencies": false,
                "workspaceDiagnostics": false
            }
        },
        "serverInfo": {
            "name": "alphabet-lsp",
            "version": "2.0.0"
        }
    })";
    return create_response(id, oss.str());
}

std::string LanguageServer::handle_did_open(const std::string& params) {
    std::string uri = extract_json_string(params, "uri");
    std::string text = extract_json_string(params, "text");
    documents_[uri] = text;
    return "";
}

std::string LanguageServer::handle_did_change(const std::string& params) {
    std::string uri = extract_json_string(params, "uri");
    if (documents_.find(uri) != documents_.end()) {
        auto diagnostics = get_diagnostics(uri, documents_[uri]);
    }
    return "";
}

std::string LanguageServer::handle_completion(int id, const std::string& /*params*/) {
    auto items = get_completions("", Position{});
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (const auto& item : items) {
        if (!first) oss << ",";
        oss << R"({"label":")" << item.label << R"(","kind":")" << item.kind
            << R"(","detail":")" << item.detail << R"(","documentation":")" << item.documentation << R"("})";
        first = false;
    }
    oss << "]";
    return create_response(id, oss.str());
}

std::string LanguageServer::handle_hover(int id, const std::string& /*params*/) {
    std::ostringstream oss;
    oss << R"({"contents":{"kind":"markdown","value":"Alphabet Language Keyword"}})";
    return create_response(id, oss.str());
}

std::string extract_json_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";
    
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    
    return json.substr(pos + 1, end - pos - 1);
}

int extract_json_int(const std::string& json, const std::string& key, int default_val) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return default_val;
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) return default_val;
    
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    
    if (pos >= json.size() || (!std::isdigit(json[pos]) && json[pos] != '-')) {
        return default_val;
    }
    
    return std::stoi(json.substr(pos));
}

}
}
