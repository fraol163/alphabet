

#include "vm.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace alphabet {

namespace json {

struct Parser {
    const std::string& src;
    size_t pos = 0;

    explicit Parser(const std::string& s) : src(s) {}

    char peek() {
        skip_ws();
        return pos < src.size() ? src[pos] : '\0';
    }

    char advance() { return pos < src.size() ? src[pos++] : '\0'; }

    void skip_ws() {
        while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r'))
            ++pos;
    }

    bool match(char c) {
        skip_ws();
        if (pos < src.size() && src[pos] == c) {
            ++pos;
            return true;
        }
        return false;
    }

    Value parse() {
        skip_ws();
        if (pos >= src.size())
            return Value(nullptr);
        char c = src[pos];
        if (c == '"')
            return parse_string_val();
        if (c == '{')
            return parse_object();
        if (c == '[')
            return parse_array();
        if (c == 't' || c == 'f')
            return parse_bool();
        if (c == 'n') {
            pos += 4;
            return Value(nullptr);
        }
        return parse_number();
    }

    Value parse_string_val() {
        std::string s = parse_string();
        return Value(std::move(s));
    }

    std::string parse_string() {
        if (advance() != '"')
            return "";
        std::string result;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\') {
                ++pos;
                if (pos < src.size()) {
                    switch (src[pos]) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    default:
                        result += src[pos];
                        break;
                    }
                    ++pos;
                }
            } else {
                result += src[pos++];
            }
        }
        if (pos < src.size())
            ++pos;
        return result;
    }

    Value parse_number() {
        size_t start = pos;
        if (pos < src.size() && (src[pos] == '-' || src[pos] == '+'))
            ++pos;
        while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9')
            ++pos;
        bool is_float = false;
        if (pos < src.size() && src[pos] == '.') {
            is_float = true;
            ++pos;
            while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9')
                ++pos;
        }
        if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
            is_float = true;
            ++pos;
            if (pos < src.size() && (src[pos] == '+' || src[pos] == '-'))
                ++pos;
            while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9')
                ++pos;
        }
        std::string num_str = src.substr(start, pos - start);
        try {
            if (is_float)
                return Value(std::stod(num_str));
            return Value(static_cast<int64_t>(std::stoll(num_str)));
        } catch (...) {
            return Value(0.0);
        }
    }

    Value parse_bool() {
        if (src.compare(pos, 4, "true") == 0) {
            pos += 4;
            return Value(true);
        }
        if (src.compare(pos, 5, "false") == 0) {
            pos += 5;
            return Value(false);
        }
        return Value(nullptr);
    }

    Value parse_array() {
        advance();
        std::vector<Value> arr;
        if (match(']'))
            return Value(std::move(arr));
        while (true) {
            arr.push_back(parse());
            if (match(']'))
                break;
            match(',');
        }
        return Value(std::move(arr));
    }

    Value parse_object() {
        advance();
        Value::Map obj;
        if (match('}'))
            return Value(std::move(obj));
        while (true) {
            std::string key = parse_string();
            match(':');
            obj[key] = parse();
            if (match('}'))
                break;
            match(',');
        }
        return Value(std::move(obj));
    }
};

std::string stringify(const Value& v) {
    if (v.is_null())
        return "null";
    if (v.is_bool())
        return v.as_bool() ? "true" : "false";
    if (v.is_integer())
        return std::to_string(v.as_integer());
    if (v.is_number()) {
        double d = v.as_number();
        if (d == static_cast<int64_t>(d))
            return std::to_string(static_cast<int64_t>(d));
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    if (v.is_string()) {
        std::string result = "\"";
        for (char c : v.as_string()) {
            switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
            }
        }
        result += "\"";
        return result;
    }
    if (v.is_list()) {
        std::string result = "[";
        const auto& lst = v.as_list();
        for (size_t i = 0; i < lst.size(); ++i) {
            if (i > 0)
                result += ",";
            result += stringify(lst[i]);
        }
        result += "]";
        return result;
    }
    if (v.is_map()) {
        std::string result = "{";
        const auto& mp = v.as_map();
        size_t i = 0;
        for (const auto& [k, val] : mp) {
            if (i > 0)
                result += ",";
            result += "\"" + k + "\":" + stringify(val);
            ++i;
        }
        result += "}";
        return result;
    }
    return "null";
}

} // namespace json

void VM::system_call(const std::string& method, int arg_count) {
    if (method == "o" && arg_count >= 1) {
        Value val = pop();
        std::cout << value_to_string(val) << std::endl;
        push(Value(nullptr));
    } else if (method == "i") {
        std::string input;
        std::getline(std::cin, input);
        try {
            double num = std::stod(input);
            push(Value(num));
        } catch (const std::exception&) {
            push(Value(input));
        }
    } else if (method == "t") {
        if (arg_count >= 1) {
            Value msg = pop();
            throw_exception(Value(value_to_string(msg)));
        } else {
            throw_exception(Value("Custom Error"));
        }
    } else if (method == "f" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop();
            push(Value(std::string("")));
        } else {
            Value path_val = pop();
            if (path_val.is_string()) {
                std::string path = path_val.as_string();
                if (path.find("..") != std::string::npos || (!path.empty() && path[0] == '/') ||
                    path.find('\0') != std::string::npos) {
                    push(Value(std::string("")));
                } else {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::ostringstream oss;
                        oss << file.rdbuf();
                        push(Value(oss.str()));
                    } else {
                        push(Value(std::string("")));
                    }
                }
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "fw" && arg_count >= 2) {
        if (sandbox_mode_) {
            pop();
            pop();
            push(Value(0.0));
        } else {
            Value content_val = pop();
            Value path_val = pop();
            if (path_val.is_string() && content_val.is_string()) {
                std::string path = path_val.as_string();
                if (path.find("..") != std::string::npos || (!path.empty() && path[0] == '/') ||
                    path.find('\0') != std::string::npos) {
                    push(Value(0.0));
                } else {
                    std::ofstream file(path);
                    if (file.is_open()) {
                        file << content_val.as_string();
                        push(Value(1.0));
                    } else {
                        push(Value(0.0));
                    }
                }
            } else {
                push(Value(0.0));
            }
        }
    }

    else if (method == "sqrt" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::sqrt(v.as_number()) : 0.0));
    } else if (method == "sin" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::sin(v.as_number()) : 0.0));
    } else if (method == "cos" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::cos(v.as_number()) : 0.0));
    } else if (method == "tan" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::tan(v.as_number()) : 0.0));
    } else if (method == "abs" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::fabs(v.as_number()) : 0.0));
    } else if (method == "floor" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::floor(v.as_number()) : 0.0));
    } else if (method == "ceil" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::ceil(v.as_number()) : 0.0));
    } else if (method == "round" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::round(v.as_number()) : 0.0));
    } else if (method == "pow" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        push(Value(a.is_number() && b.is_number() ? std::pow(a.as_number(), b.as_number()) : 0.0));
    } else if (method == "min" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        if (a.is_number() && b.is_number())
            push(Value(std::min(a.as_number(), b.as_number())));
        else if (a.is_number())
            push(a);
        else if (b.is_number())
            push(b);
        else
            throw_exception(Value("min requires at least one number argument"));
    } else if (method == "max" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        if (a.is_number() && b.is_number())
            push(Value(std::max(a.as_number(), b.as_number())));
        else if (a.is_number())
            push(a);
        else if (b.is_number())
            push(b);
        else
            throw_exception(Value("max requires at least one number argument"));
    } else if (method == "log" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::log(v.as_number()) : 0.0));
    } else if (method == "log10" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::log10(v.as_number()) : 0.0));
    }

    else if (method == "len" && arg_count >= 1) {
        Value v = pop();
        if (v.is_string()) {
            const std::string& s = v.as_string();
            size_t count = 0;
            for (unsigned char c : s) {
                if ((c & 0xC0) != 0x80)
                    ++count;
            }
            push(Value(static_cast<double>(count)));
        } else if (v.is_list())
            push(Value(static_cast<double>(v.as_list().size())));
        else if (v.is_map())
            push(Value(static_cast<double>(v.as_map().size())));
        else
            push(Value(0.0));
    } else if (method == "tostr" && arg_count >= 1) {
        Value v = pop();
        push(Value(value_to_string(v)));
    } else if (method == "tonum" && arg_count >= 1) {
        Value v = pop();
        if (v.is_number()) {
            push(v);
        } else if (v.is_bool()) {
            push(Value(v.as_bool() ? 1.0 : 0.0));
        } else if (v.is_string()) {
            try {
                push(Value(std::stod(v.as_string())));
            } catch (const std::exception&) {
                push(Value(0.0));
            }
        } else {
            push(Value(0.0));
        }
    } else if (method == "type" && arg_count >= 1) {
        Value v = pop();
        if (v.is_null())
            push(Value(std::string("null")));
        else if (v.is_bool())
            push(Value(std::string("bool")));
        else if (v.is_integer())
            push(Value(std::string("integer")));
        else if (v.is_number())
            push(Value(std::string("number")));
        else if (v.is_string())
            push(Value(std::string("string")));
        else if (v.is_list())
            push(Value(std::string("list")));
        else if (v.is_map())
            push(Value(std::string("map")));
        else if (v.is_object())
            push(Value(std::string("object")));
        else
            push(Value(std::string("unknown")));
    }

    else if (method == "split" && arg_count >= 2) {
        Value delim = pop();
        Value str = pop();
        if (str.is_string() && delim.is_string()) {
            std::vector<Value> result;
            std::string s = str.as_string();
            std::string d = delim.as_string();
            if (d.empty()) {
                for (char c : s)
                    result.push_back(Value(std::string(1, c)));
            } else {
                size_t pos = 0, found;
                while ((found = s.find(d, pos)) != std::string::npos) {
                    result.push_back(Value(s.substr(pos, found - pos)));
                    pos = found + d.size();
                }
                result.push_back(Value(s.substr(pos)));
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "join" && arg_count >= 2) {
        Value sep = pop();
        Value list = pop();
        if (list.is_list() && sep.is_string()) {
            const auto& items = list.as_list();
            std::string separator = sep.as_string();
            std::ostringstream oss;
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0)
                    oss << separator;
                oss << value_to_string(items[i]);
            }
            push(Value(oss.str()));
        } else {
            push(Value(std::string("")));
        }
    } else if (method == "replace" && arg_count >= 3) {
        Value new_val = pop();
        Value old_val = pop();
        Value str = pop();
        if (str.is_string() && old_val.is_string() && new_val.is_string()) {
            std::string s = str.as_string();
            std::string old_str = old_val.as_string();
            std::string new_str = new_val.as_string();
            if (!old_str.empty()) {
                size_t pos = 0;
                while ((pos = s.find(old_str, pos)) != std::string::npos) {
                    s.replace(pos, old_str.size(), new_str);
                    pos += new_str.size();
                }
            }
            push(Value(std::move(s)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "trim" && arg_count >= 1) {
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            size_t start = s.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) {
                push(Value(std::string("")));
                return;
            }
            size_t end = s.find_last_not_of(" \t\n\r");
            push(Value(s.substr(start, end - start + 1)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "upper" && arg_count >= 1) {
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            push(Value(std::move(s)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "lower" && arg_count >= 1) {
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            push(Value(std::move(s)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "substr" && arg_count >= 2) {
        if (arg_count >= 3) {
            Value len_val = pop();
            Value start_val = pop();
            Value str_val = pop();
            if (str_val.is_string() && start_val.is_number()) {
                std::string s = str_val.as_string();
                size_t start_idx = static_cast<size_t>(start_val.as_number());
                size_t sub_len = len_val.is_number() ? static_cast<size_t>(len_val.as_number()) : std::string::npos;
                if (start_idx < s.size()) {
                    push(Value(s.substr(start_idx, sub_len)));
                } else {
                    push(Value(std::string("")));
                }
            } else {
                push(Value(std::string("")));
            }
        } else {
            Value start_val = pop();
            Value str_val = pop();
            if (str_val.is_string() && start_val.is_number()) {
                std::string s = str_val.as_string();
                size_t start_idx = static_cast<size_t>(start_val.as_number());
                if (start_idx < s.size()) {
                    push(Value(s.substr(start_idx)));
                } else {
                    push(Value(std::string("")));
                }
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "chr" && arg_count >= 1) {
        Value v = pop();
        if (v.is_number()) {
            push(Value(std::string(1, static_cast<char>(v.as_number()))));
        } else {
            push(Value(std::string("")));
        }
    } else if (method == "ord" && arg_count >= 1) {
        Value v = pop();
        if (v.is_string() && !v.as_string().empty()) {
            push(Value(static_cast<double>(static_cast<unsigned char>(v.as_string()[0]))));
        } else {
            push(Value(0.0));
        }
    } else if (method == "starts_with" && arg_count >= 2) {
        Value prefix = pop();
        Value str = pop();
        if (str.is_string() && prefix.is_string()) {
            const auto& s = str.as_string();
            const auto& p = prefix.as_string();
            push(Value(s.size() >= p.size() && s.compare(0, p.size(), p) == 0 ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "ends_with" && arg_count >= 2) {
        Value suffix = pop();
        Value str = pop();
        if (str.is_string() && suffix.is_string()) {
            const auto& s = str.as_string();
            const auto& suf = suffix.as_string();
            push(Value(s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0 ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "find" && arg_count >= 2) {
        Value needle = pop();
        Value haystack = pop();
        if (haystack.is_string() && needle.is_string()) {
            const auto& s = haystack.as_string();
            const auto& n = needle.as_string();
            size_t pos = s.find(n);
            push(Value(pos != std::string::npos ? static_cast<double>(pos) : -1.0));
        } else if (haystack.is_list()) {
            const auto& lst = haystack.as_list();
            for (size_t i = 0; i < lst.size(); ++i) {
                if (lst[i] == needle) {
                    push(Value(static_cast<double>(i)));
                    return;
                }
            }
            push(Value(-1.0));
        } else {
            push(Value(-1.0));
        }
    } else if (method == "count" && arg_count >= 2) {
        Value needle = pop();
        Value haystack = pop();
        if (haystack.is_string() && needle.is_string()) {
            const auto& s = haystack.as_string();
            const auto& n = needle.as_string();
            size_t count = 0;
            size_t pos = 0;
            if (!n.empty()) {
                while ((pos = s.find(n, pos)) != std::string::npos) {
                    ++count;
                    pos += n.size();
                }
            }
            push(Value(static_cast<double>(count)));
        } else if (haystack.is_list()) {
            const auto& lst = haystack.as_list();
            size_t count = 0;
            for (const auto& item : lst) {
                if (item == needle)
                    ++count;
            }
            push(Value(static_cast<double>(count)));
        } else {
            push(Value(0.0));
        }
    }

    else if (method == "range") {
        double start = 0, stop = 0, step = 1;
        if (arg_count == 1) {
            Value v = pop();
            stop = v.is_number() ? v.as_number() : 0;
        } else if (arg_count == 2) {
            Value v_stop = pop();
            Value v_start = pop();
            start = v_start.is_number() ? v_start.as_number() : 0;
            stop = v_stop.is_number() ? v_stop.as_number() : 0;
        } else if (arg_count >= 3) {
            Value v_step = pop();
            Value v_stop = pop();
            Value v_start = pop();
            start = v_start.is_number() ? v_start.as_number() : 0;
            stop = v_stop.is_number() ? v_stop.as_number() : 0;
            step = v_step.is_number() ? v_step.as_number() : 1;
        }
        if (step == 0)
            step = 1;
        constexpr double MAX_RANGE_SIZE = 1000000.0;
        double range_size = (step > 0) ? ((stop - start) / step) : ((start - stop) / (-step));
        if (range_size > MAX_RANGE_SIZE) {
            throw RuntimeError("Range too large: " + std::to_string((int64_t)range_size) + " elements (max " +
                               std::to_string((int64_t)MAX_RANGE_SIZE) + ")");
        }
        std::vector<Value> result;
        if (step > 0) {
            for (double i = start; i < stop; i += step) {
                result.push_back(Value(i));
            }
        } else {
            for (double i = start; i > stop; i += step) {
                result.push_back(Value(i));
            }
        }
        push(Value(std::move(result)));
    } else if (method == "append" && arg_count >= 2) {
        Value val = pop();
        Value list_val = pop();
        if (list_val.is_list()) {
            list_val.as_list().push_back(val);
            push(list_val);
        } else {
            push(Value(std::vector<Value>{val}));
        }
    } else if (method == "pop_back" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list() && !list_val.as_list().empty()) {
            auto& lst = list_val.as_list();
            Value back = lst.back();
            lst.pop_back();
            push(back);
        } else {
            push(Value(nullptr));
        }
    } else if (method == "contains" && arg_count >= 2) {
        Value needle = pop();
        Value haystack = pop();
        if (haystack.is_list()) {
            const auto& lst = haystack.as_list();
            bool found = std::any_of(lst.begin(), lst.end(), [&needle](const Value& item) { return item == needle; });
            push(Value(found ? 1.0 : 0.0));
        } else if (haystack.is_string() && needle.is_string()) {
            push(Value(haystack.as_string().find(needle.as_string()) != std::string::npos ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "keys" && arg_count >= 1) {
        Value map_val = pop();
        if (map_val.is_map()) {
            std::vector<Value> result;
            for (const auto& [k, _] : map_val.as_map()) {
                result.push_back(Value(k));
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "values" && arg_count >= 1) {
        Value map_val = pop();
        if (map_val.is_map()) {
            std::vector<Value> result;
            for (const auto& [_, v] : map_val.as_map()) {
                result.push_back(v);
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "builder") {
        push(Value(std::vector<Value>()));
    } else if (method == "set") {
        push(Value(std::vector<Value>()));
    } else if (method == "add" && arg_count >= 2) {
        Value val = pop();
        Value set_val = pop();
        if (set_val.is_list()) {
            auto& lst = set_val.as_list();
            bool found = false;
            for (const auto& item : lst) {
                if (item == val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                lst.push_back(val);
            }
            push(set_val);
        } else {
            push(set_val);
        }
    } else if (method == "has" && arg_count >= 2) {
        Value val = pop();
        Value set_val = pop();
        if (set_val.is_list()) {
            const auto& lst = set_val.as_list();
            bool found = false;
            for (const auto& item : lst) {
                if (item == val) {
                    found = true;
                    break;
                }
            }
            push(Value(found ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "set_size" && arg_count >= 1) {
        Value set_val = pop();
        if (set_val.is_list()) {
            push(Value(static_cast<double>(set_val.as_list().size())));
        } else {
            push(Value(0.0));
        }
    } else if (method == "append_str" && arg_count >= 2) {
        Value text = pop();
        Value sb = pop();
        if (sb.is_list()) {
            sb.as_list().push_back(Value(value_to_string(text)));
            push(sb);
        } else {
            push(sb);
        }
    } else if (method == "build" && arg_count >= 1) {
        Value sb = pop();
        if (sb.is_list()) {
            std::ostringstream oss;
            for (const auto& part : sb.as_list()) {
                oss << value_to_string(part);
            }
            push(Value(oss.str()));
        } else {
            push(Value(value_to_string(sb)));
        }
    } else if (method == "reverse" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            auto& lst = list_val.as_list();
            std::reverse(lst.begin(), lst.end());
            push(list_val);
        } else if (list_val.is_string()) {
            std::string s = list_val.as_string();
            std::reverse(s.begin(), s.end());
            push(Value(std::move(s)));
        } else {
            push(list_val);
        }
    } else if (method == "sort" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            auto& lst = list_val.as_list();
            std::sort(lst.begin(), lst.end(), [](const Value& a, const Value& b) {
                if (a.is_number() && b.is_number())
                    return a.as_number() < b.as_number();
                if (a.is_string() && b.is_string())
                    return a.as_string() < b.as_string();
                return false;
            });
            push(list_val);
        } else {
            push(list_val);
        }
    } else if (method == "insert" && arg_count >= 3) {
        Value val = pop();
        Value idx_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && idx_val.is_number()) {
            auto& lst = list_val.as_list();
            size_t idx = static_cast<size_t>(idx_val.as_number());
            if (idx <= lst.size()) {
                lst.insert(lst.begin() + idx, val);
            }
            push(list_val);
        } else {
            push(list_val);
        }
    } else if (method == "remove" && arg_count >= 2) {
        Value idx_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && idx_val.is_number()) {
            auto& lst = list_val.as_list();
            size_t idx = static_cast<size_t>(idx_val.as_number());
            if (idx < lst.size()) {
                Value removed = lst[idx];
                lst.erase(lst.begin() + idx);
                push(removed);
            } else {
                push(Value(nullptr));
            }
        } else {
            push(Value(nullptr));
        }
    } else if (method == "fa" && arg_count >= 2) {
        if (sandbox_mode_) {
            pop();
            pop();
            push(Value(0.0));
        } else {
            Value content_val = pop();
            Value path_val = pop();
            if (path_val.is_string() && content_val.is_string()) {
                std::string path = path_val.as_string();
                if (path.find("..") != std::string::npos || (!path.empty() && path[0] == '/') ||
                    path.find('\0') != std::string::npos) {
                    push(Value(0.0));
                } else {
                    std::ofstream file(path, std::ios::app);
                    if (file.is_open()) {
                        file << content_val.as_string();
                        push(Value(1.0));
                    } else {
                        push(Value(0.0));
                    }
                }
            } else {
                push(Value(0.0));
            }
        }
    } else if (method == "exists" && arg_count >= 1) {
        Value path_val = pop();
        if (path_val.is_string()) {
            std::string path = path_val.as_string();
            if (path.find("..") != std::string::npos || (!path.empty() && path[0] == '/') ||
                path.find('\0') != std::string::npos) {
                push(Value(0.0));
            } else {
                std::ifstream file(path);
                push(Value(file.good() ? 1.0 : 0.0));
            }
        } else {
            push(Value(0.0));
        }
    } else if (method == "file_size" && arg_count >= 1) {
        Value path_val = pop();
        if (path_val.is_string()) {
            std::string path = path_val.as_string();
            if (path.find("..") != std::string::npos || (!path.empty() && path[0] == '/') ||
                path.find('\0') != std::string::npos) {
                push(Value(-1.0));
            } else {
                std::ifstream file(path, std::ios::binary | std::ios::ate);
                if (file.good()) {
                    push(Value(static_cast<double>(file.tellg())));
                } else {
                    push(Value(-1.0));
                }
            }
        } else {
            push(Value(-1.0));
        }
    } else if (method == "args") {
        std::vector<Value> result;
        for (const auto& arg : program_args_) {
            result.push_back(Value(arg));
        }
        push(Value(std::move(result)));
    } else if (method == "exit" && arg_count >= 1) {
        Value code_val = pop();
        exit_code_ = code_val.is_number() ? static_cast<int>(code_val.as_number()) : 0;
        frames_.clear();
        return;
    } else if (method == "sleep" && arg_count >= 1) {
        Value ms_val = pop();
        if (ms_val.is_number()) {
            int64_t ms = static_cast<int64_t>(ms_val.as_number());
            if (ms > 0 && ms <= 300000) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            }
            push(Value(nullptr));
        } else {
            push(Value(nullptr));
        }
    } else if (method == "thread" && arg_count >= 1) {
        // z.thread(lambda_name) — create a new thread with own VM context
        Value fn_val = pop();
        if (fn_val.is_string()) {
            std::string fn_name = fn_val.as_string();
            auto it = global_functions_.find(fn_name);
            if (it != global_functions_.end()) {
                // Capture globals for thread
                auto globals_copy = globals_;
                auto fns_copy = global_functions_;
                auto classes_copy = classes_;
                auto const_pool = constant_pool_;

                std::thread t([globals_copy, fns_copy, classes_copy, const_pool, fn_name, this]() {
                    try {
                        // Create a minimal VM state for the thread
                        std::vector<Value> empty_args;

                        // Build a tiny program with just the lambda
                        Program thread_prog;
                        thread_prog.constant_pool = const_pool;
                        auto fn_it = fns_copy.find(fn_name);
                        if (fn_it != fns_copy.end()) {
                            thread_prog.main = fn_it->second.bytecode;
                        }

                        // Create thread-local VM
                        VM thread_vm(thread_prog);
                        thread_vm.set_globals(globals_copy);
                        thread_vm.set_sandbox_mode(sandbox_mode_);

                        std::vector<Value> args;
                        thread_vm.call_lambda_public(fn_name, args, fns_copy);

                        // Copy back globals
                        {
                            std::lock_guard<std::mutex> lg(globals_mutex_);
                            for (const auto& [k, v] : thread_vm.get_globals()) {
                                globals_[k] = v;
                            }
                        }
                    } catch (...) {
                        // Thread exceptions are silently caught
                    }
                });
                threads_.push_back(std::move(t));
                push(Value(static_cast<double>(threads_.size() - 1)));
            } else {
                push(Value(-1.0));
            }
        } else {
            push(Value(-1.0));
        }
    } else if (method == "join" && arg_count >= 1) {
        // z.join(thread_id) — wait for thread to finish
        Value tid_val = pop();
        if (tid_val.is_number()) {
            size_t tid = static_cast<size_t>(tid_val.as_number());
            if (tid < threads_.size() && threads_[tid].joinable()) {
                threads_[tid].join();
            }
        }
        push(Value(nullptr));
    } else if (method == "join_all") {
        // z.join_all() — wait for all threads
        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
        threads_.clear();
        push(Value(nullptr));
    } else if (method == "lock" && arg_count >= 1) {
        // z.lock(name) — create a named mutex
        Value name_val = pop();
        if (name_val.is_string()) {
            std::string name = name_val.as_string();
            std::lock_guard<std::mutex> lg(locks_mutex_);
            locks_[name]; // Create mutex if not exists
        }
        push(Value(nullptr));
    } else if (method == "acquire" && arg_count >= 1) {
        // z.acquire(name) — lock a named mutex
        Value name_val = pop();
        if (name_val.is_string()) {
            std::string name = name_val.as_string();
            std::lock_guard<std::mutex> lg(locks_mutex_);
            auto it = locks_.find(name);
            if (it != locks_.end()) {
                it->second.lock();
            }
        }
        push(Value(nullptr));
    } else if (method == "release" && arg_count >= 1) {
        // z.release(name) — unlock a named mutex
        Value name_val = pop();
        if (name_val.is_string()) {
            std::string name = name_val.as_string();
            std::lock_guard<std::mutex> lg(locks_mutex_);
            auto it = locks_.find(name);
            if (it != locks_.end()) {
                it->second.unlock();
            }
        }
        push(Value(nullptr));
    } else if (method == "http_get" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop();
            push(Value(std::string("")));
        } else {
            Value url_val = pop();
            if (url_val.is_string()) {
                std::string url = url_val.as_string();
                std::string result;
                std::string cmd = "curl -sS --max-time 10 " + url + " 2>&1";
                FILE* pipe = popen(cmd.c_str(), "r");
                if (pipe) {
                    char buffer[4096];
                    while (fgets(buffer, sizeof(buffer), pipe)) {
                        result += buffer;
                    }
                    int rc = pclose(pipe);
                    if (rc != 0 && result.empty()) {
                        result = "";
                    }
                }
                push(Value(std::move(result)));
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "http_post" && arg_count >= 2) {
        if (sandbox_mode_) {
            pop();
            pop();
            push(Value(std::string("")));
        } else {
            Value body_val = pop();
            Value url_val = pop();
            if (url_val.is_string() && body_val.is_string()) {
                std::string url = url_val.as_string();
                std::string body = body_val.as_string();
                std::string result;
                std::string tmpfile = "/tmp/alpha_http_post_" + std::to_string(reinterpret_cast<uintptr_t>(this));
                {
                    std::ofstream tmp(tmpfile);
                    tmp << body;
                }
                std::string cmd = "curl -sS --max-time 10 -X POST -H \"Content-Type: application/json\" -d @" +
                                  tmpfile + " " + url + " 2>&1";
                FILE* pipe = popen(cmd.c_str(), "r");
                if (pipe) {
                    char buffer[4096];
                    while (fgets(buffer, sizeof(buffer), pipe)) {
                        result += buffer;
                    }
                    pclose(pipe);
                }
                std::remove(tmpfile.c_str());
                push(Value(std::move(result)));
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "timestamp") {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        push(Value(static_cast<double>(ms)));
    } else if (method == "env" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop();
            push(Value(std::string("")));
        } else {
            Value name_val = pop();
            if (name_val.is_string()) {
                const char* val = std::getenv(name_val.as_string().c_str());
                push(Value(val ? std::string(val) : std::string("")));
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "json_parse" && arg_count >= 1) {
        Value str_val = pop();
        if (str_val.is_string()) {
            json::Parser parser(str_val.as_string());
            push(parser.parse());
        } else {
            push(Value(nullptr));
        }
    } else if (method == "json_stringify" && arg_count >= 1) {
        Value val = pop();
        push(Value(json::stringify(val)));
    } else if (method == "exec" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop();
            push(Value(std::string("")));
        } else {
            Value cmd_val = pop();
            if (cmd_val.is_string()) {
                std::string cmd = cmd_val.as_string();
                std::string result;
                FILE* pipe = popen(cmd.c_str(), "r");
                if (pipe) {
                    char buffer[4096];
                    while (fgets(buffer, sizeof(buffer), pipe)) {
                        result += buffer;
                    }
                    int rc = pclose(pipe);
                    if (rc != 0 && result.empty()) {
                        result = "";
                    }
                }
                push(Value(std::move(result)));
            } else {
                push(Value(std::string("")));
            }
        }
    } else if (method == "system" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop();
            push(Value(0.0));
        } else {
            Value cmd_val = pop();
            if (cmd_val.is_string()) {
                int rc = std::system(cmd_val.as_string().c_str());
                push(Value(static_cast<double>(rc)));
            } else {
                push(Value(-1.0));
            }
        }
    } else if (method == "assert" && arg_count >= 1) {
        std::string msg = "Assertion failed";
        if (arg_count >= 2) {
            Value msg_val = pop();
            if (msg_val.is_string())
                msg = msg_val.as_string();
        }
        Value cond = pop();
        if (!cond.as_bool()) {
            throw RuntimeError(msg);
        }
        push(Value(nullptr));
    } else if (method == "assert_eq" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        if (!(a == b)) {
            throw RuntimeError("Assertion failed: " + value_to_string(a) + " != " + value_to_string(b));
        }
        push(Value(nullptr));
    } else if (method == "rand") {
        static bool seeded = false;
        if (!seeded) {
            srand(static_cast<unsigned>(time(nullptr)));
            seeded = true;
        }
        push(Value(static_cast<double>(rand()) / RAND_MAX));
    } else if (method == "randint" && arg_count >= 2) {
        Value max_val = pop();
        Value min_val = pop();
        static bool seeded = false;
        if (!seeded) {
            srand(static_cast<unsigned>(time(nullptr)));
            seeded = true;
        }
        int lo = min_val.is_number() ? static_cast<int>(min_val.as_number()) : 0;
        int hi = max_val.is_number() ? static_cast<int>(max_val.as_number()) : 100;
        if (lo > hi)
            std::swap(lo, hi);
        push(Value(static_cast<double>(lo + rand() % (hi - lo + 1))));
    } else if (method == "slice") {
        if (arg_count >= 3) {
            Value end_val = pop();
            Value start_val = pop();
            Value obj_val = pop();
            int64_t start = start_val.is_number() ? static_cast<int64_t>(start_val.as_number()) : 0;
            int64_t end = end_val.is_number() ? static_cast<int64_t>(end_val.as_number()) : 0;
            if (obj_val.is_list()) {
                const auto& lst = obj_val.as_list();
                if (end < 0)
                    end += static_cast<int64_t>(lst.size());
                if (start < 0)
                    start += static_cast<int64_t>(lst.size());
                if (start < 0)
                    start = 0;
                if (end > static_cast<int64_t>(lst.size()))
                    end = static_cast<int64_t>(lst.size());
                if (start > end)
                    start = end;
                std::vector<Value> result(lst.begin() + start, lst.begin() + end);
                push(Value(std::move(result)));
            } else if (obj_val.is_string()) {
                const auto& s = obj_val.as_string();
                if (end < 0)
                    end += static_cast<int64_t>(s.size());
                if (start < 0)
                    start += static_cast<int64_t>(s.size());
                if (start < 0)
                    start = 0;
                if (end > static_cast<int64_t>(s.size()))
                    end = static_cast<int64_t>(s.size());
                if (start > end)
                    start = end;
                push(Value(s.substr(start, end - start)));
            } else {
                push(Value(std::vector<Value>()));
            }
        } else if (arg_count >= 2) {
            Value start_val = pop();
            Value obj_val = pop();
            int64_t start = start_val.is_number() ? static_cast<int64_t>(start_val.as_number()) : 0;
            if (obj_val.is_list()) {
                const auto& lst = obj_val.as_list();
                if (start < 0)
                    start += static_cast<int64_t>(lst.size());
                if (start < 0)
                    start = 0;
                if (start > static_cast<int64_t>(lst.size()))
                    start = static_cast<int64_t>(lst.size());
                std::vector<Value> result(lst.begin() + start, lst.end());
                push(Value(std::move(result)));
            } else if (obj_val.is_string()) {
                const auto& s = obj_val.as_string();
                if (start < 0)
                    start += static_cast<int64_t>(s.size());
                if (start < 0)
                    start = 0;
                if (start > static_cast<int64_t>(s.size()))
                    start = static_cast<int64_t>(s.size());
                push(Value(s.substr(start)));
            } else {
                push(Value(std::vector<Value>()));
            }
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "flatten" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            std::vector<Value> result;
            std::function<void(const std::vector<Value>&)> flatten_impl;
            flatten_impl = [&](const std::vector<Value>& lst) {
                for (const auto& item : lst) {
                    if (item.is_list()) {
                        flatten_impl(item.as_list());
                    } else {
                        result.push_back(item);
                    }
                }
            };
            flatten_impl(list_val.as_list());
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>{list_val}));
        }
    } else if (method == "is_null" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_null() ? 1.0 : 0.0));
    } else if (method == "is_empty" && arg_count >= 1) {
        Value v = pop();
        if (v.is_list()) {
            push(Value(v.as_list().empty() ? 1.0 : 0.0));
        } else if (v.is_string()) {
            push(Value(v.as_string().empty() ? 1.0 : 0.0));
        } else if (v.is_map()) {
            push(Value(v.as_map().empty() ? 1.0 : 0.0));
        } else if (v.is_null()) {
            push(Value(1.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "clamp" && arg_count >= 3) {
        Value max_val = pop();
        Value min_val = pop();
        Value val = pop();
        if (val.is_number() && min_val.is_number() && max_val.is_number()) {
            double v = val.as_number();
            double lo = min_val.as_number();
            double hi = max_val.as_number();
            if (v < lo)
                v = lo;
            if (v > hi)
                v = hi;
            push(Value(v));
        } else {
            push(val);
        }
    } else if (method == "swap" && arg_count >= 3) {
        Value j_val = pop();
        Value i_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && i_val.is_number() && j_val.is_number()) {
            auto& lst = list_val.as_list();
            int64_t i = static_cast<int64_t>(i_val.as_number());
            int64_t j = static_cast<int64_t>(j_val.as_number());
            if (i < 0)
                i += static_cast<int64_t>(lst.size());
            if (j < 0)
                j += static_cast<int64_t>(lst.size());
            if (i >= 0 && static_cast<size_t>(i) < lst.size() && j >= 0 && static_cast<size_t>(j) < lst.size()) {
                std::swap(lst[static_cast<size_t>(i)], lst[static_cast<size_t>(j)]);
            }
            push(list_val);
        } else {
            push(list_val);
        }
    } else if (method == "unique" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            const auto& lst = list_val.as_list();
            std::vector<Value> result;
            for (const auto& item : lst) {
                bool found = false;
                for (const auto& existing : result) {
                    if (existing == item) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    result.push_back(item);
                }
            }
            push(Value(std::move(result)));
        } else {
            push(list_val);
        }
    } else if (method == "zip" && arg_count >= 2) {
        Value b_val = pop();
        Value a_val = pop();
        if (a_val.is_list() && b_val.is_list()) {
            const auto& a = a_val.as_list();
            const auto& b = b_val.as_list();
            size_t min_len = std::min(a.size(), b.size());
            std::vector<Value> result;
            for (size_t i = 0; i < min_len; ++i) {
                result.push_back(Value(std::vector<Value>{a[i], b[i]}));
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "enumerate" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            const auto& lst = list_val.as_list();
            std::vector<Value> result;
            for (size_t i = 0; i < lst.size(); ++i) {
                result.push_back(Value(std::vector<Value>{Value(static_cast<double>(i)), lst[i]}));
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>()));
        }
    } else if (method == "sum" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            double total = 0;
            for (const auto& item : list_val.as_list()) {
                if (item.is_number())
                    total += item.as_number();
            }
            push(Value(total));
        } else if (list_val.is_number()) {
            push(list_val);
        } else {
            push(Value(0.0));
        }
    } else if (method == "avg" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            const auto& lst = list_val.as_list();
            if (lst.empty()) {
                push(Value(0.0));
            } else {
                double total = 0;
                for (const auto& item : lst) {
                    if (item.is_number())
                        total += item.as_number();
                }
                push(Value(total / static_cast<double>(lst.size())));
            }
        } else {
            push(Value(0.0));
        }
    } else if (method == "flatten_str" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            std::string result;
            std::function<void(const std::vector<Value>&)> flatten_impl;
            flatten_impl = [&](const std::vector<Value>& lst) {
                for (const auto& item : lst) {
                    if (item.is_list()) {
                        flatten_impl(item.as_list());
                    } else {
                        result += value_to_string(item);
                    }
                }
            };
            flatten_impl(list_val.as_list());
            push(Value(result));
        } else {
            push(Value(value_to_string(list_val)));
        }
    } else if (method == "map" && arg_count >= 2) {
        Value fn_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && fn_val.is_string()) {
            std::string fn_name = fn_val.as_string();
            const auto& lst = list_val.as_list();
            std::vector<Value> result;
            for (const auto& item : lst) {
                Value mapped = call_lambda(fn_name, {item});
                result.push_back(mapped);
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>{}));
        }
    } else if (method == "filter" && arg_count >= 2) {
        Value fn_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && fn_val.is_string()) {
            std::string fn_name = fn_val.as_string();
            const auto& lst = list_val.as_list();
            std::vector<Value> result;
            for (const auto& item : lst) {
                Value keep = call_lambda(fn_name, {item});
                bool is_true = !keep.is_null() && !(keep.is_number() && keep.as_number() == 0) &&
                               !(keep.is_integer() && keep.as_integer() == 0) && !(keep.is_bool() && !keep.as_bool()) &&
                               !(keep.is_string() && keep.as_string().empty());
                if (is_true) {
                    result.push_back(item);
                }
            }
            push(Value(std::move(result)));
        } else {
            push(Value(std::vector<Value>{}));
        }
    } else if (method == "reduce" && arg_count >= 3) {
        Value fn_val = pop();
        Value init_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && fn_val.is_string()) {
            std::string fn_name = fn_val.as_string();
            const auto& lst = list_val.as_list();
            Value acc = init_val;
            for (const auto& item : lst) {
                acc = call_lambda(fn_name, {acc, item});
            }
            push(acc);
        } else {
            push(init_val);
        }
    }
}

} // namespace alphabet
