#ifndef ALPHABET_VM_H
#define ALPHABET_VM_H

#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <cstdint>
#include <memory>
#include "bytecode.h"
#include "compiler.h"

namespace alphabet {

struct AlphabetObject {
    uint16_t class_id;
    std::unordered_map<std::string, std::shared_ptr<void>> fields;

    explicit AlphabetObject(uint16_t id) : class_id(id) {}
};

using ObjectPtr = std::shared_ptr<AlphabetObject>;

struct Value {
    using List = std::vector<Value>;
    using Map = std::unordered_map<std::string, Value>;
    
    std::variant<std::monostate, double, std::string, 
                 std::shared_ptr<List>, std::shared_ptr<Map>, ObjectPtr> data;
    
    Value() : data(std::monostate{}) {}
    Value(std::nullptr_t) : data(std::monostate{}) {}
    Value(double d) : data(d) {}
    Value(const std::string& s) : data(s) {}
    Value(std::string&& s) : data(std::move(s)) {}
    Value(const List& l) : data(std::make_shared<List>(l)) {}
    Value(List&& l) : data(std::make_shared<List>(std::move(l))) {}
    Value(const Map& m) : data(std::make_shared<Map>(m)) {}
    Value(Map&& m) : data(std::make_shared<Map>(std::move(m))) {}
    Value(const ObjectPtr& o) : data(o) {}

    bool is_null() const { return std::holds_alternative<std::monostate>(data); }
    bool is_number() const { return std::holds_alternative<double>(data); }
    bool is_string() const { return std::holds_alternative<std::string>(data); }
    bool is_list() const { return std::holds_alternative<std::shared_ptr<List>>(data); }
    bool is_map() const { return std::holds_alternative<std::shared_ptr<Map>>(data); }
    bool is_object() const { return std::holds_alternative<ObjectPtr>(data); }
    
    double as_number() const { 
        if (auto* d = std::get_if<double>(&data)) return *d;
        return 0.0;
    }
    
    const std::string& as_string() const {
        static const std::string empty;
        if (auto* s = std::get_if<std::string>(&data)) return *s;
        return empty;
    }
    
    List& as_list() {
        static List empty;
        if (auto* l = std::get_if<std::shared_ptr<List>>(&data)) {
            if (*l) return **l;
        }
        return empty;
    }
    
    const List& as_list() const {
        static List empty;
        if (auto* l = std::get_if<std::shared_ptr<List>>(&data)) {
            if (*l) return **l;
        }
        return empty;
    }
    
    Map& as_map() {
        static Map empty;
        if (auto* m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m) return **m;
        }
        return empty;
    }
    
    const Map& as_map() const {
        static Map empty;
        if (auto* m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m) return **m;
        }
        return empty;
    }
    
    ObjectPtr as_object() const {
        if (auto* o = std::get_if<ObjectPtr>(&data)) return *o;
        return nullptr;
    }
};

inline bool operator==(const Value& a, const Value& b) {
    return a.data == b.data;
}

inline bool operator!=(const Value& a, const Value& b) {
    return !(a == b);
}

struct CallFrame {
    const std::vector<Instruction>* bytecode;
    size_t ip = 0;
    std::unordered_map<std::string, Value> locals;
    std::vector<std::pair<size_t, size_t>> try_stack;

    CallFrame() : bytecode(nullptr) {}
    explicit CallFrame(const std::vector<Instruction>* bc) : bytecode(bc) {}
};

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

class VM {
public:
    VM() = default;
    explicit VM(const Program& program);

    void run();
    void init(const Program& program);

private:
    std::array<Value, 65536> stack_;
    size_t stack_top_ = 0;

    std::unordered_map<std::string, Value> globals_;
    std::vector<std::string> globals_by_index_;
    std::vector<CallFrame> frames_;
    std::unordered_map<uint16_t, CompiledClass> classes_;

    void push(const Value& value);
    Value pop();
    Value& peek(size_t distance = 0);
    void run_loop();
    void execute_instruction(CallFrame& frame);
    const std::vector<Instruction>* lookup_method(const CompiledClass& cls,
                                                   const std::string& name,
                                                   const std::string& caller_class);
    void throw_exception(const Value& value);
    void system_call(const std::string& method, int arg_count);
};

std::string value_to_string(const Value& value);

}

#endif
