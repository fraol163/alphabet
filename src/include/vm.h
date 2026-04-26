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

struct Value; // Forward declaration

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

    std::variant<std::monostate, double, std::string, std::shared_ptr<List>, std::shared_ptr<Map>,
                 ObjectPtr>
        data;

    Value() : data(std::monostate{}) {}
    Value(std::nullptr_t) : data(std::monostate{}) {}
    Value(double d) : data(d) {}
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
    bool is_number() const
    {
        return std::holds_alternative<double>(data);
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

    double as_number() const
    {
        if (auto *d = std::get_if<double>(&data))
            return *d;
        return 0.0;
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
        throw std::runtime_error("Value is not a list");
    }

    const List &as_list() const
    {
        if (auto *l = std::get_if<std::shared_ptr<List>>(&data)) {
            if (*l)
                return **l;
        }
        throw std::runtime_error("Value is not a list");
    }

    Map &as_map()
    {
        if (auto *m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m)
                return **m;
        }
        throw std::runtime_error("Value is not a map");
    }

    const Map &as_map() const
    {
        if (auto *m = std::get_if<std::shared_ptr<Map>>(&data)) {
            if (*m)
                return **m;
        }
        throw std::runtime_error("Value is not a map");
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
    Value post_action_value; // Value to push after frame completes (for constructor init)
    bool push_post_action_on_return = false;

    CallFrame() : bytecode(nullptr) {}
    explicit CallFrame(const std::vector<Instruction> *bc) : bytecode(bc) {}
};

class RuntimeError : public std::runtime_error
{
  public:
    explicit RuntimeError(const std::string &msg) : std::runtime_error(msg) {}
};

class VM
{
  public:
    VM();
    ~VM();
    explicit VM(const Program &program);

    void run();
    void init(const Program &program);

    // REPL state persistence
    std::unordered_map<std::string, Value> get_globals() const
    {
        return globals_;
    }
    void set_globals(const std::unordered_map<std::string, Value> &g)
    {
        globals_ = g;
    }

    // Debugger API
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

  private:
    static constexpr size_t STACK_MAX = 65536;
    std::unique_ptr<Value[]> stack_;
    size_t stack_capacity_ = STACK_MAX;
    Value *stack_ptr_;

    std::unordered_map<std::string, Value> globals_;
    std::vector<std::string> globals_by_index_;
    std::vector<CallFrame> frames_;
    std::unordered_map<uint16_t, CompiledClass> classes_;
    std::unordered_map<std::string, uint16_t> class_name_to_id_; // Reverse lookup
    std::unordered_map<std::string, CompiledMethod> global_functions_;

    // Debug State
    bool debug_mode_ = false;
    bool sandbox_mode_ = false;
    std::unordered_set<int> breakpoints_;
    bool step_over_ = false;

    void push(const Value &value);
    Value pop();
    Value &peek(size_t distance = 0);
    void run_loop();
    void execute_instruction(CallFrame &frame);
    const std::vector<Instruction> *lookup_method(const CompiledClass &cls, const std::string &name,
                                                  const std::string &caller_class);
    void throw_exception(const Value &value);
    void system_call(const std::string &method, int arg_count);
    void run_field_init(ObjectPtr obj, const CompiledClass &cls);

    // Debugger Helpers
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

    // FFI library handle cache (avoids dlopen/dlclose on every z.dyn call)
    std::unordered_map<std::string, void*> ffi_library_cache_;
    void ffi_close_all();
};

std::string value_to_string(const Value &value);

} // namespace alphabet

#endif
