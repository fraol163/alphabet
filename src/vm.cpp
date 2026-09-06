#include "vm.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define STDIN_FILENO 0
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

namespace alphabet {

std::string value_to_string(const Value& value) {
    return std::visit(
        [](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, double>) {
                std::ostringstream oss;
                if (v == std::floor(v)) {
                    oss << static_cast<int64_t>(v);
                } else {
                    oss << v;
                }
                return oss.str();
            } else if constexpr (std::is_same_v<T, std::string>) {
                return v;
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Value::List>>) {
                if (!v)
                    return "[]";
                std::ostringstream oss;
                oss << "[";
                for (size_t i = 0; i < v->size(); ++i) {
                    if (i > 0)
                        oss << ", ";
                    oss << value_to_string((*v)[i]);
                }
                oss << "]";
                return oss.str();
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Value::Map>>) {
                if (!v)
                    return "{}";
                std::ostringstream oss;
                oss << "{";
                bool first = true;
                for (const auto& [k, val] : *v) {
                    if (!first)
                        oss << ", ";
                    oss << k << ": " << value_to_string(val);
                    first = false;
                }
                oss << "}";
                return oss.str();
            } else if constexpr (std::is_same_v<T, ObjectPtr>) {
                return v ? "Object#" + std::to_string(v->class_id) : "null";
            }
            return "unknown";
        },
        value.data);
}

static std::string value_type_name(const Value& value) {
    if (value.is_null())
        return "null";
    if (value.is_bool())
        return "bool";
    if (value.is_integer())
        return "integer";
    if (value.is_number())
        return "number";
    if (value.is_string())
        return "string";
    if (value.is_list())
        return "list";
    if (value.is_map())
        return "map";
    if (value.is_object())
        return "object";
    return "unknown";
}

VM::VM() : stack_(std::make_unique<Value[]>(STACK_MAX)), stack_ptr_(stack_.get()) {}

VM::~VM() {
    ffi_close_all();
}

void VM::ffi_close_all() {
#ifdef _WIN32
    for (auto& [path, handle] : ffi_library_cache_) {
        if (handle)
            FreeLibrary(reinterpret_cast<HMODULE>(handle));
    }
#else
    for (auto& [path, handle] : ffi_library_cache_) {
        if (handle)
            dlclose(handle);
    }
#endif
    ffi_library_cache_.clear();
}

Value VM::call_lambda(const std::string& lambda_name, const std::vector<Value>& args) {
    auto it = global_functions_.find(lambda_name);
    if (it == global_functions_.end()) {
        throw RuntimeError("Lambda not found: " + lambda_name);
    }

    const auto& func = it->second;
    size_t saved_stack = stack_ptr_ - stack_.get();
    size_t saved_frames = frames_.size();

    for (const auto& arg : args) {
        push(arg);
    }

    CallFrame frame(&func.bytecode);
    frame.locals["this"] = Value(nullptr);
    for (size_t i = 0; i < func.param_names.size() && i < args.size(); ++i) {
        frame.locals[func.param_names[i]] = args[i];
    }

    for (const auto& [name, val] : globals_) {
        if (frame.locals.find(name) == frame.locals.end()) {
            frame.locals[name] = val;
        }
    }

    frames_.push_back(frame);

    while (!frames_.empty() && frames_.size() > saved_frames) {
        auto& current_frame = frames_.back();
        if (current_frame.ip >= current_frame.bytecode->size()) {
            // Bug #6: pop the dead frame so frames_ doesn't grow unboundedly
            // on repeated calls.
            frames_.pop_back();
            break;
        }
        execute_instruction(current_frame);
    }

    Value result(nullptr);
    if (stack_ptr_ > stack_.get() + saved_stack) {
        result = pop();
    }

    stack_ptr_ = stack_.get() + saved_stack;
    return result;
}

Value VM::call_lambda_public(const std::string& lambda_name, const std::vector<Value>& args,
                             const std::unordered_map<std::string, CompiledMethod>& fns) {
    auto it = fns.find(lambda_name);
    if (it == fns.end()) {
        return Value(nullptr);
    }

    const auto& func = it->second;
    size_t saved_stack = stack_ptr_ - stack_.get();
    size_t saved_frames = frames_.size();

    for (const auto& arg : args) {
        push(arg);
    }

    CallFrame frame(&func.bytecode);
    frame.locals["this"] = Value(nullptr);
    for (size_t i = 0; i < func.param_names.size() && i < args.size(); ++i) {
        frame.locals[func.param_names[i]] = args[i];
    }

    for (const auto& [name, val] : globals_) {
        if (frame.locals.find(name) == frame.locals.end()) {
            frame.locals[name] = val;
        }
    }

    frames_.push_back(frame);

    while (!frames_.empty() && frames_.size() > saved_frames) {
        auto& current_frame = frames_.back();
        if (current_frame.ip >= current_frame.bytecode->size()) {
            // Bug #6: see call_lambda above.
            frames_.pop_back();
            break;
        }
        execute_instruction(current_frame);
    }

    Value result(nullptr);
    if (stack_ptr_ > stack_.get() + saved_stack) {
        result = pop();
    }

    stack_ptr_ = stack_.get() + saved_stack;
    return result;
}

VM::VM(const Program& program) : stack_(std::make_unique<Value[]>(STACK_MAX)), stack_ptr_(stack_.get()) {
    init(program);
}

void VM::init(const Program& program) {
    classes_ = program.classes;
    globals_by_index_ = program.globals;
    global_functions_ = program.functions;
    constant_pool_ = program.constant_pool;
    stack_ptr_ = stack_.get();

    class_name_to_id_.clear();
    for (const auto& [id, cls] : classes_) {
        class_name_to_id_[cls.name] = id;
    }

    if (!program.static_init.empty()) {
        frames_.emplace_back(&program.static_init);
        run_loop();
    }

    if (!program.main.empty()) {
        frames_.emplace_back(&program.main);
    }
}

void VM::run() {
    run_loop();
}

void VM::run_from(const Program& program, size_t bytecode_offset) {
    classes_ = program.classes;
    globals_by_index_ = program.globals;
    global_functions_ = program.functions;

    class_name_to_id_.clear();
    for (const auto& [id, cls] : classes_) {
        class_name_to_id_[cls.name] = id;
    }

    frames_.clear();
    if (!program.main.empty() && bytecode_offset < program.main.size()) {
        CallFrame frame(&program.main);
        frame.ip = bytecode_offset;
        frames_.push_back(frame);
        run_loop();
    }
}

void VM::run_incremental(const Program& program, size_t bytecode_offset) {
    classes_ = program.classes;
    globals_by_index_ = program.globals;
    global_functions_ = program.functions;
    constant_pool_ = program.constant_pool;

    class_name_to_id_.clear();
    for (const auto& [id, cls] : classes_) {
        class_name_to_id_[cls.name] = id;
    }

    if (!program.main.empty() && bytecode_offset < program.main.size()) {
        CallFrame frame(&program.main);
        frame.ip = bytecode_offset;
        frames_.push_back(frame);
        run_loop();
    }
}

void VM::push(const Value& value) {
    if (stack_ptr_ - stack_.get() >= static_cast<ptrdiff_t>(STACK_MAX)) {
        throw RuntimeError("Stack overflow");
    }
    *stack_ptr_++ = value;
}

Value VM::pop() {
    if (stack_ptr_ == stack_.get()) {
        throw RuntimeError("Stack underflow");
    }
    return *--stack_ptr_;
}

Value& VM::peek(size_t distance) {
    if (stack_ptr_ - stack_.get() <= static_cast<ptrdiff_t>(distance)) {
        throw RuntimeError("Stack peek out of bounds");
    }
    return *(stack_ptr_ - 1 - distance);
}

void VM::run_loop() {
    size_t start_frame_count = frames_.size();

    while (!frames_.empty()) {
        CallFrame& frame = frames_.back();

        if (frame.ip >= frame.bytecode->size()) {
            // Frame ran out of bytecode without executing RET. This is the
            // only place a null return is synthesized — and only when this
            // is the OUTERMOST frame (top-level entry). Nested calls are
            // expected to RET explicitly; if they fall off the end, they
            // leave the caller's stack untouched (the caller will see the
            // same stack depth as before the call and skip the pop).
            if (frames_.size() == start_frame_count) {
                push(Value(nullptr));
            }
            frames_.pop_back();
            if (frames_.size() < start_frame_count) {
                break;
            }
            continue;
        }

        try {
            execute_instruction(frame);
        } catch (const RuntimeError& e) {
            throw_exception(Value(std::string(e.what())));
        }
    }
}

void VM::execute_instruction(CallFrame& frame) {
    const Instruction& instr = (*frame.bytecode)[frame.ip];

    if (debug_mode_) {
        check_breakpoints(instr);
    }

    if (trace_callback_) {
        size_t depth = stack_ptr_ - stack_.get();
        trace_callback_(instr, depth, "");
    }

    [[maybe_unused]] size_t current_offset = frame.ip;
    frame.ip++;
    if (instr.line > 0)
        last_line_ = instr.line;

    switch (instr.op) {
    case OpCode::PUSH_CONST: {
        if (auto* d = std::get_if<double>(&instr.operand)) {
            push(Value(*d));
        } else if (auto* s = std::get_if<std::string>(&instr.operand)) {
            push(Value(*s));
        } else if (auto* i = std::get_if<int64_t>(&instr.operand)) {
            push(Value(static_cast<double>(*i)));
        } else if (std::holds_alternative<std::monostate>(instr.operand) ||
                   std::holds_alternative<std::nullptr_t>(instr.operand)) {
            push(Value(nullptr));
        }
        break;
    }

    case OpCode::PUSH_CONST_POOL: {
        auto* idx = std::get_if<int64_t>(&instr.operand);
        if (idx && static_cast<size_t>(*idx) < constant_pool_.size()) {
            const Operand& val = constant_pool_[*idx];
            if (auto* d = std::get_if<double>(&val)) {
                push(Value(*d));
            } else if (auto* s = std::get_if<std::string>(&val)) {
                push(Value(*s));
            } else if (auto* i = std::get_if<int64_t>(&val)) {
                push(Value(static_cast<double>(*i)));
            } else if (std::holds_alternative<std::nullptr_t>(val)) {
                push(Value(nullptr));
            } else {
                push(Value(nullptr));
            }
        } else {
            push(Value(nullptr));
        }
        break;
    }

    case OpCode::LOAD_SUPER: {
        Value this_val;
        auto local_it = frame.locals.find("this");
        if (local_it != frame.locals.end()) {
            this_val = local_it->second;
        }
        if (this_val.is_object()) {
            ObjectPtr obj = this_val.as_object();
            auto class_it = classes_.find(obj->class_id);
            if (class_it != classes_.end()) {
                const CompiledClass& cls = class_it->second;
                if (!cls.superclass.empty()) {
                    auto sid = class_name_to_id_.find(cls.superclass);
                    if (sid != class_name_to_id_.end()) {
                        push(Value(static_cast<double>(sid->second)));
                        return;
                    }
                }
            }
        }
        push(Value(nullptr));
        break;
    }

    case OpCode::LOAD_VAR: {
        std::visit(
            [this, &frame](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    if (static_cast<size_t>(op) < globals_by_index_.size()) {
                        const std::string& name = globals_by_index_[op];
                        auto local_it = frame.locals.find(name);
                        if (local_it != frame.locals.end()) {
                            push(local_it->second);
                            return;
                        }
                        auto it = globals_.find(name);
                        if (it != globals_.end()) {
                            push(it->second);
                            return;
                        }
                    }
                    push(Value(nullptr));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto local_it = frame.locals.find(op);
                    if (local_it != frame.locals.end()) {
                        push(local_it->second);
                        return;
                    }
                    auto it = globals_.find(op);
                    if (it != globals_.end()) {
                        push(it->second);
                        return;
                    }

                    auto this_it = frame.locals.find("this");
                    if (this_it != frame.locals.end() && this_it->second.is_object()) {
                        ObjectPtr obj = this_it->second.as_object();
                        auto field_it = obj->fields.find(op);
                        if (field_it != obj->fields.end()) {
                            auto field_ptr = field_it->second;
                            push(*field_ptr);
                            return;
                        }
                    }
                    push(Value(nullptr));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::STORE_VAR: {
        Value val = peek();
        std::visit(
            [this, &val, &frame](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    if (static_cast<size_t>(op) < globals_by_index_.size()) {
                        const std::string& name = globals_by_index_[op];
                        if (const_vars_.count(name)) {
                            throw RuntimeError("Cannot reassign const variable '" + name + "'");
                        }
                        auto local_it = frame.locals.find(name);
                        if (local_it != frame.locals.end()) {
                            local_it->second = val;
                        } else {
                            globals_[name] = val;
                        }
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    if (const_vars_.count(op)) {
                        throw RuntimeError("Cannot reassign const variable '" + std::string(op) + "'");
                    }
                    auto local_it = frame.locals.find(op);
                    if (local_it != frame.locals.end()) {
                        local_it->second = val;
                    } else {
                        auto this_it = frame.locals.find("this");
                        if (this_it != frame.locals.end() && this_it->second.is_object()) {
                            ObjectPtr obj = this_it->second.as_object();
                            obj->fields[op] = std::make_shared<Value>(val);
                        } else {
                            globals_[op] = val;
                        }
                    }
                }
            },
            instr.operand);
        break;
    }

    case OpCode::POP:
        pop();
        break;

    case OpCode::DUP:
        push(peek());
        break;

    case OpCode::LOOP_START:

        break;

    case OpCode::BREAK_JUMP:
    case OpCode::CONTINUE_JUMP: {
        if (auto* target = std::get_if<int64_t>(&instr.operand)) {
            frame.ip = static_cast<size_t>(*target);
        }
        break;
    }

    case OpCode::ADD: {
        Value b = pop();
        Value a = pop();
        if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() + b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() + b.as_number()));
        } else if (a.is_string() && b.is_string()) {
            push(Value(a.as_string() + b.as_string()));
        } else if (a.is_string() && (b.is_number() || b.is_bool())) {
            push(Value(a.as_string() + value_to_string(b)));
        } else if ((a.is_number() || a.is_bool()) && b.is_string()) {
            push(Value(value_to_string(a) + b.as_string()));
        } else if (a.is_null() || b.is_null()) {
            // null + non-null arithmetic propagates null as "no value" pass-through
            // (defensible design choice: Python: TypeError, JS: numeric coercion, C: undefined).
            push(Value(nullptr));
        } else {
            throw RuntimeError("Type error: cannot add " + value_type_name(a) + " and " + value_type_name(b));
        }
        break;
    }

    case OpCode::SUB: {
        Value b = pop();
        Value a = pop();
        if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() - b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() - b.as_number()));
        } else if (a.is_null() || b.is_null()) {
            push(Value(nullptr));
        } else {
            throw RuntimeError("Type error: cannot subtract " + value_type_name(b) + " from " + value_type_name(a) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::MUL: {
        Value b = pop();
        Value a = pop();
        if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() * b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() * b.as_number()));
        } else if (a.is_null() || b.is_null()) {
            push(Value(nullptr));
        } else if (a.is_string() && (b.is_integer() || b.is_number())) {
            // String repetition: `"ab" * 3` → `"ababab"`. The integer
            // literal `3` is pushed to the stack as a double (see
            // OpCode::PUSH_CONST in this file, which widens int64_t
            // to double), so accept either an integer or any number
            // for the repeat count.
            const std::string& s = a.as_string();
            int64_t n = b.as_integer();
            if (n < 0) {
                throw RuntimeError("Type error: cannot repeat string negative times.");
            }
            std::string out;
            out.reserve(s.size() * static_cast<size_t>(n));
            for (int64_t i = 0; i < n; ++i) {
                out.append(s);
            }
            push(Value(out));
        } else if (b.is_string() && (a.is_integer() || a.is_number())) {
            const std::string& s = b.as_string();
            int64_t n = a.as_integer();
            if (n < 0) {
                throw RuntimeError("Type error: cannot repeat string negative times.");
            }
            std::string out;
            out.reserve(s.size() * static_cast<size_t>(n));
            for (int64_t i = 0; i < n; ++i) {
                out.append(s);
            }
            push(Value(out));
        } else {
            throw RuntimeError("Type error: cannot multiply " + value_type_name(a) + " and " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::DIV: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            push(Value(nullptr));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            if (b.as_number() != 0) {
                push(Value(a.as_number() / b.as_number()));
            } else {
                throw RuntimeError("Division by zero");
            }
        } else {
            throw RuntimeError("Type error: cannot divide " + value_type_name(a) + " by " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::PERCENT: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            push(Value(nullptr));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(std::fmod(a.as_number(), b.as_number())));
        } else {
            throw RuntimeError("Type error: cannot modulo " + value_type_name(a) + " by " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::EQ: {
        Value b = pop();
        Value a = pop();
        push(Value(a == b));
        break;
    }

    case OpCode::GT: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            // null comparisons yield false (not null) — matches JS,
            // makes `i (x > 5)` a usable guard for nullable values.
            push(Value(false));
        } else if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() > b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() > b.as_number()));
        } else {
            throw RuntimeError("Type error: cannot compare " + value_type_name(a) + " > " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::LT: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            push(Value(false));
        } else if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() < b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() < b.as_number()));
        } else {
            throw RuntimeError("Type error: cannot compare " + value_type_name(a) + " < " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::NE: {
        Value b = pop();
        Value a = pop();
        push(Value(a != b));
        break;
    }

    case OpCode::GE: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            push(Value(false));
        } else if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() >= b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() >= b.as_number()));
        } else {
            throw RuntimeError("Type error: cannot compare " + value_type_name(a) + " >= " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::LE: {
        Value b = pop();
        Value a = pop();
        if (a.is_null() || b.is_null()) {
            push(Value(false));
        } else if (a.is_integer() && b.is_integer()) {
            push(Value(a.as_integer() <= b.as_integer()));
        } else if ((a.is_number() || a.is_bool()) && (b.is_number() || b.is_bool())) {
            push(Value(a.as_number() <= b.as_number()));
        } else {
            throw RuntimeError("Type error: cannot compare " + value_type_name(a) + " <= " + value_type_name(b) +
                               " (both must be numbers)");
        }
        break;
    }

    case OpCode::NOT: {
        Value a = pop();
        bool is_false = a.is_null() || (a.is_number() && a.as_number() == 0) ||
                        (a.is_integer() && a.as_integer() == 0) || (a.is_bool() && !a.as_bool()) ||
                        (a.is_string() && a.as_string().empty());
        push(Value(is_false));
        break;
    }

    case OpCode::JUMP: {
        if (auto* target = std::get_if<int64_t>(&instr.operand)) {
            frame.ip = static_cast<size_t>(*target);
        }
        break;
    }

    case OpCode::JUMP_IF_FALSE: {
        Value cond = pop();
        bool is_false = cond.is_null() || (cond.is_number() && cond.as_number() == 0) ||
                        (cond.is_integer() && cond.as_integer() == 0) || (cond.is_bool() && !cond.as_bool()) ||
                        (cond.is_string() && cond.as_string().empty());
        if (is_false) {
            if (auto* target = std::get_if<int64_t>(&instr.operand)) {
                frame.ip = static_cast<size_t>(*target);
            }
        }
        break;
    }

    case OpCode::JUMP_IF_TRUE: {
        Value cond = pop();
        bool is_true = !cond.is_null() && !(cond.is_number() && cond.as_number() == 0) &&
                       !(cond.is_integer() && cond.as_integer() == 0) && !(cond.is_bool() && !cond.as_bool()) &&
                       !(cond.is_string() && cond.as_string().empty());
        if (is_true) {
            if (auto* target = std::get_if<int64_t>(&instr.operand)) {
                frame.ip = static_cast<size_t>(*target);
            }
        }
        break;
    }

    case OpCode::RET: {
        Value ret_val = pop();
        CallFrame finished_frame = frames_.back();
        frames_.pop_back();
        if (!frames_.empty()) {
            if (finished_frame.push_post_action_on_return) {
                push(finished_frame.post_action_value);
            } else {
                push(ret_val);
            }
        }
        break;
    }

    case OpCode::CALL: {
        std::visit(
            [this, &frame](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::pair<std::string, int>>) {
                    const auto& [method_name, arg_count] = op;

                    std::vector<Value> args;
                    for (int i = 0; i < arg_count; ++i) {
                        args.push_back(pop());
                    }
                    std::reverse(args.begin(), args.end());

                    Value callee = pop();

                    if (callee.is_string() && callee.as_string() == "SYSTEM_Z") {
                        if (method_name == "dyn" && args.size() >= 2) {
                            if (sandbox_mode_) {
                                throw RuntimeError("FFI: z.dyn blocked in sandbox mode");
                            }
                            if (!args[0].is_string() || !args[1].is_string()) {
                                throw RuntimeError("z.dyn requires string library path and function name");
                            }
                            std::string lib_path = args[0].as_string();
                            std::string func_name_str = args[1].as_string();

                            void* handle = nullptr;
                            auto cache_it = ffi_library_cache_.find(lib_path);
                            if (cache_it != ffi_library_cache_.end()) {
                                handle = cache_it->second;
                            } else {
#ifdef _WIN32
                                handle = reinterpret_cast<void*>(LoadLibraryA(lib_path.c_str()));
#else
                                handle = dlopen(lib_path.c_str(), RTLD_NOW);
#endif
                                if (!handle) {
                                    std::string err_msg = "FFI: Cannot load library " + lib_path;
#ifndef _WIN32
                                    err_msg += ": ";
                                    err_msg += dlerror();
#endif
                                    throw RuntimeError(err_msg);
                                }
                                ffi_library_cache_[lib_path] = handle;
                            }

#ifdef _WIN32
                            FARPROC raw_func = GetProcAddress(reinterpret_cast<HMODULE>(handle), func_name_str.c_str());
#else
                            void* raw_func = dlsym(handle, func_name_str.c_str());
#endif
                            if (!raw_func) {
                                throw RuntimeError("FFI: Function '" + func_name_str + "' not found in library " +
                                                   lib_path);
                            }

                            int ffi_arg_count = static_cast<int>(args.size()) - 2;
                            int64_t result = 0;
                            if (ffi_arg_count == 0) {
                                typedef int64_t (*Func0)();
                                result = reinterpret_cast<Func0>(raw_func)();
                            } else if (ffi_arg_count == 1) {
                                // Refuse to call with a non-numeric first argument:
                                // many C functions (e.g. getenv) take a pointer
                                // and will segfault if the pointer is NULL.
                                if (!args[2].is_number()) {
                                    throw RuntimeError(
                                        "FFI: z.dyn argument 0 must be a number "
                                        "(string/pointer args are not supported)");
                                }
                                typedef int64_t (*Func1)(int64_t);
                                int64_t a0 = static_cast<int64_t>(args[2].as_number());
                                result = reinterpret_cast<Func1>(raw_func)(a0);
                            } else if (ffi_arg_count == 2) {
                                if (!args[2].is_number() || !args[3].is_number()) {
                                    throw RuntimeError(
                                        "FFI: z.dyn arguments must be numbers "
                                        "(string/pointer args are not supported)");
                                }
                                typedef int64_t (*Func2)(int64_t, int64_t);
                                int64_t a0 = static_cast<int64_t>(args[2].as_number());
                                int64_t a1 = static_cast<int64_t>(args[3].as_number());
                                result = reinterpret_cast<Func2>(raw_func)(a0, a1);
                            } else if (ffi_arg_count == 3) {
                                if (!args[2].is_number() || !args[3].is_number() || !args[4].is_number()) {
                                    throw RuntimeError(
                                        "FFI: z.dyn arguments must be numbers "
                                        "(string/pointer args are not supported)");
                                }
                                typedef int64_t (*Func3)(int64_t, int64_t, int64_t);
                                int64_t a0 = static_cast<int64_t>(args[2].as_number());
                                int64_t a1 = static_cast<int64_t>(args[3].as_number());
                                int64_t a2 = static_cast<int64_t>(args[4].as_number());
                                result = reinterpret_cast<Func3>(raw_func)(a0, a1, a2);
                            } else if (ffi_arg_count == 4) {
                                if (!args[2].is_number() || !args[3].is_number() ||
                                    !args[4].is_number() || !args[5].is_number()) {
                                    throw RuntimeError(
                                        "FFI: z.dyn arguments must be numbers "
                                        "(string/pointer args are not supported)");
                                }
                                typedef int64_t (*Func4)(int64_t, int64_t, int64_t, int64_t);
                                int64_t a0 = static_cast<int64_t>(args[2].as_number());
                                int64_t a1 = static_cast<int64_t>(args[3].as_number());
                                int64_t a2 = static_cast<int64_t>(args[4].as_number());
                                int64_t a3 = static_cast<int64_t>(args[5].as_number());
                                result = reinterpret_cast<Func4>(raw_func)(a0, a1, a2, a3);
                            } else {
                                throw RuntimeError("FFI: z.dyn supports up to 4 arguments");
                            }

                            push(Value(static_cast<double>(result)));
                            return;
                        }

                        for (auto& arg : args) {
                            push(arg);
                        }
                        system_call(method_name, arg_count);
                        return;
                    }

                    if (callee.is_string()) {
                        auto func_it = global_functions_.find(callee.as_string());
                        if (func_it != global_functions_.end()) {
                            const CompiledMethod& method_info = func_it->second;
                            check_call_depth();
                            CallFrame new_frame(&method_info.bytecode);
                            for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                                new_frame.locals[method_info.param_names[i]] = args[i];
                            }
                            frames_.push_back(std::move(new_frame));
                            return;
                        }
                    }

                    if (callee.is_null()) {
                        auto func_it = global_functions_.find(method_name);
                        if (func_it != global_functions_.end()) {
                            const CompiledMethod& method_info = func_it->second;
                            check_call_depth();
                            CallFrame new_frame(&method_info.bytecode);
                            for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                                new_frame.locals[method_info.param_names[i]] = args[i];
                            }
                            frames_.push_back(std::move(new_frame));
                            return;
                        }

                        static const std::unordered_set<std::string> BUILTIN_NAMES = {"o",
                                                                                      "i",
                                                                                      "t",
                                                                                      "f",
                                                                                      "fw",
                                                                                      "fa",
                                                                                      "exists",
                                                                                      "exec",
                                                                                      "system",
                                                                                      "exit",
                                                                                      "args",
                                                                                      "sqrt",
                                                                                      "sin",
                                                                                      "cos",
                                                                                      "tan",
                                                                                      "abs",
                                                                                      "floor",
                                                                                      "ceil",
                                                                                      "round",
                                                                                      "pow",
                                                                                      "min",
                                                                                      "max",
                                                                                      "log",
                                                                                      "log10",
                                                                                      "rand",
                                                                                      "randint",
                                                                                      "clamp",
                                                                                      "len",
                                                                                      "tostr",
                                                                                      "tonum",
                                                                                      "type",
                                                                                      "is_null",
                                                                                      "is_empty",
                                                                                      "split",
                                                                                      "join",
                                                                                      "replace",
                                                                                      "trim",
                                                                                      "upper",
                                                                                      "lower",
                                                                                      "substr",
                                                                                      "chr",
                                                                                      "ord",
                                                                                      "starts_with",
                                                                                      "ends_with",
                                                                                      "find",
                                                                                      "count",
                                                                                      "range",
                                                                                      "append",
                                                                                      "pop_back",
                                                                                      "contains",
                                                                                      "keys",
                                                                                      "values",
                                                                                      "builder",
                                                                                      "set",
                                                                                      "add",
                                                                                      "has",
                                                                                      "set_size",
                                                                                      "append_str",
                                                                                      "build",
                                                                                      "reverse",
                                                                                      "sort",
                                                                                      "insert",
                                                                                      "remove",
                                                                                      "flatten",
                                                                                      "flatten_str",
                                                                                      "slice",
                                                                                      "swap",
                                                                                      "unique",
                                                                                      "zip",
                                                                                      "enumerate",
                                                                                      "sum",
                                                                                      "avg",
                                                                                      "sleep",
                                                                                      "http_get",
                                                                                      "http_post",
                                                                                      "timestamp",
                                                                                      "env",
                                                                                      "json_parse",
                                                                                      "json_stringify",
                                                                                      "assert",
                                                                                      "assert_eq"};
                        if (BUILTIN_NAMES.count(method_name)) {
                            for (auto& arg : args) {
                                push(arg);
                            }
                            system_call(method_name, arg_count);
                            return;
                        }
                    }

                    if (callee.is_object()) {
                        ObjectPtr obj = callee.as_object();

                        auto class_it = classes_.find(obj->class_id);
                        if (class_it == classes_.end()) {
                            throw RuntimeError("Unknown class ID: " + std::to_string(obj->class_id));
                        }

                        const CompiledClass& cls = class_it->second;

                        const CompiledClass* current_cls = &cls;
                        auto method_it = current_cls->methods.end();
                        while (current_cls) {
                            method_it = current_cls->methods.find(method_name);
                            if (method_it != current_cls->methods.end())
                                break;
                            if (!current_cls->superclass.empty()) {
                                auto sid = class_name_to_id_.find(current_cls->superclass);
                                if (sid != class_name_to_id_.end()) {
                                    auto sci = classes_.find(sid->second);
                                    if (sci != classes_.end()) {
                                        current_cls = &sci->second;
                                        continue;
                                    }
                                }
                            }
                            current_cls = nullptr;
                        }

                        if (!current_cls || method_it == current_cls->methods.end()) {
                            if (method_name == "init") {
                                push(callee);
                                return;
                            }
                            // Fall through to check static methods too.
                            // The class-body compiler may have placed
                            // `s 11 add(...)` (static) into `static_methods`
                            // rather than `methods`, so `sc.add(...)` (calling
                            // a static method on an instance) needs to look
                            // there as well.
                            auto static_it = cls.static_methods.find(method_name);
                            if (static_it != cls.static_methods.end()) {
                                const CompiledMethod& method_info = static_it->second;
                                check_call_depth();
                                CallFrame new_frame(&method_info.bytecode);
                                for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                                    new_frame.locals[method_info.param_names[i]] = args[i];
                                }
                                frames_.push_back(std::move(new_frame));
                                return;
                            }
                            throw RuntimeError("Method '" + method_name + "' not found in class '" + cls.name + "'");
                        }

                        const CompiledMethod& method_info = method_it->second;
                        check_call_depth();
                        CallFrame new_frame(&method_info.bytecode);
                        new_frame.locals["this"] = callee;

                        for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                            new_frame.locals[method_info.param_names[i]] = args[i];
                        }

                        frames_.push_back(std::move(new_frame));
                        return;
                    }

                    if (callee.is_integer() || (callee.is_number() && !callee.is_null())) {
                        double dval = callee.as_number();
                        int64_t ival = static_cast<int64_t>(dval);
                        if (static_cast<double>(ival) == dval && ival > 0) {
                            uint16_t static_class_id = static_cast<uint16_t>(ival);
                            auto class_it = classes_.find(static_class_id);
                            if (class_it != classes_.end()) {
                                const CompiledClass& cls = class_it->second;

                                if (method_name == "super") {
                                    Value this_val;
                                    auto local_it = frame.locals.find("this");
                                    if (local_it != frame.locals.end()) {
                                        this_val = local_it->second;
                                    }
                                    auto method_it = cls.methods.find(cls.name);
                                    if (method_it == cls.methods.end())
                                        method_it = cls.methods.find("init");
                                    if (method_it != cls.methods.end()) {
                                        const CompiledMethod& method_info = method_it->second;
                                        check_call_depth();
                                        CallFrame new_frame(&method_info.bytecode);
                                        new_frame.locals["this"] = this_val;
                                        for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                                            new_frame.locals[method_info.param_names[i]] = args[i];
                                        }
                                        frames_.push_back(std::move(new_frame));
                                        return;
                                    }
                                }

                                auto method_it = cls.static_methods.find(method_name);
                                if (method_it != cls.static_methods.end()) {
                                    const CompiledMethod& method_info = method_it->second;
                                    check_call_depth();
                                    CallFrame new_frame(&method_info.bytecode);
                                    for (size_t i = 0; i < args.size() && i < method_info.param_names.size(); ++i) {
                                        new_frame.locals[method_info.param_names[i]] = args[i];
                                    }
                                    frames_.push_back(std::move(new_frame));
                                    return;
                                }
                            }
                        }
                    }

                    push(Value(nullptr));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::NEW: {
        std::visit(
            [this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::pair<std::string, int>>) {
                    const auto& [class_name, arg_count] = op;
                    uint16_t class_id = 0;
                    auto name_it = class_name_to_id_.find(class_name);
                    if (name_it != class_name_to_id_.end()) {
                        class_id = name_it->second;
                    }
                    ObjectPtr obj = std::make_shared<AlphabetObject>(class_id);

                    std::vector<Value> args;
                    for (int i = 0; i < arg_count; ++i) {
                        args.push_back(pop());
                    }
                    std::reverse(args.begin(), args.end());

                    auto class_it = classes_.find(class_id);
                    if (class_it != classes_.end()) {
                        // E218: Cannot instantiate abstract class.
                        // An abstract class is one declared with the `a`
                        // modifier, or any class that inherits an
                        // unimplemented abstract method.
                        if (class_it->second.is_abstract) {
                            throw RuntimeError("Cannot instantiate abstract class '" + class_name + "'");
                        }
                        run_field_init(obj, class_it->second);

                        const CompiledClass* init_cls = &class_it->second;
                        bool found_init = false;
                        while (init_cls) {
                            auto it = init_cls->methods.find("init");
                            if (it == init_cls->methods.end())
                                it = init_cls->methods.find(class_name);
                            if (it != init_cls->methods.end()) {
                                found_init = true;
                                break;
                            }
                            if (!init_cls->superclass.empty()) {
                                auto sid = class_name_to_id_.find(init_cls->superclass);
                                if (sid != class_name_to_id_.end()) {
                                    auto sci = classes_.find(sid->second);
                                    if (sci != classes_.end()) {
                                        init_cls = &sci->second;
                                        continue;
                                    }
                                }
                            }
                            break;
                        }
                        if (found_init) {
                            auto it = init_cls->methods.find("init");
                            if (it == init_cls->methods.end())
                                it = init_cls->methods.find(class_name);
                            auto& init_method = it->second;
                            CallFrame init_frame(&init_method.bytecode);
                            init_frame.locals["this"] = Value(obj);

                            if (arg_count > 0) {
                                for (size_t i = 0; i < args.size() && i < init_method.param_names.size(); ++i) {
                                    init_frame.locals[init_method.param_names[i]] = args[i];
                                }
                            }

                            check_call_depth();
                            init_frame.post_action_value = Value(obj);
                            init_frame.push_post_action_on_return = true;
                            frames_.push_back(std::move(init_frame));
                            return;
                        }
                    }

                    push(Value(obj));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    uint16_t class_id = 0;
                    auto name_it = class_name_to_id_.find(op);
                    if (name_it != class_name_to_id_.end()) {
                        class_id = name_it->second;
                    }
                    ObjectPtr obj = std::make_shared<AlphabetObject>(class_id);

                    auto class_it = classes_.find(class_id);
                    if (class_it != classes_.end()) {
                        run_field_init(obj, class_it->second);
                    }

                    push(Value(obj));
                } else if constexpr (std::is_same_v<T, int64_t>) {
                    ObjectPtr obj = std::make_shared<AlphabetObject>(static_cast<uint16_t>(op));
                    push(Value(obj));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::PRINT: {
        Value val = pop();
        Value obj = pop();
        if (frame.ip - 1 >= executed_up_to_) {
            if (trace_callback_) {
                output_buffer_ = value_to_string(val);
            } else {
                std::cout << value_to_string(val) << std::endl;
                std::cout.flush();
            }
        }
        push(Value(nullptr));
        break;
    }

    case OpCode::HALT:
        frames_.clear();
        break;

    case OpCode::SETUP_TRY: {
        std::visit(
            [&frame, this](const auto& op) {
                if constexpr (std::is_same_v<std::decay_t<decltype(op)>, int64_t>) {
                    frame.try_stack.emplace_back(static_cast<size_t>(op), (stack_ptr_ - stack_.get()));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::POP_TRY:
        if (!frame.try_stack.empty()) {
            frame.try_stack.pop_back();
        }
        break;

    case OpCode::THROW: {
        Value val = pop();
        throw_exception(val);
        break;
    }

    case OpCode::GET_STATIC: {
        std::visit(
            [this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    Value class_val = pop();
                    if (class_val.is_number()) {
                        uint16_t class_id = static_cast<uint16_t>(class_val.as_number());
                        auto cls_it = classes_.find(class_id);
                        if (cls_it != classes_.end()) {
                            std::string key = cls_it->second.name + "." + std::string(op);
                            auto it = globals_.find(key);
                            if (it != globals_.end()) {
                                push(it->second);
                                return;
                            }
                        }
                    }
                    push(Value(nullptr));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::SET_STATIC: {
        std::visit(
            [this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    Value val = pop();
                    Value class_val = pop();
                    if (class_val.is_number()) {
                        uint16_t class_id = static_cast<uint16_t>(class_val.as_number());
                        auto cls_it = classes_.find(class_id);
                        if (cls_it != classes_.end()) {
                            std::string key = cls_it->second.name + "." + std::string(op);
                            globals_[key] = val;
                        }
                    }
                    push(val);
                }
            },
            instr.operand);
        break;
    }

    case OpCode::LOAD_FIELD: {
        std::visit(
            [this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    Value obj_val = pop();
                    if (obj_val.is_object()) {
                        ObjectPtr obj = obj_val.as_object();
                        auto it = obj->fields.find(op);
                        if (it != obj->fields.end()) {
                            auto field_ptr = it->second;
                            push(*field_ptr);
                        } else {
                            push(Value(nullptr));
                        }
                    } else {
                        push(Value(nullptr));
                    }
                }
            },
            instr.operand);
        break;
    }

    case OpCode::STORE_FIELD: {
        std::visit(
            [this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    Value val = pop();
                    Value obj_val = pop();
                    if (obj_val.is_object()) {
                        ObjectPtr obj = obj_val.as_object();
                        obj->fields[op] = std::make_shared<Value>(val);
                    }
                    push(val);
                }
            },
            instr.operand);
        break;
    }

    case OpCode::BUILD_LIST: {
        std::visit(
            [this](const auto& op) {
                if constexpr (std::is_same_v<std::decay_t<decltype(op)>, int64_t>) {
                    size_t count = static_cast<size_t>(op);
                    Value::List items;
                    for (size_t i = 0; i < count; ++i) {
                        items.push_back(pop());
                    }
                    std::reverse(items.begin(), items.end());
                    push(Value(std::move(items)));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::BUILD_MAP: {
        std::visit(
            [this](const auto& op) {
                if constexpr (std::is_same_v<std::decay_t<decltype(op)>, int64_t>) {
                    size_t count = static_cast<size_t>(op);
                    Value::Map m;
                    for (size_t i = 0; i < count; ++i) {
                        Value val = pop();
                        Value key_val = pop();
                        if (key_val.is_string()) {
                            m[key_val.as_string()] = val;
                        }
                    }
                    push(Value(std::move(m)));
                }
            },
            instr.operand);
        break;
    }

    case OpCode::LOAD_INDEX: {
        Value idx = pop();
        Value obj = pop();

        if (obj.is_list() && idx.is_number()) {
            const auto& list = obj.as_list();
            double raw = idx.as_number();
            int64_t index = static_cast<int64_t>(raw);

            if (index < 0)
                index += static_cast<int64_t>(list.size());
            if (index >= 0 && static_cast<size_t>(index) < list.size()) {
                push(list[static_cast<size_t>(index)]);
            } else {
                push(Value(nullptr));
            }
        } else if (obj.is_map() && idx.is_string()) {
            const auto& map = obj.as_map();
            auto it = map.find(idx.as_string());
            if (it != map.end()) {
                push(it->second);
            } else {
                push(Value(nullptr));
            }
        } else {
            push(Value(nullptr));
        }
        break;
    }

    case OpCode::STORE_INDEX: {
        Value val = pop();
        Value idx = pop();
        Value obj = pop();

        if (obj.is_list() && idx.is_number()) {
            auto& list = obj.as_list();
            double raw = idx.as_number();
            int64_t index = static_cast<int64_t>(raw);
            if (index < 0)
                index += static_cast<int64_t>(list.size());
            if (index >= 0 && static_cast<size_t>(index) < list.size()) {
                list[static_cast<size_t>(index)] = val;
            }
        } else if (obj.is_map() && idx.is_string()) {
            auto& map = obj.as_map();
            map[idx.as_string()] = val;
        }
        push(val);
        break;
    }

    case OpCode::MARK_CONST: {
        Value name_val = pop();
        if (name_val.is_string()) {
            const_vars_.insert(name_val.as_string());
        }
        break;
    }

    default:
        break;
    }
}

void VM::set_source(const std::string& source) {
    source_lines_.clear();
    std::istringstream stream(source);
    std::string l;
    while (std::getline(stream, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        source_lines_.push_back(l);
    }
}

void VM::check_breakpoints(const Instruction& instr) {
    bool is_bp = (breakpoints_.find(instr.line) != breakpoints_.end());
    bool is_step = (step_over_ || (step_line_ && instr.line > 0 && instr.line != step_from_line_));

    if (is_bp || is_step) {
        step_over_ = false;
        step_line_ = false;
        step_from_line_ = instr.line;

        bool is_tty = isatty(STDIN_FILENO);
        if (is_tty) {
            std::cout << "\033[1;33m[Stopped]\033[0m line " << instr.line
                      << " (" << (is_bp ? "breakpoint" : "step") << ")\n";
            if (instr.line > 0 && static_cast<size_t>(instr.line) <= source_lines_.size()) {
                std::cout << "  \033[1;36m-->\033[0m " << instr.line << ": "
                          << source_lines_[instr.line - 1] << "\n";
            }
        } else {
            std::cout << "{\"event\":\"stopped\",\"line\":" << instr.line << ",\"reason\":\""
                      << (is_bp ? "breakpoint" : "step") << "\"}" << std::endl;
        }
        wait_for_debugger_command();
    }
}

void VM::wait_for_debugger_command() {
    bool is_tty = isatty(STDIN_FILENO);
    std::string line;
    while (true) {
        if (is_tty) {
            std::cout << "(" << debugger_prompt_ << ") " << std::flush;
        }
        if (!std::getline(std::cin, line)) {
            break;
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));

        if (line == "continue" || line == "c") {
            break;
        } else if (line == "step" || line == "s" || line == "next" || line == "n") {
            step_line_ = true;
            step_from_line_ = last_line_;
            break;
        } else if (line == "si") {
            step_over_ = true;
            break;
        } else if (line == "quit" || line == "q") {
            exit(0);
        } else if (line == "locals" || line == "l") {
            if (is_tty) {
                if (!frames_.empty()) {
                    std::cout << "Locals:\n";
                    for (const auto& [name, val] : frames_.back().locals) {
                        std::cout << "  " << name << " = " << value_to_string(val) << "\n";
                    }
                } else {
                    std::cout << "  (no active frame)\n";
                }
            } else {
                if (!frames_.empty()) {
                    std::cout << get_locals_json(frames_.back()) << std::endl;
                } else {
                    std::cout << "{}" << std::endl;
                }
            }
        } else if (line == "globals" || line == "g") {
            if (is_tty) {
                std::cout << "Globals:\n";
                for (const auto& [name, val] : globals_) {
                    std::cout << "  " << name << " = " << value_to_string(val) << "\n";
                }
            } else {
                std::ostringstream oss;
                oss << "{";
                bool first_g = true;
                for (const auto& [name, val] : globals_) {
                    if (!first_g) oss << ",";
                    oss << "\"" << name << "\": \"" << value_to_string(val) << "\"";
                    first_g = false;
                }
                oss << "}";
                std::cout << oss.str() << std::endl;
            }
        } else if (line == "stack" || line == "bt") {
            if (is_tty) {
                std::cout << "Call Stack (depth " << frames_.size() << "):\n";
                for (size_t i = 0; i < frames_.size(); ++i) {
                    std::cout << "  #" << i << " IP: " << frames_[i].ip << "\n";
                }
            } else {
                std::cout << get_stack_trace() << std::endl;
            }
        } else if (line.rfind("print ", 0) == 0 || line.rfind("p ", 0) == 0) {
            size_t sp = line.find(' ');
            std::string var_name = line.substr(sp + 1);
            first = var_name.find_first_not_of(" \t");
            if (first != std::string::npos) var_name = var_name.substr(first);
            bool found = false;
            if (!frames_.empty()) {
                auto it = frames_.back().locals.find(var_name);
                if (it != frames_.back().locals.end()) {
                    std::cout << var_name << " = " << value_to_string(it->second) << "\n";
                    found = true;
                }
            }
            if (!found) {
                auto it = globals_.find(var_name);
                if (it != globals_.end()) {
                    std::cout << var_name << " = " << value_to_string(it->second) << "\n";
                    found = true;
                }
            }
            if (!found) {
                std::cout << "Variable '" << var_name << "' not found in scope\n";
            }
        } else if (line == "print" || line == "p") {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < static_cast<size_t>(stack_ptr_ - stack_.get()); ++i) {
                if (i > 0) oss << ", ";
                oss << "\"" << value_to_string(stack_[i]) << "\"";
            }
            oss << "]";
            std::cout << oss.str() << std::endl;
        } else if (line.rfind("add_break ", 0) == 0 || line.rfind("b ", 0) == 0 || line.rfind("break ", 0) == 0) {
            size_t space_pos = line.find(' ');
            try {
                int l = std::stoi(line.substr(space_pos + 1));
                add_breakpoint(l);
                if (is_tty) {
                    std::cout << "Breakpoint set at line " << l << "\n";
                } else {
                    std::cout << "{\"ok\":true,\"breakpoint\":" << l << "}" << std::endl;
                }
            } catch (...) {
                std::cout << "Invalid line number\n";
            }
        } else if (line.rfind("del_break ", 0) == 0 || line.rfind("db ", 0) == 0) {
            size_t space_pos = line.find(' ');
            try {
                int l = std::stoi(line.substr(space_pos + 1));
                remove_breakpoint(l);
                if (is_tty) {
                    std::cout << "Breakpoint removed at line " << l << "\n";
                } else {
                    std::cout << "{\"ok\":true,\"removed\":" << l << "}" << std::endl;
                }
            } catch (...) {
                std::cout << "Invalid line number\n";
            }
        } else if (line == "breakpoints" || line == "bl") {
            if (is_tty) {
                std::cout << "Breakpoints:\n";
                if (breakpoints_.empty()) {
                    std::cout << "  (none)\n";
                } else {
                    for (int bp : breakpoints_) {
                        std::cout << "  Line " << bp << "\n";
                    }
                }
            } else {
                std::ostringstream oss;
                oss << "[";
                bool first_bp = true;
                for (int bp : breakpoints_) {
                    if (!first_bp) oss << ",";
                    oss << bp;
                    first_bp = false;
                }
                oss << "]";
                std::cout << oss.str() << std::endl;
            }
        } else if (line == "help" || line == "h" || line == "?") {
            std::cout << "Debugger commands:\n"
                      << "  continue (c)          Resume execution\n"
                      << "  step (s) / next (n)   Step to next line\n"
                      << "  si                    Step one instruction\n"
                      << "  print <var> (p <var>) Print variable value\n"
                      << "  print (p)             Show operand stack\n"
                      << "  locals (l)            Show local variables\n"
                      << "  globals (g)           Show global variables\n"
                      << "  stack (bt)            Show call stack trace\n"
                      << "  add_break N (b N)     Set breakpoint at line N\n"
                      << "  del_break N (db N)    Remove breakpoint at line N\n"
                      << "  breakpoints (bl)      List all breakpoints\n"
                      << "  quit (q)              Exit debugger\n"
                      << "  help (h, ?)           Show this help\n";
        } else {
            std::cout << "Unknown command: " << line << ". Type 'help' for available commands.\n";
        }
    }
}

std::string VM::get_stack_trace() {
    std::ostringstream oss;
    oss << "{\"frames\":[";
    for (size_t i = 0; i < frames_.size(); ++i) {
        if (i > 0)
            oss << ",";
        oss << "{\"index\":" << i << ",\"ip\":" << frames_[i].ip << "}";
    }
    oss << "],\"depth\":" << frames_.size() << "}";
    return oss.str();
}

std::string VM::get_locals_json(const CallFrame& frame) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& [name, val] : frame.locals) {
        if (!first)
            oss << ",";
        oss << "\"" << name << "\": \"" << value_to_string(val) << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

void VM::run_field_init(ObjectPtr obj, const CompiledClass& cls) {
    std::vector<const CompiledClass*> chain;
    const CompiledClass* current = &cls;
    while (current) {
        chain.push_back(current);
        if (!current->superclass.empty()) {
            auto sid = class_name_to_id_.find(current->superclass);
            if (sid != class_name_to_id_.end()) {
                auto sci = classes_.find(sid->second);
                if (sci != classes_.end()) {
                    current = &sci->second;
                    continue;
                }
            }
        }
        break;
    }

    std::reverse(chain.begin(), chain.end());

    for (const auto* c : chain) {
        if (c->field_init.empty())
            continue;
        const auto& fi = c->field_init;
        for (size_t ip = 0; ip < fi.size();) {
            const auto& instr = fi[ip];
            if (instr.op == OpCode::RET)
                break;
            if (instr.op == OpCode::LOAD_VAR) {
                push(Value(obj));
            } else if (instr.op == OpCode::PUSH_CONST || instr.op == OpCode::PUSH_CONST_POOL) {
                if (instr.op == OpCode::PUSH_CONST_POOL) {
                    auto* idx = std::get_if<int64_t>(&instr.operand);
                    if (idx && static_cast<size_t>(*idx) < constant_pool_.size()) {
                        const Operand& val = constant_pool_[*idx];
                        if (auto* i = std::get_if<int64_t>(&val)) {
                            push(Value(*i));
                        } else if (auto* d = std::get_if<double>(&val)) {
                            push(Value(*d));
                        } else if (auto* s = std::get_if<std::string>(&val)) {
                            push(Value(*s));
                        } else {
                            push(Value(nullptr));
                        }
                    } else {
                        push(Value(nullptr));
                    }
                } else {
                    if (auto* i = std::get_if<int64_t>(&instr.operand)) {
                        push(Value(*i));
                    } else if (auto* d = std::get_if<double>(&instr.operand)) {
                        push(Value(*d));
                    } else if (auto* s = std::get_if<std::string>(&instr.operand)) {
                        push(Value(*s));
                    } else {
                        push(Value(nullptr));
                    }
                }
            } else if (instr.op == OpCode::STORE_FIELD) {
                Value val = pop();
                Value obj_val = pop();
                if (obj_val.is_object()) {
                    auto o = obj_val.as_object();
                    std::visit(
                        [&](const auto& f) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(f)>, std::string>) {
                                o->fields[f] = std::make_shared<Value>(val);
                            }
                        },
                        instr.operand);
                }
                push(val);
            } else if (instr.op == OpCode::POP) {
                pop();
            } else if (instr.op == OpCode::BUILD_LIST) {
                // The empty list `[]` is emitted as BUILD_LIST with
                // count=0. Mirror the main-loop implementation: pop
                // `count` elements and build a list. For count=0
                // (the common case for field initializers like `[]`)
                // this just pushes an empty list.
                int64_t count = 0;
                if (auto* i = std::get_if<int64_t>(&instr.operand)) {
                    count = *i;
                }
                Value::List items;
                for (int64_t i = 0; i < count; ++i) {
                    items.push_back(pop());
                }
                std::reverse(items.begin(), items.end());
                push(Value(std::move(items)));
            } else if (instr.op == OpCode::BUILD_MAP) {
                // Mirror the main-loop implementation: pop
                // `count * 2` values (alternating value, key) and
                // build a map. For an empty map (count=0) this just
                // pushes an empty map.
                int64_t count = 0;
                if (auto* i = std::get_if<int64_t>(&instr.operand)) {
                    count = *i;
                }
                Value::Map map;
                for (int64_t i = 0; i < count; ++i) {
                    Value v = pop();
                    Value k = pop();
                    if (k.is_string()) {
                        map[k.as_string()] = std::move(v);
                    }
                }
                push(Value(std::move(map)));
            }
            ip++;
        }
    }
}

void VM::mark_const(const std::string& name) {
    const_vars_.insert(name);
}

void VM::throw_exception(const Value& value) {
    while (!frames_.empty()) {
        CallFrame& frame = frames_.back();

        if (!frame.try_stack.empty()) {
            auto [handler_ip, stack_depth] = frame.try_stack.back();
            frame.try_stack.pop_back();

            while (static_cast<size_t>(stack_ptr_ - stack_.get()) > stack_depth) {
                pop();
            }

            push(value);
            frame.ip = handler_ip;
            return;
        }

        frames_.pop_back();
    }

    last_unhandled_error_ = value_to_string(value);
    if (exit_code_ == 0) {
        exit_code_ = 1;
    }
    had_runtime_error_ = true;
    std::cerr << "Unhandled exception: " << value_to_string(value) << std::endl;
}

const std::vector<Instruction>* VM::lookup_method(const CompiledClass& cls, const std::string& name,
                                                  const std::string&) {
    const CompiledClass* current = &cls;
    while (current) {
        auto it = current->methods.find(name);
        if (it != current->methods.end()) {
            return &it->second.bytecode;
        }

        if (!current->superclass.empty()) {
            auto super_id_it = class_name_to_id_.find(current->superclass);
            if (super_id_it != class_name_to_id_.end()) {
                auto cls_it = classes_.find(super_id_it->second);
                if (cls_it != classes_.end()) {
                    current = &cls_it->second;
                    continue;
                }
            }
        }
        break;
    }
    return nullptr;
}

} // namespace alphabet
