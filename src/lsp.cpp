#include "lsp.h"
#include "lexer.h"
#include "parser.h"
#include "version.h"
#include <algorithm>
#include <cctype>

namespace alphabet {
namespace lsp {

static std::string escape_json(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\t':
            out += "\\t";
            break;
        case '\r':
            out += "\\r";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string JsonValue::dump() const {
    switch (type) {
    case NULL_T:
        return "null";
    case BOOL_T:
        return bool_val ? "true" : "false";
    case INT_T:
        return std::to_string(int_val);
    case STRING_T:
        return "\"" + escape_json(str_val) + "\"";
    case ARRAY_T: {
        std::string out = "[";
        for (size_t i = 0; i < arr_val.size(); ++i) {
            if (i > 0)
                out += ",";
            out += arr_val[i].dump();
        }
        return out + "]";
    }
    case OBJECT_T: {
        std::string out = "{";
        bool first = true;
        for (const auto& [k, v] : obj_val) {
            if (!first)
                out += ",";
            out += "\"" + escape_json(k) + "\":" + v.dump();
            first = false;
        }
        return out + "}";
    }
    }
    return "null";
}

static size_t skip_ws(const std::string& s, size_t pos) {
    while (pos < s.size() && std::isspace(s[pos]))
        ++pos;
    return pos;
}

static JsonValue parse_value(const std::string& s, size_t& pos);

static std::string parse_string(const std::string& s, size_t& pos) {
    if (s[pos] != '"')
        return "";
    ++pos;
    std::string out;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\') {
            ++pos;
            if (pos < s.size()) {
                switch (s[pos]) {
                case 'n':
                    out += '\n';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case '/':
                    out += '/';
                    break;
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case 'u':
                    // Minimal \uXXXX handling: read 4 hex digits and emit the
                    // BMP codepoint as UTF-8. Sufficient for LSP filenames and
                    // most identifiers; surrogate pairs are not combined.
                    if (pos + 4 < s.size()) {
                        unsigned int code = 0;
                        bool bad = false;
                        for (int k = 0; k < 4; ++k) {
                            char h = s[pos + 1 + k];
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (unsigned)(h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (unsigned)(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (unsigned)(h - 'A' + 10);
                            else { bad = true; break; }
                        }
                        if (!bad) {
                            pos += 4;
                            if (code < 0x80) {
                                out += (char)code;
                            } else if (code < 0x800) {
                                out += (char)(0xC0 | (code >> 6));
                                out += (char)(0x80 | (code & 0x3F));
                            } else {
                                out += (char)(0xE0 | (code >> 12));
                                out += (char)(0x80 | ((code >> 6) & 0x3F));
                                out += (char)(0x80 | (code & 0x3F));
                            }
                        } else {
                            // Truncated/bad escape — emit replacement char.
                            out += '?';
                        }
                    } else {
                        out += '?';
                    }
                    break;
                default:
                    // Unknown escape: keep the character verbatim per RFC 8259.
                    out += s[pos];
                    break;
                }
            }
        } else {
            out += s[pos];
        }
        ++pos;
    }
    if (pos < s.size())
        ++pos;
    return out;
}

static JsonValue parse_value(const std::string& s, size_t& pos) {
    pos = skip_ws(s, pos);
    if (pos >= s.size())
        return JsonValue::null();

    char c = s[pos];
    if (c == '"') {
        return JsonValue::string(parse_string(s, pos));
    }
    if (c == '{') {
        ++pos;
        JsonValue obj = JsonValue::object();
        pos = skip_ws(s, pos);
        while (pos < s.size() && s[pos] != '}') {
            pos = skip_ws(s, pos);
            std::string key = parse_string(s, pos);
            pos = skip_ws(s, pos);
            if (pos < s.size() && s[pos] == ':')
                ++pos;
            JsonValue val = parse_value(s, pos);
            obj.set(key, val);
            pos = skip_ws(s, pos);
            if (pos < s.size() && s[pos] == ',')
                ++pos;
        }
        if (pos < s.size())
            ++pos;
        return obj;
    }
    if (c == '[') {
        ++pos;
        JsonValue arr = JsonValue::array();
        pos = skip_ws(s, pos);
        while (pos < s.size() && s[pos] != ']') {
            arr.push(parse_value(s, pos));
            pos = skip_ws(s, pos);
            if (pos < s.size() && s[pos] == ',')
                ++pos;
        }
        if (pos < s.size())
            ++pos;
        return arr;
    }
    if (c == 't' || c == 'f') {
        bool val = (s.substr(pos, 4) == "true");
        pos += val ? 4 : 5;
        return JsonValue::boolean(val);
    }
    if (c == 'n') {
        pos += 4;
        return JsonValue::null();
    }
    if (c == '-' || std::isdigit(c)) {
        size_t start = pos;
        if (s[pos] == '-')
            ++pos;
        while (pos < s.size() && std::isdigit(s[pos]))
            ++pos;
        return JsonValue::integer(std::stoi(s.substr(start, pos - start)));
    }
    return JsonValue::null();
}

JsonValue JsonValue::parse(const std::string& json) {
    size_t pos = 0;
    return parse_value(json, pos);
}

LanguageServer::LanguageServer() {}

void LanguageServer::send_message(const std::string& body) {
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    std::cout.flush();
}

void LanguageServer::send_response(int id, const JsonValue& result) {
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("id", JsonValue::integer(id));
    msg.set("result", result);
    send_message(msg.dump());
}

void LanguageServer::send_error(int id, int code, const std::string& message) {
    JsonValue err = JsonValue::object();
    err.set("code", JsonValue::integer(code));
    err.set("message", JsonValue::string(message));
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("id", JsonValue::integer(id));
    msg.set("error", err);
    send_message(msg.dump());
}

void LanguageServer::send_notification(const std::string& method, const JsonValue& params) {
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("method", JsonValue::string(method));
    msg.set("params", params);
    send_message(msg.dump());
}

void LanguageServer::run() {
    std::cerr << "Alphabet LSP Server v" << ALPHABET_VERSION << " started\n";
    std::cerr << "Waiting for editor connection...\n";
    std::cerr << "Capabilities: hover, completion, definition, references, rename, "
              "symbols, formatting, rangeFormatting, signatureHelp, codeAction\n\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        int content_length = 0;
        if (line.find("Content-Length:") == 0) {
            content_length = std::stoi(line.substr(15));
        } else {
            continue;
        }

        std::getline(std::cin, line);

        std::string body(content_length, '\0');
        std::cin.read(&body[0], content_length);

        JsonValue msg = JsonValue::parse(body);
        std::string method = msg.get_str("method");
        int id = msg.get_int("id", -1);

        if (method == "initialize") {
            send_response(id, handle_initialize(id, msg.get("params")));
        } else if (method == "initialized" || method == "$/setTrace") {
        } else if (method == "textDocument/didOpen") {
            handle_did_open(msg.get("params"));
        } else if (method == "textDocument/didChange") {
            handle_did_change(msg.get("params"));
        } else if (method == "textDocument/didSave") {
            handle_did_save(msg.get("params"));
        } else if (method == "textDocument/didClose") {
            handle_did_close(msg.get("params"));
        } else if (method == "textDocument/completion") {
            send_response(id, handle_completion(id, msg.get("params")));
        } else if (method == "textDocument/hover") {
            send_response(id, handle_hover(id, msg.get("params")));
        } else if (method == "textDocument/documentSymbol") {
            send_response(id, handle_document_symbol(id, msg.get("params")));
        } else if (method == "textDocument/definition") {
            send_response(id, handle_definition(id, msg.get("params")));
        } else if (method == "textDocument/references") {
            send_response(id, handle_references(id, msg.get("params")));
        } else if (method == "textDocument/rename") {
            send_response(id, handle_rename(id, msg.get("params")));
        } else if (method == "textDocument/signatureHelp") {
            send_response(id, handle_signature_help(id, msg.get("params")));
        } else if (method == "textDocument/formatting") {
            send_response(id, handle_formatting(id, msg.get("params")));
        } else if (method == "textDocument/rangeFormatting") {
            send_response(id, handle_range_formatting(id, msg.get("params")));
        } else if (method == "textDocument/codeAction") {
            send_response(id, JsonValue::array());
        } else if (method == "shutdown") {
            send_response(id, JsonValue::null());
        } else if (method == "exit") {
            break;
        } else if (id >= 0) {
            send_error(id, -32601, "Method not found: " + method);
        }
    }
}

JsonValue LanguageServer::handle_initialize(int, const JsonValue&) {
    JsonValue caps = JsonValue::object();

    JsonValue sync = JsonValue::object();
    sync.set("openClose", JsonValue::boolean(true));
    sync.set("change", JsonValue::integer(1));
    sync.set("willSave", JsonValue::boolean(false));
    sync.set("save", JsonValue::boolean(true));
    caps.set("textDocumentSync", sync);

    JsonValue comp = JsonValue::object();
    JsonValue triggers = JsonValue::array();
    triggers.push(JsonValue::string(" "));
    triggers.push(JsonValue::string("."));
    triggers.push(JsonValue::string("("));
    triggers.push(JsonValue::string("\n"));
    comp.set("triggerCharacters", triggers);
    JsonValue res = JsonValue::object();
    res.set("editRange", JsonValue::boolean(true));
    comp.set("resolveProvider", res);
    caps.set("completionProvider", comp);

    caps.set("hoverProvider", JsonValue::boolean(true));

    caps.set("documentSymbolProvider", JsonValue::boolean(true));
    caps.set("definitionProvider", JsonValue::boolean(true));
    caps.set("referencesProvider", JsonValue::boolean(true));
    caps.set("renameProvider", JsonValue::boolean(true));
    caps.set("documentFormattingProvider", JsonValue::boolean(true));
    caps.set("documentRangeFormattingProvider", JsonValue::boolean(true));

    JsonValue sig = JsonValue::object();
    JsonValue sig_triggers = JsonValue::array();
    sig_triggers.push(JsonValue::string("("));
    sig_triggers.push(JsonValue::string(","));
    sig.set("triggerCharacters", sig_triggers);
    caps.set("signatureHelpProvider", sig);

    JsonValue result = JsonValue::object();
    result.set("capabilities", caps);

    JsonValue server_info = JsonValue::object();
    server_info.set("name", JsonValue::string("alphabet-lsp"));
    server_info.set("version", JsonValue::string("2.3.5"));
    result.set("serverInfo", server_info);

    return result;
}

void LanguageServer::handle_did_save(const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it != documents_.end()) {
        publish_diagnostics(uri, it->second);
    }
}

void LanguageServer::handle_did_close(const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    documents_.erase(uri);
    JsonValue empty = JsonValue::array();
    JsonValue params_out = JsonValue::object();
    params_out.set("uri", JsonValue::string(uri));
    params_out.set("diagnostics", empty);
    send_notification("textDocument/publishDiagnostics", params_out);
}

void LanguageServer::handle_did_open(const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    std::string content = params.get("textDocument").get_str("text");
    documents_[uri] = content;
    publish_diagnostics(uri, content);
}

void LanguageServer::handle_did_change(const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue& changes = params.get("contentChanges");
    if (changes.type == JsonValue::ARRAY_T && !changes.arr_val.empty()) {
        std::string content = changes.arr_val[0].get_str("text");
        documents_[uri] = content;
        publish_diagnostics(uri, content);
    }
}

void LanguageServer::publish_diagnostics(const std::string& uri, const std::string& content) {
    JsonValue diags = JsonValue::array();

    // Helper lambda to build a full-line range for a given 0-based line number
    auto line_range = [&](int line, int start_col, int end_col) {
        JsonValue range = JsonValue::object();
        JsonValue start = JsonValue::object();
        start.set("line", JsonValue::integer(line));
        start.set("character", JsonValue::integer(start_col));
        range.set("start", start);
        JsonValue end = JsonValue::object();
        end.set("line", JsonValue::integer(line));
        end.set("character", JsonValue::integer(end_col));
        range.set("end", end);
        return range;
    };

    auto push_diag = [&](int line, int col, int end_col, int severity,
                         const std::string& code, const std::string& message) {
        JsonValue d = JsonValue::object();
        d.set("range", line_range(line, col, end_col));
        d.set("severity", JsonValue::integer(severity));
        d.set("code", JsonValue::string(code));
        d.set("source", JsonValue::string("alphabet"));
        d.set("message", JsonValue::string(message));
        diags.push(d);
    };

    // Normalize line endings: LSP doesn't promise CRLF vs LF, and the parser
    // can mis-count columns if we pass raw CRLF through unchanged. Cheap to do.
    std::string normalized;
    normalized.reserve(content.size());
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\r') {
            if (i + 1 < content.size() && content[i + 1] == '\n')
                continue;
            normalized.push_back('\n');
        } else {
            normalized.push_back(content[i]);
        }
    }
    const std::string& src = normalized;

    // Strip just the first line for the header check so a string literal in
    // the body doesn't accidentally satisfy the directive requirement.
    std::string first_line;
    {
        size_t nl = src.find('\n');
        first_line = (nl == std::string::npos) ? src : src.substr(0, nl);
    }

    bool has_header = first_line.rfind("#alphabet<", 0) == 0;

    if (!has_header) {
        push_diag(0, 0, (int)first_line.size(), 1, "alphabet/E001",
                  "Missing required header '#alphabet<lang>' on line 1 "
                  "(valid: en, am, de, es, fr)");
    }

    if (has_header) {
        try {
            Lexer lexer(src);
            auto tokens = lexer.scan_tokens();
            Parser parser(tokens, src);
            parser.parse();

            if (parser.had_errors() && !parser.first_error().empty()) {
                std::string err = parser.first_error();
                int err_line = 0, err_col = 0;
                size_t lp = err.find("line ");
                if (lp != std::string::npos) {
                    try {
                        err_line = std::stoi(err.substr(lp + 5)) - 1;
                    } catch (...) { err_line = 0; }
                }
                size_t cp = err.find("column ");
                if (cp != std::string::npos) {
                    try {
                        err_col = std::stoi(err.substr(cp + 7)) - 1;
                    } catch (...) { err_col = 0; }
                }
                if (err_line < 0) err_line = 0;
                if (err_col < 0) err_col = 0;

                size_t newline = err.find('\n');
                std::string clean_msg = (newline != std::string::npos) ? err.substr(0, newline) : err;

                // Find actual end of offending token by reading the source line
                int end_col = err_col + 1;
                {
                    std::istringstream ls(src);
                    std::string l;
                    int ln = 0;
                    while (std::getline(ls, l)) {
                        if (ln == err_line) {
                            int end = err_col;
                            while (end < (int)l.size() && (std::isalnum(l[end]) || l[end] == '_'))
                                ++end;
                            end_col = std::max(end, err_col + 1);
                            if (end_col > (int)l.size()) end_col = (int)l.size();
                            break;
                        }
                        ++ln;
                    }
                }

                push_diag(err_line, err_col, end_col, 1, "alphabet/E100", clean_msg);
            }
        } catch (const std::exception&) {
            // Swallow lexer/parser crashes so they don't bring the server down.
        }
    }

    JsonValue params_out = JsonValue::object();
    params_out.set("uri", JsonValue::string(uri));
    params_out.set("diagnostics", diags);
    send_notification("textDocument/publishDiagnostics", params_out);
}

JsonValue LanguageServer::handle_completion(int, const JsonValue&) {
    JsonValue items = JsonValue::array();

    struct {
        const char* label;
        const char* detail;
        const char* doc;
    } keywords[] = {
        {"i", "if", "Conditional: i (cond) { ... }"},
        {"e", "else", "Alternative branch"},
        {"l", "loop", "Loop: l (cond) { ... } or l (init : cond : incr) { ... }"},
        {"b", "break", "Exit current loop"},
        {"k", "continue", "Skip to next iteration"},
        {"r", "return", "Return from function"},
        {"c", "class", "Define class: c Name { ... }"},
        {"m", "method", "Define method: m ret_type name(params) { ... }"},
        {"n", "new", "Create instance: n ClassName()"},
        {"v", "public", "Public visibility"},
        {"p", "private", "Private visibility"},
        {"s", "static", "Static member"},
        {"t", "try", "Try block for exceptions"},
        {"h", "handle", "Catch exceptions: h (type var) { ... }"},
        {"x", "import", "Import module: x \"path\""},
        {"q", "match", "Pattern match: q (expr) { ... }"},
        {"j", "interface", "Define interface"},
        {"true", "true", "Boolean true (1.0)"},
        {"false", "false", "Boolean false (0.0)"},
        {"const", "const", "Constant declaration: const name = value"},
    };
    for (auto& kw : keywords) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(kw.label));
        item.set("kind", JsonValue::integer(14));
        item.set("detail", JsonValue::string(kw.detail));
        item.set("documentation", JsonValue::string(kw.doc));
        items.push(item);
    }

    struct {
        const char* label;
        const char* detail;
    } types[] = {
        {"0", "void"}, {"1", "i8"},    {"2", "i16"},   {"3", "i32"},  {"4", "i64"},   {"5", "int"},  {"6", "f32"},
        {"7", "f64"},  {"8", "float"}, {"11", "bool"}, {"12", "str"}, {"13", "list"}, {"14", "map"},
    };
    for (auto& tp : types) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(tp.label));
        item.set("kind", JsonValue::integer(25));
        item.set("detail", JsonValue::string(tp.detail));
        items.push(item);
    }

    struct {
        const char* label;
        const char* detail;
        const char* doc;
    } builtins[] = {
        {"z.o", "print", "Print value: z.o(value)"},
        {"z.i", "input", "Read from stdin"},
        {"z.sqrt", "sqrt", "Square root: z.sqrt(x)"},
        {"z.abs", "abs", "Absolute value: z.abs(x)"},
        {"z.sin", "sin", "Sine: z.sin(x)"},
        {"z.cos", "cos", "Cosine: z.cos(x)"},
        {"z.pow", "pow", "Power: z.pow(base, exp)"},
        {"z.floor", "floor", "Floor: z.floor(x)"},
        {"z.ceil", "ceil", "Ceiling: z.ceil(x)"},
        {"z.len", "len", "Length: z.len(str_or_list)"},
        {"z.type", "type", "Type name: z.type(val)"},
        {"z.tostr", "tostr", "Convert to string"},
        {"z.tonum", "tonum", "Convert to number"},
        {"z.tos", "tos", "Alias for tostr"},
        {"z.t", "throw", "Throw exception: z.t(msg)"},
        {"z.f", "file", "Read file: z.f(path)"},
        {"z.split", "split", "Split string: z.split(str, delim)"},
        {"z.join", "join", "Join list: z.join(list, sep)"},
        {"z.replace", "replace", "Replace: z.replace(str, old, new)"},
        {"z.trim", "trim", "Trim whitespace: z.trim(str)"},
        {"z.upper", "upper", "Uppercase: z.upper(str)"},
        {"z.lower", "lower", "Lowercase: z.lower(str)"},
        {"z.dyn", "FFI", "Call C function: z.dyn(\"lib.so\", \"func\", args...)"},
    };
    for (auto& bi : builtins) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(bi.label));
        item.set("kind", JsonValue::integer(3));
        item.set("detail", JsonValue::string(bi.detail));
        item.set("documentation", JsonValue::string(bi.doc));
        items.push(item);
    }

    JsonValue result = JsonValue::object();
    result.set("items", items);
    result.set("isIncomplete", JsonValue::boolean(true));
    return result;
}

std::string LanguageServer::get_hover_doc(const std::string& word) {
    static const std::unordered_map<std::string, std::string> docs = {
        {"i", "if - Conditional statement\nSyntax: i (condition) { body }"},
        {"e", "else - Alternative branch\nSyntax: e { body }"},
        {"l", "loop - While/for loop\nSyntax: l (cond) { body } or l (init : cond : incr) { body }"},
        {"b", "break - Exit current loop"},
        {"k", "continue - Skip to next loop iteration"},
        {"r", "return - Return from function\nSyntax: r value"},
        {"c", "class - Define class\nSyntax: c ClassName { fields, methods }"},
        {"m", "method - Define function/method\nSyntax: m return_type name(params) { body }"},
        {"n", "new - Create instance\nSyntax: n ClassName(args)"},
        {"v", "public - Public visibility modifier"},
        {"p", "private - Private visibility modifier"},
        {"s", "static - Static member modifier"},
        {"t", "try - Exception handling\nSyntax: t { body } h (type var) { handler }"},
        {"h", "handle - Catch exceptions\nSyntax: h (type var) { handler }"},
        {"x", "import - Import module\nSyntax: x \"path/module.abc\""},
        {"q", "match - Pattern matching\nSyntax: q (expr) { pattern: body }"},
        {"j", "interface - Define interface\nSyntax: j Name { method signatures }"},
        {"true", "true - Boolean true literal\nEquivalent to 1.0"},
        {"false", "false - Boolean false literal\nEquivalent to 0.0"},
        {"const", "const - Declare constant\nSyntax: const name = expression\nCannot be reassigned "
                  "after declaration"},
        {"z.o", "print - Output to stdout\nSyntax: z.o(value)"},
        {"z.i", "input - Read from stdin\nReturns string or number"},
        {"z.sqrt", "sqrt - Square root\nSyntax: z.sqrt(number)"},
        {"z.abs", "abs - Absolute value\nSyntax: z.abs(number)"},
        {"z.pow", "pow - Exponentiation\nSyntax: z.pow(base, exponent)"},
        {"z.len", "len - Length of string/list\nSyntax: z.len(str_or_list)"},
        {"z.type", "type - Get type as string\nSyntax: z.type(value)"},
        {"z.tostr", "tostr - Convert to string\nSyntax: z.tostr(value)"},
        {"z.tonum", "tonum - Convert to number\nSyntax: z.tonum(value)"},
        {"z.tos", "tos - Alias for tostr\nSyntax: z.tos(value)"},
        {"z.t", "throw - Throw exception\nSyntax: z.t(message)"},
        {"z.f", "file - Read file contents\nSyntax: z.f(path)\nSandbox: returns empty string in "
                "sandbox mode"},
        {"z.sin", "sin - Sine\nSyntax: z.sin(radians)"},
        {"z.cos", "cos - Cosine\nSyntax: z.cos(radians)"},
        {"z.floor", "floor - Floor\nSyntax: z.floor(number)"},
        {"z.ceil", "ceil - Ceiling\nSyntax: z.ceil(number)"},
        {"z.split", "split - Split string\nSyntax: z.split(string, delimiter) → list"},
        {"z.join", "join - Join list\nSyntax: z.join(list, separator) → string"},
        {"z.replace", "replace - Replace substring\nSyntax: z.replace(string, old, new) → string"},
        {"z.trim", "trim - Trim whitespace\nSyntax: z.trim(string) → string"},
        {"z.upper", "upper - Uppercase\nSyntax: z.upper(string) → string"},
        {"z.lower", "lower - Lowercase\nSyntax: z.lower(string) → string"},
        {"z.dyn", "FFI - Call native C function\nSyntax: z.dyn(\"lib.so\", \"func_name\", "
                  "args...)\nSandbox: blocked in sandbox mode"},
    };
    auto it = docs.find(word);
    return it != docs.end() ? it->second : "";
}

// ============================================================================
// Document symbol scan (used by both hover and document symbol provider)
// ============================================================================
//
// Walks the source line-by-line, the same way the lexer would, but without
// instantiating the full parser. Returns a map: symbol name → markdown
// documentation string suitable for hover. Only single-document scope for
// now; cross-file symbols would need a project/index model.

struct DocSymbol {
    std::string kind;       // "function" | "method" | "class" | "interface" | "variable" | "constant" | "field"
    std::string signature;  // raw declaration text (one line)
    int line = 0;            // 0-based line where declared
};

static std::unordered_map<std::string, std::string> scan_user_symbols(const std::string& source) {
    std::unordered_map<std::string, std::string> out;
    std::istringstream stream(source);
    std::string line;
    int line_num = 0;
    std::string current_class;

    auto capture_signature = [&](const std::string& kind, const std::string& name,
                                  const std::string& line_text) {
        std::string md = "```alphabet\n";
        md += line_text;
        md += "\n```\n\n*";
        md += kind;
        md += "* declared on line ";
        md += std::to_string(line_num + 1);
        if (!current_class.empty()) {
            md += " (in `";
            md += current_class;
            md += "`)";
        }
        out[name] = md;
    };

    while (std::getline(stream, line)) {
        size_t first_ns = line.find_first_not_of(" \t");
        if (first_ns == std::string::npos) { ++line_num; continue; }
        char fc = line[first_ns];
        if (fc == '/' || fc == '#') { ++line_num; continue; }

        // class Name
        if (fc == 'c' && first_ns + 1 < line.size() && line[first_ns + 1] == ' ') {
            size_t p = first_ns + 2;
            while (p < line.size() && line[p] == ' ') ++p;
            size_t s = p;
            while (p < line.size() && (std::isalnum(line[p]) || line[p] == '_')) ++p;
            if (p > s) {
                std::string name = line.substr(s, p - s);
                current_class = name;
                capture_signature("class", name, line);
            }
        }
        // interface Name
        else if (fc == 'j' && first_ns + 1 < line.size() && line[first_ns + 1] == ' ') {
            size_t p = first_ns + 2;
            while (p < line.size() && line[p] == ' ') ++p;
            size_t s = p;
            while (p < line.size() && (std::isalnum(line[p]) || line[p] == '_')) ++p;
            if (p > s) {
                std::string name = line.substr(s, p - s);
                capture_signature("interface", name, line);
            }
        }
        // m <type> name(params)
        else if (line.find("m ") != std::string::npos) {
            size_t mp = line.find("m ");
            bool anchored = (mp == first_ns) || (mp > 0 && (line[mp - 1] == ' ' || line[mp - 1] == '\t'));
            if (anchored) {
                size_t p = mp + 2;
                while (p < line.size() && line[p] == ' ') ++p;
                while (p < line.size() && std::isdigit(line[p])) ++p;
                while (p < line.size() && line[p] == ' ') ++p;
                size_t s = p;
                while (p < line.size() && (std::isalnum(line[p]) || line[p] == '_')) ++p;
                if (p > s) {
                    std::string name = line.substr(s, p - s);
                    std::string kind = current_class.empty() ? "function" : "method";
                    capture_signature(kind, name, line);
                }
            }
        }
        // const name = ...
        else if (line.substr(first_ns, 5) == "const" &&
                 (first_ns + 5 >= line.size() || line[first_ns + 5] == ' ')) {
            size_t p = first_ns + 5;
            while (p < line.size() && line[p] == ' ') ++p;
            size_t s = p;
            while (p < line.size() && (std::isalnum(line[p]) || line[p] == '_')) ++p;
            if (p > s) {
                std::string name = line.substr(s, p - s);
                capture_signature("constant", name, line);
            }
        }
        // <digit type> name = expr   (variable/field)
        else if (std::isdigit(fc)) {
            size_t p = first_ns;
            while (p < line.size() && std::isdigit(line[p])) ++p;
            while (p < line.size() && line[p] == ' ') ++p;
            size_t s = p;
            while (p < line.size() && (std::isalnum(line[p]) || line[p] == '_')) ++p;
            if (p > s) {
                size_t after = p;
                while (after < line.size() && line[after] == ' ') ++after;
                if (after < line.size() && line[after] == '=') {
                    std::string name = line.substr(s, p - s);
                    std::string kind = current_class.empty() ? "variable" : "field";
                    capture_signature(kind, name, line);
                }
            }
        }

        if (fc == '}' && !current_class.empty()) current_class.clear();
        ++line_num;
    }
    return out;
}

JsonValue LanguageServer::handle_hover(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue& pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::null();

    std::string current_line;
    int ln = 0;
    std::istringstream stream(it->second);
    while (std::getline(stream, current_line)) {
        if (ln == line)
            break;
        ++ln;
    }

    if (character >= (int)current_line.size())
        return JsonValue::null();

    int start = character, end = character;
    while (start > 0 &&
           (std::isalnum(current_line[start - 1]) || current_line[start - 1] == '_' || current_line[start - 1] == '.'))
        --start;
    while (end < (int)current_line.size() &&
           (std::isalnum(current_line[end]) || current_line[end] == '_' || current_line[end] == '.'))
        ++end;
    if (start == end)
        return JsonValue::null();

    std::string word = current_line.substr(start, end - start);

    // User-defined symbols take precedence over built-ins — this lets users
    // legitimately shadow single-letter keywords (e.g. `5 x = 0`) without
    // getting the keyword's docstring when they hover their own variable.
    std::string doc;
    auto user_syms = scan_user_symbols(it->second);
    auto uit = user_syms.find(word);
    if (uit != user_syms.end()) {
        doc = uit->second;
    } else {
        doc = get_hover_doc(word);
    }
    if (doc.empty())
        return JsonValue::null();

    JsonValue contents = JsonValue::object();
    contents.set("kind", JsonValue::string("markdown"));
    contents.set("value", JsonValue::string(doc));

    JsonValue result = JsonValue::object();
    result.set("contents", contents);
    return result;
}

static std::string word_at(const std::string& line, int character) {
    if (character < 0 || character >= (int)line.size())
        return "";
    int start = character, end = character;
    while (start > 0 && (std::isalnum(line[start - 1]) || line[start - 1] == '_' || line[start - 1] == '.'))
        --start;
    while (end < (int)line.size() && (std::isalnum(line[end]) || line[end] == '_' || line[end] == '.'))
        ++end;
    return line.substr(start, end - start);
}

static bool is_type_prefix(const std::string& line, size_t pos) {
    size_t i = pos;
    if (i >= line.size() || !std::isdigit(line[i]))
        return false;
    while (i < line.size() && std::isdigit(line[i]))
        ++i;

    if (i >= line.size() || line[i] != ' ')
        return false;
    while (i < line.size() && line[i] == ' ')
        ++i;
    return i < line.size() && (std::isalpha(line[i]) || line[i] == '_');
}

static const int SK_FILE = 1;
static const int SK_CLASS = 5;
static const int SK_METHOD = 6;
static const int SK_FUNCTION = 12;
static const int SK_VARIABLE = 13;
static const int SK_CONSTANT = 14;
static const int SK_FIELD = 8;

JsonValue LanguageServer::handle_document_symbol(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::array();

    const std::string& content = it->second;
    JsonValue symbols = JsonValue::array();

    std::istringstream stream(content);
    std::string line;
    int line_num = 0;

    std::string current_class;

    std::vector<size_t> class_indices;

    while (std::getline(stream, line)) {
        size_t first_nonspace = line.find_first_not_of(" \t");
        if (first_nonspace == std::string::npos) {
            ++line_num;
            continue;
        }
        char fc = line[first_nonspace];
        if (fc == '/' || fc == '#') {
            ++line_num;
            continue;
        }

        if (fc == 'c' && first_nonspace + 1 < line.size() && line[first_nonspace + 1] == ' ') {
            size_t name_start = first_nonspace + 2;
            while (name_start < line.size() && line[name_start] == ' ')
                ++name_start;
            size_t name_end = name_start;
            while (name_end < line.size() && (std::isalnum(line[name_end]) || line[name_end] == '_'))
                ++name_end;
            if (name_end > name_start) {
                std::string name = line.substr(name_start, name_end - name_start);
                current_class = name;

                JsonValue sym = JsonValue::object();
                sym.set("name", JsonValue::string(name));
                sym.set("kind", JsonValue::integer(SK_CLASS));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(line_num));
                start.set("character", JsonValue::integer((int)first_nonspace));
                range.set("start", start);
                range.set("end", start);
                sym.set("range", range);
                sym.set("selectionRange", range);
                sym.set("detail", JsonValue::string("class"));

                JsonValue children = JsonValue::array();
                sym.set("children", children);

                class_indices.push_back(symbols.arr_val.size());
                symbols.push(sym);
            }
        }

        else if (line.find("m ") != std::string::npos) {
            size_t m_pos = line.find("m ");

            bool is_method = false;
            if (m_pos == first_nonspace || (m_pos > 0 && (line[m_pos - 1] == ' ' || line[m_pos - 1] == '\t'))) {
                size_t after_m = m_pos + 2;
                while (after_m < line.size() && line[after_m] == ' ')
                    ++after_m;

                if (after_m < line.size() && std::isdigit(line[after_m])) {
                    is_method = true;
                }
            }
            if (is_method) {
                size_t pos = m_pos + 2;
                while (pos < line.size() && line[pos] == ' ')
                    ++pos;

                while (pos < line.size() && std::isdigit(line[pos]))
                    ++pos;
                while (pos < line.size() && line[pos] == ' ')
                    ++pos;

                size_t name_start = pos;
                while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                    ++pos;
                if (pos > name_start) {
                    std::string name = line.substr(name_start, pos - name_start);
                    bool is_static = (line.find("s ") != std::string::npos && line.find("s ") < m_pos);

                    JsonValue sym = JsonValue::object();
                    sym.set("name", JsonValue::string(name));
                    sym.set("kind", JsonValue::integer(current_class.empty() ? SK_FUNCTION : SK_METHOD));
                    JsonValue range = JsonValue::object();
                    JsonValue start_p = JsonValue::object();
                    start_p.set("line", JsonValue::integer(line_num));
                    start_p.set("character", JsonValue::integer((int)m_pos));
                    range.set("start", start_p);
                    range.set("end", start_p);
                    sym.set("range", range);
                    sym.set("selectionRange", range);
                    std::string detail = (current_class.empty() ? "function" : "method");
                    if (is_static)
                        detail = "static " + detail;
                    sym.set("detail", JsonValue::string(detail));

                    symbols.push(sym);
                }
            }
        }

        else if (fc == 'j' && first_nonspace + 1 < line.size() && line[first_nonspace + 1] == ' ') {
            size_t name_start = first_nonspace + 2;
            while (name_start < line.size() && line[name_start] == ' ')
                ++name_start;
            size_t name_end = name_start;
            while (name_end < line.size() && (std::isalnum(line[name_end]) || line[name_end] == '_'))
                ++name_end;
            if (name_end > name_start) {
                std::string name = line.substr(name_start, name_end - name_start);

                JsonValue sym = JsonValue::object();
                sym.set("name", JsonValue::string(name));
                sym.set("kind", JsonValue::integer(SK_CLASS));
                JsonValue range = JsonValue::object();
                JsonValue start_p = JsonValue::object();
                start_p.set("line", JsonValue::integer(line_num));
                start_p.set("character", JsonValue::integer((int)first_nonspace));
                range.set("start", start_p);
                range.set("end", start_p);
                sym.set("range", range);
                sym.set("selectionRange", range);
                sym.set("detail", JsonValue::string("interface"));
                symbols.push(sym);
            }
        }

        else if (line.substr(first_nonspace, 5) == "const" &&
                 (first_nonspace + 5 >= line.size() || line[first_nonspace + 5] == ' ')) {
            size_t pos = first_nonspace + 5;
            while (pos < line.size() && line[pos] == ' ')
                ++pos;
            size_t name_start = pos;
            while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                ++pos;
            if (pos > name_start) {
                std::string name = line.substr(name_start, pos - name_start);

                JsonValue sym = JsonValue::object();
                sym.set("name", JsonValue::string(name));
                sym.set("kind", JsonValue::integer(SK_CONSTANT));
                JsonValue range = JsonValue::object();
                JsonValue start_p = JsonValue::object();
                start_p.set("line", JsonValue::integer(line_num));
                start_p.set("character", JsonValue::integer((int)first_nonspace));
                range.set("start", start_p);
                range.set("end", start_p);
                sym.set("range", range);
                sym.set("selectionRange", range);
                sym.set("detail", JsonValue::string("const"));
                symbols.push(sym);
            }
        }

        else if (is_type_prefix(line, first_nonspace)) {
            size_t pos = first_nonspace;

            while (pos < line.size() && std::isdigit(line[pos]))
                ++pos;
            while (pos < line.size() && line[pos] == ' ')
                ++pos;
            size_t name_start = pos;
            while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                ++pos;
            if (pos > name_start) {
                size_t after_name = pos;
                while (after_name < line.size() && line[after_name] == ' ')
                    ++after_name;
                if (after_name < line.size() && line[after_name] == '=') {
                    std::string name = line.substr(name_start, pos - name_start);

                    std::string type_str = line.substr(first_nonspace, name_start - first_nonspace - 1);

                    int kind = current_class.empty() ? SK_VARIABLE : SK_FIELD;
                    std::string detail = "type " + type_str;

                    JsonValue sym = JsonValue::object();
                    sym.set("name", JsonValue::string(name));
                    sym.set("kind", JsonValue::integer(kind));
                    JsonValue range = JsonValue::object();
                    JsonValue start_p = JsonValue::object();
                    start_p.set("line", JsonValue::integer(line_num));
                    start_p.set("character", JsonValue::integer((int)first_nonspace));
                    range.set("start", start_p);
                    range.set("end", start_p);
                    sym.set("range", range);
                    sym.set("selectionRange", range);
                    sym.set("detail", JsonValue::string(detail));
                    symbols.push(sym);
                }
            }
        }

        else if (fc == '}' && !current_class.empty()) {
            current_class.clear();
        }

        ++line_num;
    }

    return symbols;
}

JsonValue LanguageServer::handle_definition(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue& pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    auto doc_it = documents_.find(uri);
    if (doc_it == documents_.end())
        return JsonValue::null();

    const std::string& content = doc_it->second;

    std::istringstream stream(content);
    std::string current_line;
    int ln = 0;
    while (std::getline(stream, current_line)) {
        if (ln == line)
            break;
        ++ln;
    }

    std::string word = word_at(current_line, character);
    if (word.empty())
        return JsonValue::null();

    std::string search_name = word;
    size_t dot_pos = word.find('.');
    if (dot_pos != std::string::npos) {
        if (word.substr(0, 2) == "z.")
            return JsonValue::null();
        search_name = word.substr(dot_pos + 1);
    }

    std::istringstream def_stream(content);
    std::string def_line;
    int def_line_num = 0;

    while (std::getline(def_stream, def_line)) {
        size_t first_nonspace = def_line.find_first_not_of(" \t");
        if (first_nonspace == std::string::npos) {
            ++def_line_num;
            continue;
        }
        char fc = def_line[first_nonspace];
        if (fc == '/' || fc == '#') {
            ++def_line_num;
            continue;
        }

        if (fc == 'c' && first_nonspace + 1 < def_line.size() && def_line[first_nonspace + 1] == ' ') {
            size_t ns = first_nonspace + 2;
            while (ns < def_line.size() && def_line[ns] == ' ')
                ++ns;
            size_t ne = ns;
            while (ne < def_line.size() && (std::isalnum(def_line[ne]) || def_line[ne] == '_'))
                ++ne;
            if (ne > ns && def_line.substr(ns, ne - ns) == search_name) {
                JsonValue loc = JsonValue::object();
                loc.set("uri", JsonValue::string(uri));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(def_line_num));
                start.set("character", JsonValue::integer((int)ns));
                range.set("start", start);
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(def_line_num));
                end.set("character", JsonValue::integer((int)ne));
                range.set("end", end);
                loc.set("range", range);
                return loc;
            }
        }

        if (fc == 'j' && first_nonspace + 1 < def_line.size() && def_line[first_nonspace + 1] == ' ') {
            size_t ns = first_nonspace + 2;
            while (ns < def_line.size() && def_line[ns] == ' ')
                ++ns;
            size_t ne = ns;
            while (ne < def_line.size() && (std::isalnum(def_line[ne]) || def_line[ne] == '_'))
                ++ne;
            if (ne > ns && def_line.substr(ns, ne - ns) == search_name) {
                JsonValue loc = JsonValue::object();
                loc.set("uri", JsonValue::string(uri));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(def_line_num));
                start.set("character", JsonValue::integer((int)ns));
                range.set("start", start);
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(def_line_num));
                end.set("character", JsonValue::integer((int)ne));
                range.set("end", end);
                loc.set("range", range);
                return loc;
            }
        }

        if (def_line.find("m ") != std::string::npos) {
            size_t m_pos = def_line.find("m ");
            bool is_method = false;
            if (m_pos == first_nonspace || (m_pos > 0 && (def_line[m_pos - 1] == ' ' || def_line[m_pos - 1] == '\t'))) {
                size_t after_m = m_pos + 2;
                while (after_m < def_line.size() && def_line[after_m] == ' ')
                    ++after_m;
                if (after_m < def_line.size() && std::isdigit(def_line[after_m]))
                    is_method = true;
            }
            if (is_method) {
                size_t pos2 = m_pos + 2;
                while (pos2 < def_line.size() && def_line[pos2] == ' ')
                    ++pos2;
                while (pos2 < def_line.size() && std::isdigit(def_line[pos2]))
                    ++pos2;
                while (pos2 < def_line.size() && def_line[pos2] == ' ')
                    ++pos2;
                size_t name_start = pos2;
                while (pos2 < def_line.size() && (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
                    ++pos2;
                if (pos2 > name_start && def_line.substr(name_start, pos2 - name_start) == search_name) {
                    JsonValue loc = JsonValue::object();
                    loc.set("uri", JsonValue::string(uri));
                    JsonValue range = JsonValue::object();
                    JsonValue start = JsonValue::object();
                    start.set("line", JsonValue::integer(def_line_num));
                    start.set("character", JsonValue::integer((int)name_start));
                    range.set("start", start);
                    JsonValue end = JsonValue::object();
                    end.set("line", JsonValue::integer(def_line_num));
                    end.set("character", JsonValue::integer((int)pos2));
                    range.set("end", end);
                    loc.set("range", range);
                    return loc;
                }
            }
        }

        if (def_line.substr(first_nonspace, 5) == "const" &&
            (first_nonspace + 5 >= def_line.size() || def_line[first_nonspace + 5] == ' ')) {
            size_t pos2 = first_nonspace + 5;
            while (pos2 < def_line.size() && def_line[pos2] == ' ')
                ++pos2;
            size_t name_start = pos2;
            while (pos2 < def_line.size() && (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
                ++pos2;
            if (pos2 > name_start && def_line.substr(name_start, pos2 - name_start) == search_name) {
                JsonValue loc = JsonValue::object();
                loc.set("uri", JsonValue::string(uri));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(def_line_num));
                start.set("character", JsonValue::integer((int)name_start));
                range.set("start", start);
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(def_line_num));
                end.set("character", JsonValue::integer((int)pos2));
                range.set("end", end);
                loc.set("range", range);
                return loc;
            }
        }

        if (is_type_prefix(def_line, first_nonspace)) {
            size_t pos2 = first_nonspace;
            while (pos2 < def_line.size() && std::isdigit(def_line[pos2]))
                ++pos2;
            while (pos2 < def_line.size() && def_line[pos2] == ' ')
                ++pos2;
            size_t name_start = pos2;
            while (pos2 < def_line.size() && (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
                ++pos2;
            if (pos2 > name_start) {
                size_t after_name = pos2;
                while (after_name < def_line.size() && def_line[after_name] == ' ')
                    ++after_name;
                if (after_name < def_line.size() && def_line[after_name] == '=' &&
                    def_line.substr(name_start, pos2 - name_start) == search_name) {
                    JsonValue loc = JsonValue::object();
                    loc.set("uri", JsonValue::string(uri));
                    JsonValue range = JsonValue::object();
                    JsonValue start = JsonValue::object();
                    start.set("line", JsonValue::integer(def_line_num));
                    start.set("character", JsonValue::integer((int)name_start));
                    range.set("start", start);
                    JsonValue end = JsonValue::object();
                    end.set("line", JsonValue::integer(def_line_num));
                    end.set("character", JsonValue::integer((int)pos2));
                    range.set("end", end);
                    loc.set("range", range);
                    return loc;
                }
            }
        }

        ++def_line_num;
    }

    return JsonValue::null();
}

// ============================================================================
// Formatting
// ============================================================================
//
// Format strategy:
//   1. Normalize line endings to \n
//   2. Trim trailing whitespace from each line
//   3. Strip leading/trailing blank lines
//   4. Preserve the user's original indentation but re-tabify runs of spaces
//      in multiples of 4
//   5. Ensure single trailing newline
//
// This is intentionally simple and safe — we never re-flow code, only clean up
// whitespace. The formatter is idempotent: formatting a formatted file leaves
// it unchanged.

static std::string rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\r");
    return end == std::string::npos ? "" : s.substr(0, end + 1);
}

static std::string normalize_indent(const std::string& line, int /*unit*/) {
    size_t i = 0;
    int spaces = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
        if (line[i] == '\t')
            spaces += 4;
        else
            ++spaces;
        ++i;
    }
    int depth = spaces / 4;
    int extra = spaces % 4;
    std::string out;
    for (int d = 0; d < depth; ++d)
        out += "    ";
    out += std::string(extra, ' ');
    out += line.substr(i);
    return out;
}

std::string LanguageServer::format_text(const std::string& source) const {
    std::string normalized;
    normalized.reserve(source.size());

    for (size_t i = 0; i < source.size(); ++i) {
        if (source[i] == '\r') {
            if (i + 1 < source.size() && source[i + 1] == '\n')
                continue;
            normalized.push_back('\n');
        } else {
            normalized.push_back(source[i]);
        }
    }

    std::vector<std::string> lines;
    std::string cur;
    for (char c : normalized) {
        if (c == '\n') {
            lines.push_back(rtrim(cur));
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty())
        lines.push_back(rtrim(cur));

    while (!lines.empty() && lines.front().empty())
        lines.erase(lines.begin());
    while (!lines.empty() && lines.back().empty())
        lines.pop_back();

    // Collapse runs of 2+ blank lines into a single blank line.
    std::vector<std::string> collapsed;
    collapsed.reserve(lines.size());
    bool last_blank = false;
    for (auto& ln : lines) {
        bool blank = ln.empty();
        if (blank && last_blank)
            continue;
        collapsed.push_back(ln);
        last_blank = blank;
    }

    std::string out;
    for (size_t i = 0; i < collapsed.size(); ++i) {
        out += normalize_indent(collapsed[i], indent_unit());
        out += '\n';
    }
    return out;
}

JsonValue LanguageServer::handle_formatting(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        JsonValue empty = JsonValue::array();
        return empty;
    }

    int tab_size = params.get("options").get_int("tabSize", 4);
    if (tab_size <= 0) tab_size = 4;

    std::string source = it->second;
    std::string formatted = format_text(source);

    int orig_lines = 1;
    for (char c : source)
        if (c == '\n') ++orig_lines;
    int new_lines = 1;
    for (char c : formatted)
        if (c == '\n') ++new_lines;

    JsonValue range = JsonValue::object();
    JsonValue start = JsonValue::object();
    start.set("line", JsonValue::integer(0));
    start.set("character", JsonValue::integer(0));
    range.set("start", start);
    JsonValue end = JsonValue::object();
    end.set("line", JsonValue::integer(std::max(orig_lines, new_lines)));
    end.set("character", JsonValue::integer(0));
    range.set("end", end);

    JsonValue edit = JsonValue::object();
    edit.set("range", range);
    edit.set("newText", JsonValue::string(formatted));

    JsonValue edits = JsonValue::array();
    edits.push(edit);

    (void)tab_size;
    return edits;
}

JsonValue LanguageServer::handle_range_formatting(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        JsonValue empty = JsonValue::array();
        return empty;
    }

    const JsonValue& range = params.get("range");
    int start_line = range.get("start").get_int("line");
    int end_line = range.get("end").get_int("line");

    std::istringstream stream(it->second);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(stream, line))
        lines.push_back(line);

    if (start_line < 0) start_line = 0;
    if (end_line >= (int)lines.size()) end_line = (int)lines.size() - 1;
    if (start_line > end_line) {
        JsonValue empty = JsonValue::array();
        return empty;
    }

    std::string block;
    for (int i = start_line; i <= end_line; ++i) {
        block += lines[i];
        if (i < end_line) block += '\n';
    }

    std::string formatted = format_text(block);
    if (formatted.size() && formatted.back() == '\n')
        formatted.pop_back();

    std::vector<std::string> out_lines;
    std::string cur;
    for (char c : formatted) {
        if (c == '\n') {
            out_lines.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty() || !out_lines.empty())
        out_lines.push_back(cur);

    JsonValue edits = JsonValue::array();
    for (size_t i = 0; i < out_lines.size(); ++i) {
        int ln = start_line + (int)i;
        if (ln >= (int)lines.size()) break;
        JsonValue edit_range = JsonValue::object();
        JsonValue edit_start = JsonValue::object();
        edit_start.set("line", JsonValue::integer(ln));
        edit_start.set("character", JsonValue::integer(0));
        edit_range.set("start", edit_start);
        JsonValue edit_end = JsonValue::object();
        edit_end.set("line", JsonValue::integer(ln + 1));
        edit_end.set("character", JsonValue::integer(0));
        edit_range.set("end", edit_end);

        JsonValue edit = JsonValue::object();
        edit.set("range", edit_range);
        edit.set("newText", JsonValue::string(out_lines[i]));
        edits.push(edit);
    }

    return edits;
}

// ============================================================================
// Signature help — show builtin signatures
// ============================================================================

JsonValue LanguageServer::handle_signature_help(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::null();

    const JsonValue& pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    std::istringstream stream(it->second);
    std::string cur_line;
    int ln = 0;
    while (std::getline(stream, cur_line)) {
        if (ln == line) break;
        ++ln;
    }

    int paren_pos = -1;
    int depth = 0;
    for (int i = std::min(character, (int)cur_line.size() - 1); i >= 0; --i) {
        if (cur_line[i] == ')')
            ++depth;
        else if (cur_line[i] == '(') {
            if (depth == 0) {
                paren_pos = i;
                break;
            }
            --depth;
        }
    }
    if (paren_pos < 0)
        return JsonValue::null();

    int start = paren_pos;
    while (start > 0 && (std::isalnum(cur_line[start - 1]) || cur_line[start - 1] == '_' ||
                         cur_line[start - 1] == '.'))
        --start;
    std::string call_name = cur_line.substr(start, paren_pos - start);

    struct Sig {
        const char* name;
        const char* sig;
        const char* doc;
    } sigs[] = {
        {"z.o",      "z.o(value)",                 "Print value to stdout"},
        {"z.i",      "z.i()",                       "Read a line from stdin"},
        {"z.sqrt",   "z.sqrt(x: number): number",   "Square root"},
        {"z.abs",    "z.abs(x: number): number",    "Absolute value"},
        {"z.sin",    "z.sin(x: number): number",    "Sine (radians)"},
        {"z.cos",    "z.cos(x: number): number",    "Cosine (radians)"},
        {"z.pow",    "z.pow(base, exp): number",    "Power"},
        {"z.floor",  "z.floor(x: number): number",  "Floor"},
        {"z.ceil",   "z.ceil(x: number): number",   "Ceiling"},
        {"z.len",    "z.len(x): number",            "Length of string or list"},
        {"z.type",   "z.type(x): string",          "Type name"},
        {"z.tostr",  "z.tostr(x): string",          "Convert to string"},
        {"z.tonum",  "z.tonum(x): number",          "Convert to number"},
        {"z.t",      "z.t(message)",                "Throw an exception"},
        {"z.f",      "z.f(path): string",           "Read file"},
        {"z.split",  "z.split(s, delim): list",     "Split string"},
        {"z.join",   "z.join(list, sep): string",   "Join list"},
        {"z.replace","z.replace(s, old, new): str", "Replace substring"},
        {"z.trim",   "z.trim(s): string",           "Trim whitespace"},
        {"z.upper",  "z.upper(s): string",          "Uppercase"},
        {"z.lower",  "z.lower(s): string",          "Lowercase"},
        {"z.dyn",    "z.dyn(lib, fn, ...args)",     "FFI call"},
    };

    JsonValue signatures = JsonValue::array();
    int active = 0;
    for (size_t i = 0; i < sizeof(sigs) / sizeof(sigs[0]); ++i) {
        if (call_name == sigs[i].name) {
            JsonValue s = JsonValue::object();
            s.set("label", JsonValue::string(sigs[i].sig));
            s.set("documentation", JsonValue::string(sigs[i].doc));
            signatures.push(s);
            active = (int)signatures.arr_val.size() - 1;
        }
    }
    if (signatures.arr_val.empty())
        return JsonValue::null();

    JsonValue result = JsonValue::object();
    result.set("signatures", signatures);
    result.set("activeSignature", JsonValue::integer(active));
    result.set("activeParameter", JsonValue::integer(0));
    return result;
}

// ============================================================================
// References — find all references to the symbol under cursor
// ============================================================================

JsonValue LanguageServer::handle_references(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::array();

    const JsonValue& pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    std::istringstream stream(it->second);
    std::string current_line;
    int ln = 0;
    while (std::getline(stream, current_line)) {
        if (ln == line) break;
        ++ln;
    }
    if (character > (int)current_line.size())
        return JsonValue::array();

    int s = character, e = character;
    while (s > 0 && (std::isalnum(current_line[s - 1]) || current_line[s - 1] == '_'))
        --s;
    while (e < (int)current_line.size() && (std::isalnum(current_line[e]) || current_line[e] == '_'))
        ++e;
    if (s == e) return JsonValue::array();

    std::string needle = current_line.substr(s, e - s);
    if (needle.empty()) return JsonValue::array();

    JsonValue refs = JsonValue::array();
    std::istringstream s2(it->second);
    std::string line_str;
    int line_num = 0;
    while (std::getline(s2, line_str)) {
        size_t search_from = 0;
        while (search_from < line_str.size()) {
            size_t pos2 = line_str.find(needle, search_from);
            if (pos2 == std::string::npos) break;
            bool word_left = pos2 > 0 && (std::isalnum(line_str[pos2 - 1]) || line_str[pos2 - 1] == '_');
            bool word_right =
                pos2 + needle.size() < line_str.size() &&
                (std::isalnum(line_str[pos2 + needle.size()]) || line_str[pos2 + needle.size()] == '_');
            if (!word_left && !word_right) {
                JsonValue loc = JsonValue::object();
                loc.set("uri", JsonValue::string(uri));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(line_num));
                start.set("character", JsonValue::integer((int)pos2));
                range.set("start", start);
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(line_num));
                end.set("character", JsonValue::integer((int)(pos2 + needle.size())));
                range.set("end", end);
                loc.set("range", range);
                refs.push(loc);
            }
            search_from = pos2 + needle.size();
            if (search_from == 0) break;
        }
        ++line_num;
    }
    return refs;
}

// ============================================================================
// Rename — rename symbol across the document
// ============================================================================

JsonValue LanguageServer::handle_rename(int, const JsonValue& params) {
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::null();

    const JsonValue& pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");
    std::string new_name = params.get("newName").str_val;
    if (new_name.empty()) return JsonValue::null();

    std::istringstream stream(it->second);
    std::string current_line;
    int ln = 0;
    while (std::getline(stream, current_line)) {
        if (ln == line) break;
        ++ln;
    }
    if (character > (int)current_line.size()) return JsonValue::null();

    int s = character, e = character;
    while (s > 0 && (std::isalnum(current_line[s - 1]) || current_line[s - 1] == '_'))
        --s;
    while (e < (int)current_line.size() && (std::isalnum(current_line[e]) || current_line[e] == '_'))
        ++e;
    if (s == e) return JsonValue::null();

    std::string old_name = current_line.substr(s, e - s);
    if (old_name == new_name) return JsonValue::null();

    JsonValue edits = JsonValue::array();
    std::istringstream s2(it->second);
    std::string line_str;
    int line_num = 0;
    while (std::getline(s2, line_str)) {
        size_t search_from = 0;
        while (search_from < line_str.size()) {
            size_t pos2 = line_str.find(old_name, search_from);
            if (pos2 == std::string::npos) break;
            bool word_left = pos2 > 0 && (std::isalnum(line_str[pos2 - 1]) || line_str[pos2 - 1] == '_');
            bool word_right =
                pos2 + old_name.size() < line_str.size() &&
                (std::isalnum(line_str[pos2 + old_name.size()]) || line_str[pos2 + old_name.size()] == '_');
            if (!word_left && !word_right) {
                JsonValue edit = JsonValue::object();
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(line_num));
                start.set("character", JsonValue::integer((int)pos2));
                range.set("start", start);
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(line_num));
                end.set("character", JsonValue::integer((int)(pos2 + old_name.size())));
                range.set("end", end);
                edit.set("range", range);
                edit.set("newText", JsonValue::string(new_name));
                edits.push(edit);
            }
            search_from = pos2 + old_name.size();
            if (search_from == 0) break;
        }
        ++line_num;
    }

    JsonValue workspace_edit = JsonValue::object();
    JsonValue changes = JsonValue::object();
    changes.set(uri, edits);
    workspace_edit.set("changes", changes);
    return workspace_edit;
}

} // namespace lsp
} // namespace alphabet
