#include "lsp.h"
#include "lexer.h"
#include "parser.h"
#include <algorithm>
#include <cctype>

namespace alphabet {
namespace lsp {

// Simple JSON serializer
static std::string escape_json(const std::string &s)
{
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

std::string JsonValue::dump() const
{
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
        for (const auto &[k, v] : obj_val) {
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

// Simple JSON parser (handles LSP message format)
static size_t skip_ws(const std::string &s, size_t pos)
{
    while (pos < s.size() && std::isspace(s[pos]))
        ++pos;
    return pos;
}

static JsonValue parse_value(const std::string &s, size_t &pos);

static std::string parse_string(const std::string &s, size_t &pos)
{
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
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                default:
                    out += s[pos];
                    break;
                }
            }
        }
        else {
            out += s[pos];
        }
        ++pos;
    }
    if (pos < s.size())
        ++pos; // skip closing "
    return out;
}

static JsonValue parse_value(const std::string &s, size_t &pos)
{
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
        pos += 4; // null
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

JsonValue JsonValue::parse(const std::string &json)
{
    size_t pos = 0;
    return parse_value(json, pos);
}

// LSP server implementation
LanguageServer::LanguageServer() {}

void LanguageServer::send_message(const std::string &body)
{
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    std::cout.flush();
}

void LanguageServer::send_response(int id, const JsonValue &result)
{
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("id", JsonValue::integer(id));
    msg.set("result", result);
    send_message(msg.dump());
}

void LanguageServer::send_error(int id, int code, const std::string &message)
{
    JsonValue err = JsonValue::object();
    err.set("code", JsonValue::integer(code));
    err.set("message", JsonValue::string(message));
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("id", JsonValue::integer(id));
    msg.set("error", err);
    send_message(msg.dump());
}

void LanguageServer::send_notification(const std::string &method, const JsonValue &params)
{
    JsonValue msg = JsonValue::object();
    msg.set("jsonrpc", JsonValue::string("2.0"));
    msg.set("method", JsonValue::string(method));
    msg.set("params", params);
    send_message(msg.dump());
}

void LanguageServer::run()
{
    std::string line;
    while (std::getline(std::cin, line)) {
        int content_length = 0;
        if (line.find("Content-Length:") == 0) {
            content_length = std::stoi(line.substr(15));
        }
        else {
            continue;
        }

        // Read empty line
        std::getline(std::cin, line);

        // Read JSON body
        std::string body(content_length, '\0');
        std::cin.read(&body[0], content_length);

        JsonValue msg = JsonValue::parse(body);
        std::string method = msg.get_str("method");
        int id = msg.get_int("id", -1);

        if (method == "initialize") {
            send_response(id, handle_initialize(id, msg.get("params")));
        }
        else if (method == "initialized" || method == "$/setTrace") {
            // No response
        }
        else if (method == "textDocument/didOpen") {
            handle_did_open(msg.get("params"));
        }
        else if (method == "textDocument/didChange") {
            handle_did_change(msg.get("params"));
        }
        else if (method == "textDocument/completion") {
            send_response(id, handle_completion(id, msg.get("params")));
        }
        else if (method == "textDocument/hover") {
            send_response(id, handle_hover(id, msg.get("params")));
        }
        else if (method == "shutdown") {
            send_response(id, JsonValue::null());
        }
        else if (method == "exit") {
            break;
        }
        else if (id >= 0) {
            send_error(id, -32601, "Method not found: " + method);
        }
    }
}

JsonValue LanguageServer::handle_initialize(int /*id*/, const JsonValue & /*params*/)
{
    JsonValue caps = JsonValue::object();

    JsonValue sync = JsonValue::object();
    sync.set("openClose", JsonValue::boolean(true));
    sync.set("change", JsonValue::integer(1));
    caps.set("textDocumentSync", sync);

    JsonValue comp = JsonValue::object();
    JsonValue triggers = JsonValue::array();
    triggers.push(JsonValue::string(" "));
    triggers.push(JsonValue::string("."));
    triggers.push(JsonValue::string("("));
    comp.set("triggerCharacters", triggers);
    caps.set("completionProvider", comp);

    caps.set("hoverProvider", JsonValue::boolean(true));

    JsonValue result = JsonValue::object();
    result.set("capabilities", caps);

    JsonValue server_info = JsonValue::object();
    server_info.set("name", JsonValue::string("alphabet-lsp"));
    server_info.set("version", JsonValue::string("2.1.0"));
    result.set("serverInfo", server_info);

    return result;
}

void LanguageServer::handle_did_open(const JsonValue &params)
{
    std::string uri = params.get("textDocument").get_str("uri");
    std::string content = params.get("textDocument").get_str("text");
    documents_[uri] = content;
    publish_diagnostics(uri, content);
}

void LanguageServer::handle_did_change(const JsonValue &params)
{
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue &changes = params.get("contentChanges");
    if (changes.type == JsonValue::ARRAY_T && !changes.arr_val.empty()) {
        std::string content = changes.arr_val[0].get_str("text");
        documents_[uri] = content;
        publish_diagnostics(uri, content);
    }
}

void LanguageServer::publish_diagnostics(const std::string &uri, const std::string &content)
{
    JsonValue diags = JsonValue::array();

    if (content.find("#alphabet<") == std::string::npos) {
        JsonValue d = JsonValue::object();
        JsonValue range = JsonValue::object();
        JsonValue start = JsonValue::object();
        start.set("line", JsonValue::integer(0));
        start.set("character", JsonValue::integer(0));
        JsonValue end = JsonValue::object();
        end.set("line", JsonValue::integer(0));
        end.set("character", JsonValue::integer(10));
        range.set("start", start);
        range.set("end", end);
        d.set("range", range);
        d.set("severity", JsonValue::integer(1));
        d.set("message", JsonValue::string("Missing required header '#alphabet<lang>' on line 1"));
        diags.push(d);
    }
    else {
        // Try to parse for real errors
        try {
            Lexer lexer(content);
            auto tokens = lexer.scan_tokens();
            Parser parser(tokens, content);
            parser.parse();

            if (parser.had_errors() && !parser.first_error().empty()) {
                // Extract line/column from error message "Error at line X, column Y: ..."
                std::string err = parser.first_error();
                int err_line = 0, err_col = 0;
                size_t lp = err.find("line ");
                if (lp != std::string::npos) {
                    err_line = std::stoi(err.substr(lp + 5)) - 1; // 0-based
                }
                size_t cp = err.find("column ");
                if (cp != std::string::npos) {
                    err_col = std::stoi(err.substr(cp + 7)) - 1; // 0-based
                }

                JsonValue d = JsonValue::object();
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(err_line));
                start.set("character", JsonValue::integer(err_col));
                JsonValue end = JsonValue::object();
                end.set("line", JsonValue::integer(err_line));
                end.set("character", JsonValue::integer(err_col + 5));
                range.set("start", start);
                range.set("end", end);
                d.set("range", range);
                d.set("severity", JsonValue::integer(1));
                // Clean up error message (remove source context lines)
                size_t newline = err.find('\n');
                std::string clean_msg =
                    (newline != std::string::npos) ? err.substr(0, newline) : err;
                d.set("message", JsonValue::string(clean_msg));
                diags.push(d);
            }
        }
        catch (...) {
            // Ignore parse errors that can't be processed
        }
    }

    JsonValue params_out = JsonValue::object();
    params_out.set("uri", JsonValue::string(uri));
    params_out.set("diagnostics", diags);
    send_notification("textDocument/publishDiagnostics", params_out);
}

JsonValue LanguageServer::handle_completion(int /*id*/, const JsonValue & /*params*/)
{
    JsonValue items = JsonValue::array();

    // Keywords
    struct
    {
        const char *label;
        const char *detail;
        const char *doc;
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
    for (auto &kw : keywords) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(kw.label));
        item.set("kind", JsonValue::integer(14)); // Keyword
        item.set("detail", JsonValue::string(kw.detail));
        item.set("documentation", JsonValue::string(kw.doc));
        items.push(item);
    }

    // Types
    struct
    {
        const char *label;
        const char *detail;
    } types[] = {
        {"0", "void"}, {"1", "i8"},    {"2", "i16"},  {"3", "i32"},   {"4", "i64"},
        {"5", "int"},  {"6", "f32"},   {"7", "f64"},  {"8", "float"}, {"11", "bool"},
        {"12", "str"}, {"13", "list"}, {"14", "map"},
    };
    for (auto &tp : types) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(tp.label));
        item.set("kind", JsonValue::integer(25)); // TypeParameter
        item.set("detail", JsonValue::string(tp.detail));
        items.push(item);
    }

    // Built-in functions
    struct
    {
        const char *label;
        const char *detail;
        const char *doc;
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
    for (auto &bi : builtins) {
        JsonValue item = JsonValue::object();
        item.set("label", JsonValue::string(bi.label));
        item.set("kind", JsonValue::integer(3)); // Function
        item.set("detail", JsonValue::string(bi.detail));
        item.set("documentation", JsonValue::string(bi.doc));
        items.push(item);
    }

    JsonValue result = JsonValue::object();
    result.set("items", items);
    result.set("isIncomplete", JsonValue::boolean(true));
    return result;
}

std::string LanguageServer::get_hover_doc(const std::string &word)
{
    static const std::unordered_map<std::string, std::string> docs = {
        {"i", "if - Conditional statement\nSyntax: i (condition) { body }"},
        {"e", "else - Alternative branch\nSyntax: e { body }"},
        {"l",
         "loop - While/for loop\nSyntax: l (cond) { body } or l (init : cond : incr) { body }"},
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

JsonValue LanguageServer::handle_hover(int /*id*/, const JsonValue &params)
{
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue &pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::null();

    // Get line content
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

    // Extract word at cursor
    int start = character, end = character;
    while (start > 0 && (std::isalnum(current_line[start - 1]) || current_line[start - 1] == '_' ||
                         current_line[start - 1] == '.'))
        --start;
    while (end < (int)current_line.size() && (std::isalnum(current_line[end]) ||
                                              current_line[end] == '_' || current_line[end] == '.'))
        ++end;
    if (start == end)
        return JsonValue::null();

    std::string word = current_line.substr(start, end - start);
    std::string doc = get_hover_doc(word);
    if (doc.empty())
        return JsonValue::null();

    JsonValue contents = JsonValue::object();
    contents.set("kind", JsonValue::string("markdown"));
    contents.set("value", JsonValue::string("```\n" + doc + "\n```"));

    JsonValue result = JsonValue::object();
    result.set("contents", contents);
    return result;
}

} // namespace lsp
} // namespace alphabet
