#ifndef ALPHABET_LSP_H
#define ALPHABET_LSP_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>

namespace alphabet {
namespace lsp {

// Lightweight JSON value (subset sufficient for LSP)
struct JsonValue {
    enum Type { NULL_T, BOOL_T, INT_T, STRING_T, ARRAY_T, OBJECT_T };
    Type type = NULL_T;
    bool bool_val = false;
    int int_val = 0;
    std::string str_val;
    std::vector<JsonValue> arr_val;
    std::unordered_map<std::string, JsonValue> obj_val;

    static JsonValue null() { JsonValue v; v.type = NULL_T; return v; }
    static JsonValue boolean(bool b) { JsonValue v; v.type = BOOL_T; v.bool_val = b; return v; }
    static JsonValue integer(int i) { JsonValue v; v.type = INT_T; v.int_val = i; return v; }
    static JsonValue string(const std::string& s) { JsonValue v; v.type = STRING_T; v.str_val = s; return v; }
    static JsonValue array() { JsonValue v; v.type = ARRAY_T; return v; }
    static JsonValue object() { JsonValue v; v.type = OBJECT_T; return v; }

    void push(const JsonValue& val) { arr_val.push_back(val); }
    void set(const std::string& key, const JsonValue& val) { obj_val[key] = val; }

    bool has(const std::string& key) const { return obj_val.count(key) > 0; }
    const JsonValue& get(const std::string& key) const {
        static JsonValue null_val;
        auto it = obj_val.find(key);
        return it != obj_val.end() ? it->second : null_val;
    }
    int get_int(const std::string& key, int default_val = 0) const {
        auto it = obj_val.find(key);
        return (it != obj_val.end() && it->second.type == INT_T) ? it->second.int_val : default_val;
    }
    std::string get_str(const std::string& key) const {
        auto it = obj_val.find(key);
        return (it != obj_val.end() && it->second.type == STRING_T) ? it->second.str_val : "";
    }

    std::string dump() const;
    static JsonValue parse(const std::string& json);
};

class LanguageServer {
public:
    LanguageServer();
    void run();

private:
    std::unordered_map<std::string, std::string> documents_;

    void send_response(int id, const JsonValue& result);
    void send_error(int id, int code, const std::string& message);
    void send_notification(const std::string& method, const JsonValue& params);
    void send_message(const std::string& body);

    JsonValue handle_initialize(int id, const JsonValue& params);
    void handle_did_open(const JsonValue& params);
    void handle_did_change(const JsonValue& params);
    JsonValue handle_completion(int id, const JsonValue& params);
    JsonValue handle_hover(int id, const JsonValue& params);

    void publish_diagnostics(const std::string& uri, const std::string& content);
    std::string get_hover_doc(const std::string& word);
};

}  // namespace lsp
}  // namespace alphabet

#endif
