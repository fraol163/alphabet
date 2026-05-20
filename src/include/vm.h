#ifndef ALPHABET_VM_H
#define ALPHABET_VM_H

#include "compiler.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "bytecode.h"

namespace alphabet {

class RuntimeError : public std::runtime_error
{
  public:
    explicit RuntimeError(const std::string &msg) : std::runtime_error(msg) {}
};

struct Value;

struct AlphabetObject
{
    uint16_t class_id;
    std::unordered_map<std::string, std::shared_ptr<Value>> fields;

    explicit AlphabetObject(uint16_t id) : class_id(id) {}
};

using ObjectPtr = std::shared_ptr<AlphabetObject>;

struct Value
{
    using List = std::vector<Value>;
    using Map = std::unordered_map<std::string, Value>;

    std::variant<std::monostate, int64_t, double, bool, std::string, std::shared_ptr<List>,
                 std::shared_ptr<Map>, ObjectPtr>
        data;

    Value() : data(std::monostate{}) {}
    Value(std::nullptr_t) : data(std::monostate{}) {}
    Value(int64_t i) : data(i) {}
    Value(int i) : data(static_cast<int64_t>(i)) {}
    Value(double d) : data(d) {}
    Value(bool b) : data(b) {}
    Value(const std::string &s) : data(s) {}
    Value(std::string &&s) : data(std::move(s)) {}
    Value(const List &l) : data(std::make_shared<List>(l)) {}
    Value(List &&l) : data(std::make_shared<List>(std::move(l))) {}
    Value(const Map &m) : data(std::make_shared<Map>(m)) {}
    Value(Map &&m) : data(std::make_shared<Map>(std::move(m))) {}
    Value(const ObjectPtr &o) : data(o) {}

    bool is_null() const
    {
        return std::holds_alternative<std::monostate>(data);
    }
    bool is_integer() const
    {
        return std::holds_alternative<int64_t>(data);
    }
    bool is_number() const
    {
        return std::holds_alternative<double>(data) || std::holds_alternative<int64_t>(data);
    }
    bool is_bool() const
    {
        return std::holds_alternative<bool>(data);
    }
    bool is_string() const
    {
        return std::holds_alternative<std::string>(data);
    }
    bool is_list() const
    {
        return std::holds_alternative<std::shared_ptr<List>>(data);
    }
    bool is_map() const
    {
        return std::holds_alternative<std::shared_ptr<Map>>(data);
    }
    bool is_object() const
    {
        return std::holds_alternative<ObjectPtr>(data);
    }

    int64_t as_integer() const
    {
        if (auto *i = std::get_if<int64_t>(&data))
            return *i;
        if (auto *d = std::get_if<double>(&data))
            return static_cast<int64_t>(*d);
        if (auto *b = std::get_if<bool>(&data))
            return *b ? 1 : 0;
        return 0;
    }

    double as_number() const
    {
        if (auto *i = std::get_if<int64_t>(&data))
            return static_cast<double>(*i);
        if (auto *d = std::get_if<double>(&data))
            return *d;
        if (auto *b = std::get_if<bool>(&data))
            return *b ? 1.0 : 0.0;
        return 0.0;
    }

    bool as_bool() const
    {
        if (auto *b = std::get_if<bool>(&data))
            return *b;
        if (auto *i = std::get_if<int64_t>(&data))
            return *i != 0;
        if (auto *d = std::get_if<double>(&data))
            return *d != 0.0;
        return false;
    }

    const std::string &as_string() const
    {
        static const std::string empty;
        if (auto *s = std::get_if<std::string>(&data))
            return *s;
        return empty;
    }

    List &as_list()
    {
        if (auto *l = std::get_if<std::shared_ptr<List>>(&data)) {
            if (*l)
                return **l;
        }
        throw RuntimeError("Value is not a list");
    }

    const List &as_list() const
    {
        if (auto *l = std::get_if<std::shared_ptr<List>>(&data)) {
            if (*l)
                return **l;
        }
        throw RuntimeError("Value is not a list");
    }

    Map &as_map()
    {
        if (auto *m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m)
                return **m;
        }
        throw RuntimeError("Value is not a map");
    }

    const Map &as_map() const
    {
        if (auto *m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m)
                return **m;
        }
        throw RuntimeError("Value is not a map");
    }

    ObjectPtr as_object() const
    {
        if (auto *o = std::get_if<ObjectPtr>(&data))
            return *o;
        return nullptr;
    }
};

inline bool operator==(const Value &a, const Value &b)
{
    if (a.is_list() && b.is_list()) {
        const auto &la = a.as_list();
        const auto &lb = b.as_list();
        if (la.size() != lb.size())
            return false;
        for (size_t i = 0; i < la.size(); ++i) {
            if (!(la[i] == lb[i]))
                return false;
        }
        return true;
    }
    if (a.is_map() && b.is_map()) {
        const auto &ma = a.as_map();
        const auto &mb = b.as_map();
        if (ma.size() != mb.size())
            return false;
        for (const auto &[k, v] : ma) {
            auto it = mb.find(k);
            if (it == mb.end() || !(it->second == v))
                return false;
        }
        return true;
    }
    // Cross-type numeric comparisons (int64_t, double, bool)
    if ((a.is_integer() || a.is_number() || a.is_bool()) &&
        (b.is_integer() || b.is_number() || b.is_bool())) {
        // If both are integers, compare as integers
        if (a.is_integer() && b.is_integer())
            return a.as_integer() == b.as_integer();
        return a.as_number() == b.as_number();
    }
    return a.data == b.data;
}

inline bool operator!=(const Value &a, const Value &b)
{
    return !(a == b);
}

struct CallFrame
{
    const std::vector<Instruction> *bytecode;
    size_t ip = 0;
    std::unordered_map<std::string, Value> locals;
    std::vector<std::pair<size_t, size_t>> try_stack;
    Value post_action_value; 
    bool push_post_action_on_return = false;

    CallFrame() : bytecode(nullptr) {}
    explicit CallFrame(const std::vector<Instruction> *bc) : bytecode(bc) {}
};

class VM
{
  public:
    VM();
    ~VM();
    explicit VM(const Program &program);

    void run();
    void run_from(const Program &program, size_t bytecode_offset);
    void init(const Program &program);

    std::unordered_map<std::string, Value> get_globals() const
    {
        return globals_;
    }
    void set_globals(const std::unordered_map<std::string, Value> &g)
    {
        globals_ = g;
    }

    void set_debug_mode(bool enabled)
    {
        debug_mode_ = enabled;
    }
    void set_sandbox_mode(bool enabled)
    {
        sandbox_mode_ = enabled;
    }
    void add_breakpoint(int line)
    {
        breakpoints_.insert(line);
    }
    void remove_breakpoint(int line)
    {
        breakpoints_.erase(line);
    }

    void push(const Value &value);
    Value pop();
    Value &peek(size_t distance = 0);

    bool sandbox_mode() const
    {
        return sandbox_mode_;
    }

    void throw_exception(const Value &value);
    void mark_const(const std::string &name);

  private:
    static constexpr size_t STACK_MAX = 65536;
    std::unique_ptr<Value[]> stack_;
    size_t stack_capacity_ = STACK_MAX;
    Value *stack_ptr_;

    std::unordered_map<std::string, Value> globals_;
    std::vector<std::string> globals_by_index_;
    std::vector<CallFrame> frames_;
    std::unordered_map<uint16_t, CompiledClass> classes_;
    std::unordered_map<std::string, uint16_t> class_name_to_id_; 
    std::unordered_map<std::string, CompiledMethod> global_functions_;

    bool debug_mode_ = false;
    bool sandbox_mode_ = false;
    std::unordered_set<int> breakpoints_;
    bool step_over_ = false;

    void run_loop();
    void execute_instruction(CallFrame &frame);
    const std::vector<Instruction> *lookup_method(const CompiledClass &cls, const std::string &name,
                                                  const std::string &caller_class);
    void system_call(const std::string &method, int arg_count);
    void run_field_init(ObjectPtr obj, const CompiledClass &cls);

    void check_breakpoints(const Instruction &instr);
    void wait_for_debugger_command();
    std::string get_stack_trace();
    std::string get_locals_json(const CallFrame &frame);

    static constexpr size_t MAX_CALL_DEPTH = 1000;
    void check_call_depth() const
    {
        if (frames_.size() >= MAX_CALL_DEPTH) {
            throw RuntimeError("Stack overflow: max call depth (" + std::to_string(MAX_CALL_DEPTH) +
                               ") exceeded");
        }
    }

    std::unordered_map<std::string, void *> ffi_library_cache_;
    void ffi_close_all();
    std::unordered_set<std::string> const_vars_;
};

std::string value_to_string(const Value &value);

} 

#endif
