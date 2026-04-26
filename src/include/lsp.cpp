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
        else if (method == "textDocument/documentSymbol") {
            send_response(id, handle_document_symbol(id, msg.get("params")));
        }
        else if (method == "textDocument/definition") {
            send_response(id, handle_definition(id, msg.get("params")));
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

    caps.set("documentSymbolProvider", JsonValue::boolean(true));
    caps.set("definitionProvider", JsonValue::boolean(true));

    JsonValue result = JsonValue::object();
    result.set("capabilities", caps);

    JsonValue server_info = JsonValue::object();
    server_info.set("name", JsonValue::string("alphabet-lsp"));
    server_info.set("version", JsonValue::string("2.3.3"));
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

// Helper: extract the word at a given position in a line
static std::string word_at(const std::string &line, int character)
{
    if (character < 0 || character >= (int)line.size())
        return "";
    int start = character, end = character;
    while (start > 0 &&
           (std::isalnum(line[start - 1]) || line[start - 1] == '_' || line[start - 1] == '.'))
        --start;
    while (end < (int)line.size() &&
           (std::isalnum(line[end]) || line[end] == '_' || line[end] == '.'))
        ++end;
    return line.substr(start, end - start);
}

// Helper: check if a character is a type-id digit start (Alphabet uses numeric type IDs)
static bool is_type_prefix(const std::string &line, size_t pos)
{
    // A type prefix is a number followed by a space and an identifier
    // e.g. "5 x =" or "12 name("  or "15 g ="
    size_t i = pos;
    if (i >= line.size() || !std::isdigit(line[i]))
        return false;
    while (i < line.size() && std::isdigit(line[i]))
        ++i;
    // Must be followed by whitespace then an identifier character
    if (i >= line.size() || line[i] != ' ')
        return false;
    while (i < line.size() && line[i] == ' ')
        ++i;
    return i < line.size() && (std::isalpha(line[i]) || line[i] == '_');
}

// LSP SymbolKind constants
static const int SK_FILE = 1;
static const int SK_CLASS = 5;
static const int SK_METHOD = 6;
static const int SK_FUNCTION = 12;
static const int SK_VARIABLE = 13;
static const int SK_CONSTANT = 14;
static const int SK_FIELD = 8;

JsonValue LanguageServer::handle_document_symbol(int /*id*/, const JsonValue &params)
{
    std::string uri = params.get("textDocument").get_str("uri");
    auto it = documents_.find(uri);
    if (it == documents_.end())
        return JsonValue::array();

    const std::string &content = it->second;
    JsonValue symbols = JsonValue::array();

    std::istringstream stream(content);
    std::string line;
    int line_num = 0;

    // Track current class for nesting
    std::string current_class;
    int class_start_line = 0;
    // Indices in the symbols array that are classes (for adding children later)
    std::vector<size_t> class_indices;

    while (std::getline(stream, line)) {
        // Skip blank lines and comments
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

        // Check for class: "c ClassName {"
        if (fc == 'c' && first_nonspace + 1 < line.size() && line[first_nonspace + 1] == ' ') {
            size_t name_start = first_nonspace + 2;
            while (name_start < line.size() && line[name_start] == ' ')
                ++name_start;
            size_t name_end = name_start;
            while (name_end < line.size() &&
                   (std::isalnum(line[name_end]) || line[name_end] == '_'))
                ++name_end;
            if (name_end > name_start) {
                std::string name = line.substr(name_start, name_end - name_start);
                current_class = name;
                class_start_line = line_num;

                JsonValue sym = JsonValue::object();
                sym.set("name", JsonValue::string(name));
                sym.set("kind", JsonValue::integer(SK_CLASS));
                JsonValue range = JsonValue::object();
                JsonValue start = JsonValue::object();
                start.set("line", JsonValue::integer(line_num));
                start.set("character", JsonValue::integer((int)first_nonspace));
                range.set("start", start);
                range.set("end", start); // simplified end
                sym.set("range", range);
                sym.set("selectionRange", range);
                sym.set("detail", JsonValue::string("class"));
                // children will be added later
                JsonValue children = JsonValue::array();
                sym.set("children", children);

                class_indices.push_back(symbols.arr_val.size());
                symbols.push(sym);
            }
        }
        // Check for method/function: "m type_id name(params)" or
        // with modifiers like "v m type_id name(" or "s v m type_id name("
        else if (line.find("m ") != std::string::npos) {
            // Find the 'm' keyword that starts a method definition
            size_t m_pos = line.find("m ");
            // Verify it's the method keyword (at word boundary, possibly after
            // modifiers v/p/s)
            bool is_method = false;
            if (m_pos == first_nonspace ||
                (m_pos > 0 && (line[m_pos - 1] == ' ' || line[m_pos - 1] == '\t'))) {
                // Check that this looks like a method: m <type> <name>(
                size_t after_m = m_pos + 2;
                while (after_m < line.size() && line[after_m] == ' ')
                    ++after_m;
                // Should have a type number then a name
                if (after_m < line.size() && std::isdigit(line[after_m])) {
                    is_method = true;
                }
            }
            if (is_method) {
                // Skip past "m <type_id> "
                size_t pos = m_pos + 2;
                while (pos < line.size() && line[pos] == ' ')
                    ++pos;
                // Skip type id (digits)
                while (pos < line.size() && std::isdigit(line[pos]))
                    ++pos;
                while (pos < line.size() && line[pos] == ' ')
                    ++pos;
                // Extract name
                size_t name_start = pos;
                while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                    ++pos;
                if (pos > name_start) {
                    std::string name = line.substr(name_start, pos - name_start);
                    bool is_static =
                        (line.find("s ") != std::string::npos && line.find("s ") < m_pos);

                    JsonValue sym = JsonValue::object();
                    sym.set("name", JsonValue::string(name));
                    sym.set("kind",
                            JsonValue::integer(current_class.empty() ? SK_FUNCTION : SK_METHOD));
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
        // Check for interface: "j InterfaceName {"
        else if (fc == 'j' && first_nonspace + 1 < line.size() && line[first_nonspace + 1] == ' ') {
            size_t name_start = first_nonspace + 2;
            while (name_start < line.size() && line[name_start] == ' ')
                ++name_start;
            size_t name_end = name_start;
            while (name_end < line.size() &&
                   (std::isalnum(line[name_end]) || line[name_end] == '_'))
                ++name_end;
            if (name_end > name_start) {
                std::string name = line.substr(name_start, name_end - name_start);

                JsonValue sym = JsonValue::object();
                sym.set("name", JsonValue::string(name));
                sym.set("kind", JsonValue::integer(SK_CLASS)); // interface ~ class
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
        // Check for const: "const name = ..."
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
        // Check for variable declaration: "<type_id> <name> = ..."
        // e.g. "5 x = 10" or "12 msg = z.o(...)"
        else if (is_type_prefix(line, first_nonspace)) {
            size_t pos = first_nonspace;
            // Skip type id
            while (pos < line.size() && std::isdigit(line[pos]))
                ++pos;
            while (pos < line.size() && line[pos] == ' ')
                ++pos;
            size_t name_start = pos;
            while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                ++pos;
            if (pos > name_start) {
                // Verify this is an assignment (has '=' after name)
                size_t after_name = pos;
                while (after_name < line.size() && line[after_name] == ' ')
                    ++after_name;
                if (after_name < line.size() && line[after_name] == '=') {
                    std::string name = line.substr(name_start, pos - name_start);

                    // Extract type id
                    std::string type_str =
                        line.substr(first_nonspace, name_start - first_nonspace - 1);

                    // Determine if it's a field (inside a class) or variable
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
        // Detect closing brace to end class scope
        else if (fc == '}' && !current_class.empty()) {
            current_class.clear();
        }

        ++line_num;
    }

    return symbols;
}

JsonValue LanguageServer::handle_definition(int /*id*/, const JsonValue &params)
{
    std::string uri = params.get("textDocument").get_str("uri");
    const JsonValue &pos = params.get("position");
    int line = pos.get_int("line");
    int character = pos.get_int("character");

    auto doc_it = documents_.find(uri);
    if (doc_it == documents_.end())
        return JsonValue::null();

    const std::string &content = doc_it->second;

    // Get the line at cursor position
    std::istringstream stream(content);
    std::string current_line;
    int ln = 0;
    while (std::getline(stream, current_line)) {
        if (ln == line)
            break;
        ++ln;
    }

    // Extract the word at cursor
    std::string word = word_at(current_line, character);
    if (word.empty())
        return JsonValue::null();

    // For dotted names like "z.o", use the whole thing for builtins but
    // for user definitions, search the base name
    std::string search_name = word;
    size_t dot_pos = word.find('.');
    if (dot_pos != std::string::npos) {
        // If it starts with 'z.', it's a builtin - no user definition
        if (word.substr(0, 2) == "z.")
            return JsonValue::null();
        search_name = word.substr(dot_pos + 1);
    }

    // Search through all lines for the definition of search_name
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

        // Check class definition: "c ClassName {"
        if (fc == 'c' && first_nonspace + 1 < def_line.size() &&
            def_line[first_nonspace + 1] == ' ') {
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

        // Check interface: "j Name {"
        if (fc == 'j' && first_nonspace + 1 < def_line.size() &&
            def_line[first_nonspace + 1] == ' ') {
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

        // Check method/function: "m <type> name("
        if (def_line.find("m ") != std::string::npos) {
            size_t m_pos = def_line.find("m ");
            bool is_method = false;
            if (m_pos == first_nonspace ||
                (m_pos > 0 && (def_line[m_pos - 1] == ' ' || def_line[m_pos - 1] == '\t'))) {
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
                while (pos2 < def_line.size() &&
                       (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
                    ++pos2;
                if (pos2 > name_start &&
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

        // Check const: "const name = ..."
        if (def_line.substr(first_nonspace, 5) == "const" &&
            (first_nonspace + 5 >= def_line.size() || def_line[first_nonspace + 5] == ' ')) {
            size_t pos2 = first_nonspace + 5;
            while (pos2 < def_line.size() && def_line[pos2] == ' ')
                ++pos2;
            size_t name_start = pos2;
            while (pos2 < def_line.size() &&
                   (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
                ++pos2;
            if (pos2 > name_start &&
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

        // Check variable declaration: "<type_id> name = ..."
        if (is_type_prefix(def_line, first_nonspace)) {
            size_t pos2 = first_nonspace;
            while (pos2 < def_line.size() && std::isdigit(def_line[pos2]))
                ++pos2;
            while (pos2 < def_line.size() && def_line[pos2] == ' ')
                ++pos2;
            size_t name_start = pos2;
            while (pos2 < def_line.size() &&
                   (std::isalnum(def_line[pos2]) || def_line[pos2] == '_'))
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

    // No definition found
    return JsonValue::null();
}

} // namespace lsp
} // namespace alphabet
