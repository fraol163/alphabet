// vm_builtins.cpp -- Extracted VM system_call built-in methods
// Part of the Alphabet Language v2.3.3

#include "vm.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace alphabet {

void VM::system_call(const std::string &method, int arg_count)
{
    // =====================================================================
    // I/O
    // =====================================================================
    if (method == "o" && arg_count >= 1) {
        Value val = pop();
        std::cout << value_to_string(val) << std::endl;
        push(Value(nullptr));
    }
    else if (method == "i") {
        std::string input;
        std::getline(std::cin, input);
        try {
            double num = std::stod(input);
            push(Value(num));
        }
        catch (...) {
            push(Value(input));
        }
    }
    else if (method == "t") {
        if (arg_count >= 1) {
            Value msg = pop();
            throw_exception(Value(value_to_string(msg)));
        }
        else {
            throw_exception(Value("Custom Error"));
        }
    }
    else if (method == "f" && arg_count >= 1) {
        if (sandbox_mode_) {
            pop(); // discard path arg
            push(Value(std::string("")));
        }
        else {
            Value path_val = pop();
            if (path_val.is_string()) {
                std::string path = path_val.as_string();
                if (path.find("..") != std::string::npos) {
                    push(Value(std::string("")));
                }
                else {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::ostringstream oss;
                        oss << file.rdbuf();
                        push(Value(oss.str()));
                    }
                    else {
                        push(Value(std::string("")));
                    }
                }
            }
            else {
                push(Value(std::string("")));
            }
        }
    }
    else if (method == "fw" && arg_count >= 2) {
        // z.fw(path, content) -> write content to file, returns success (1/0)
        if (sandbox_mode_) {
            pop();
            pop(); // discard args
            push(Value(0.0));
        }
        else {
            Value content_val = pop();
            Value path_val = pop();
            if (path_val.is_string() && content_val.is_string()) {
                std::string path = path_val.as_string();
                if (path.find("..") != std::string::npos) {
                    push(Value(0.0));
                }
                else {
                    std::ofstream file(path);
                    if (file.is_open()) {
                        file << content_val.as_string();
                        push(Value(1.0));
                    }
                    else {
                        push(Value(0.0));
                    }
                }
            }
            else {
                push(Value(0.0));
            }
        }
    }

    // =====================================================================
    // Math
    // =====================================================================
    else if (method == "sqrt" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::sqrt(v.as_number()) : 0.0));
    }
    else if (method == "sin" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::sin(v.as_number()) : 0.0));
    }
    else if (method == "cos" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::cos(v.as_number()) : 0.0));
    }
    else if (method == "tan" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::tan(v.as_number()) : 0.0));
    }
    else if (method == "abs" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::fabs(v.as_number()) : 0.0));
    }
    else if (method == "floor" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::floor(v.as_number()) : 0.0));
    }
    else if (method == "ceil" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::ceil(v.as_number()) : 0.0));
    }
    else if (method == "round" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::round(v.as_number()) : 0.0));
    }
    else if (method == "pow" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        push(Value(a.is_number() && b.is_number() ? std::pow(a.as_number(), b.as_number()) : 0.0));
    }
    else if (method == "min" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        if (a.is_number() && b.is_number())
            push(Value(std::min(a.as_number(), b.as_number())));
        else
            push(a.is_number() ? a : b);
    }
    else if (method == "max" && arg_count >= 2) {
        Value b = pop();
        Value a = pop();
        if (a.is_number() && b.is_number())
            push(Value(std::max(a.as_number(), b.as_number())));
        else
            push(a.is_number() ? a : b);
    }
    else if (method == "log" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::log(v.as_number()) : 0.0));
    }
    else if (method == "log10" && arg_count >= 1) {
        Value v = pop();
        push(Value(v.is_number() ? std::log10(v.as_number()) : 0.0));
    }

    // =====================================================================
    // Type & Conversion
    // =====================================================================
    else if (method == "len" && arg_count >= 1) {
        Value v = pop();
        if (v.is_string())
            push(Value(static_cast<double>(v.as_string().size())));
        else if (v.is_list())
            push(Value(static_cast<double>(v.as_list().size())));
        else if (v.is_map())
            push(Value(static_cast<double>(v.as_map().size())));
        else
            push(Value(0.0));
    }
    else if (method == "tostr" && arg_count >= 1) {
        Value v = pop();
        push(Value(value_to_string(v)));
    }
    else if (method == "tonum" && arg_count >= 1) {
        Value v = pop();
        if (v.is_number()) {
            push(v);
        }
        else if (v.is_string()) {
            try {
                push(Value(std::stod(v.as_string())));
            }
            catch (...) {
                push(Value(0.0));
            }
        }
        else {
            push(Value(0.0));
        }
    }
    else if (method == "type" && arg_count >= 1) {
        Value v = pop();
        if (v.is_null())
            push(Value(std::string("null")));
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

    // =====================================================================
    // String Operations
    // =====================================================================
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
            }
            else {
                size_t pos = 0, found;
                while ((found = s.find(d, pos)) != std::string::npos) {
                    result.push_back(Value(s.substr(pos, found - pos)));
                    pos = found + d.size();
                }
                result.push_back(Value(s.substr(pos)));
            }
            push(Value(std::move(result)));
        }
        else {
            push(Value(std::vector<Value>()));
        }
    }
    else if (method == "join" && arg_count >= 2) {
        Value sep = pop();
        Value list = pop();
        if (list.is_list() && sep.is_string()) {
            const auto &items = list.as_list();
            std::string separator = sep.as_string();
            std::ostringstream oss;
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0)
                    oss << separator;
                oss << value_to_string(items[i]);
            }
            push(Value(oss.str()));
        }
        else {
            push(Value(std::string("")));
        }
    }
    else if (method == "replace" && arg_count >= 3) {
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
        }
        else {
            push(Value(value_to_string(str)));
        }
    }
    else if (method == "trim" && arg_count >= 1) {
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
        }
        else {
            push(Value(value_to_string(str)));
        }
    }
    else if (method == "upper" && arg_count >= 1) {
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            push(Value(std::move(s)));
        }
        else {
            push(Value(value_to_string(str)));
        }
    }
    else if (method == "lower" && arg_count >= 1) {
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            push(Value(std::move(s)));
        }
        else {
            push(Value(value_to_string(str)));
        }
    }
    else if (method == "substr" && arg_count >= 2) {
        if (arg_count >= 3) {
            Value len_val = pop();
            Value start_val = pop();
            Value str_val = pop();
            if (str_val.is_string() && start_val.is_number()) {
                std::string s = str_val.as_string();
                size_t start_idx = static_cast<size_t>(start_val.as_number());
                size_t sub_len = len_val.is_number() ? static_cast<size_t>(len_val.as_number())
                                                     : std::string::npos;
                if (start_idx < s.size()) {
                    push(Value(s.substr(start_idx, sub_len)));
                }
                else {
                    push(Value(std::string("")));
                }
            }
            else {
                push(Value(std::string("")));
            }
        }
        else {
            Value start_val = pop();
            Value str_val = pop();
            if (str_val.is_string() && start_val.is_number()) {
                std::string s = str_val.as_string();
                size_t start_idx = static_cast<size_t>(start_val.as_number());
                if (start_idx < s.size()) {
                    push(Value(s.substr(start_idx)));
                }
                else {
                    push(Value(std::string("")));
                }
            }
            else {
                push(Value(std::string("")));
            }
        }
    }
    else if (method == "chr" && arg_count >= 1) {
        Value v = pop();
        if (v.is_number()) {
            push(Value(std::string(1, static_cast<char>(v.as_number()))));
        }
        else {
            push(Value(std::string("")));
        }
    }
    else if (method == "ord" && arg_count >= 1) {
        Value v = pop();
        if (v.is_string() && !v.as_string().empty()) {
            push(Value(static_cast<double>(static_cast<unsigned char>(v.as_string()[0]))));
        }
        else {
            push(Value(0.0));
        }
    }
    else if (method == "starts_with" && arg_count >= 2) {
        Value prefix = pop();
        Value str = pop();
        if (str.is_string() && prefix.is_string()) {
            const auto &s = str.as_string();
            const auto &p = prefix.as_string();
            push(Value(s.size() >= p.size() && s.compare(0, p.size(), p) == 0 ? 1.0 : 0.0));
        }
        else {
            push(Value(0.0));
        }
    }
    else if (method == "ends_with" && arg_count >= 2) {
        Value suffix = pop();
        Value str = pop();
        if (str.is_string() && suffix.is_string()) {
            const auto &s = str.as_string();
            const auto &suf = suffix.as_string();
            push(Value(s.size() >= suf.size() &&
                               s.compare(s.size() - suf.size(), suf.size(), suf) == 0
                           ? 1.0
                           : 0.0));
        }
        else {
            push(Value(0.0));
        }
    }

    // =====================================================================
    // Collection Operations
    // =====================================================================
    else if (method == "range") {
        double start = 0, stop = 0, step = 1;
        if (arg_count == 1) {
            Value v = pop();
            stop = v.is_number() ? v.as_number() : 0;
        }
        else if (arg_count == 2) {
            Value v_stop = pop();
            Value v_start = pop();
            start = v_start.is_number() ? v_start.as_number() : 0;
            stop = v_stop.is_number() ? v_stop.as_number() : 0;
        }
        else if (arg_count >= 3) {
            Value v_step = pop();
            Value v_stop = pop();
            Value v_start = pop();
            start = v_start.is_number() ? v_start.as_number() : 0;
            stop = v_stop.is_number() ? v_stop.as_number() : 0;
            step = v_step.is_number() ? v_step.as_number() : 1;
        }
        if (step == 0)
            step = 1;
        std::vector<Value> result;
        if (step > 0) {
            for (double i = start; i < stop; i += step) {
                result.push_back(Value(i));
            }
        }
        else {
            for (double i = start; i > stop; i += step) {
                result.push_back(Value(i));
            }
        }
        push(Value(std::move(result)));
    }
    else if (method == "append" && arg_count >= 2) {
        Value val = pop();
        Value list_val = pop();
        if (list_val.is_list()) {
            list_val.as_list().push_back(val);
            push(list_val);
        }
        else {
            push(Value(std::vector<Value>{val}));
        }
    }
    else if (method == "pop_back" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list() && !list_val.as_list().empty()) {
            auto &lst = list_val.as_list();
            Value back = lst.back();
            lst.pop_back();
            push(back);
        }
        else {
            push(Value(nullptr));
        }
    }
    else if (method == "contains" && arg_count >= 2) {
        Value needle = pop();
        Value haystack = pop();
        if (haystack.is_list()) {
            bool found = false;
            for (const auto &item : haystack.as_list()) {
                if (item == needle) {
                    found = true;
                    break;
                }
            }
            push(Value(found ? 1.0 : 0.0));
        }
        else if (haystack.is_string() && needle.is_string()) {
            push(Value(haystack.as_string().find(needle.as_string()) != std::string::npos ? 1.0
                                                                                          : 0.0));
        }
        else {
            push(Value(0.0));
        }
    }
    else if (method == "keys" && arg_count >= 1) {
        Value map_val = pop();
        if (map_val.is_map()) {
            std::vector<Value> result;
            for (const auto &[k, _] : map_val.as_map()) {
                result.push_back(Value(k));
            }
            push(Value(std::move(result)));
        }
        else {
            push(Value(std::vector<Value>()));
        }
    }
    else if (method == "values" && arg_count >= 1) {
        Value map_val = pop();
        if (map_val.is_map()) {
            std::vector<Value> result;
            for (const auto &[_, v] : map_val.as_map()) {
                result.push_back(v);
            }
            push(Value(std::move(result)));
        }
        else {
            push(Value(std::vector<Value>()));
        }
    }
    else if (method == "reverse" && arg_count >= 1) {
        Value list_val = pop();
        if (list_val.is_list()) {
            auto lst = list_val.as_list(); // copy
            std::reverse(lst.begin(), lst.end());
            push(Value(std::move(lst)));
        }
        else if (list_val.is_string()) {
            std::string s = list_val.as_string();
            std::reverse(s.begin(), s.end());
            push(Value(std::move(s)));
        }
        else {
            push(list_val);
        }
    }
    else if (method == "insert" && arg_count >= 3) {
        // z.insert(list, index, value) -> inserts value at index
        Value val = pop();
        Value idx_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && idx_val.is_number()) {
            auto &lst = list_val.as_list();
            size_t idx = static_cast<size_t>(idx_val.as_number());
            if (idx <= lst.size()) {
                lst.insert(lst.begin() + idx, val);
            }
            push(list_val);
        }
        else {
            push(list_val);
        }
    }
    else if (method == "remove" && arg_count >= 2) {
        // z.remove(list, index) -> removes element at index, returns it
        Value idx_val = pop();
        Value list_val = pop();
        if (list_val.is_list() && idx_val.is_number()) {
            auto &lst = list_val.as_list();
            size_t idx = static_cast<size_t>(idx_val.as_number());
            if (idx < lst.size()) {
                Value removed = lst[idx];
                lst.erase(lst.begin() + idx);
                push(removed);
            }
            else {
                push(Value(nullptr));
            }
        }
        else {
            push(Value(nullptr));
        }
    }
}

} // namespace alphabet
