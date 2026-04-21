#include "vm.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace alphabet {

std::string value_to_string(const Value& value) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "null";
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
            if (!v) return "[]";
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < v->size(); ++i) {
                if (i > 0) oss << ", ";
                oss << value_to_string((*v)[i]);
            }
            oss << "]";
            return oss.str();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Value::Map>>) {
            if (!v) return "{}";
            std::ostringstream oss;
            oss << "{";
            bool first = true;
            for (const auto& [k, val] : *v) {
                if (!first) oss << ", ";
                oss << k << ": " << value_to_string(val);
                first = false;
            }
            oss << "}";
            return oss.str();
        } else if constexpr (std::is_same_v<T, ObjectPtr>) {
            return v ? "Object#" + std::to_string(v->class_id) : "null";
        }
        return "unknown";
    }, value.data);
}

VM::VM() {
    stack_.resize(INITIAL_STACK_SIZE);
}

VM::VM(const Program& program) {
    stack_.resize(INITIAL_STACK_SIZE);
    init(program);
}

void VM::init(const Program& program) {
    classes_ = program.classes;
    globals_by_index_ = program.globals;
    global_functions_ = program.functions;

    // Build reverse lookup for class name → ID
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

void VM::push(const Value& value) {
    if (stack_top_ >= stack_.size()) {
        stack_.resize(stack_.size() * 2);
    }
    stack_[stack_top_++] = value;
}

Value VM::pop() {
    if (stack_top_ == 0) {
        throw RuntimeError("Stack underflow");
    }
    return stack_[--stack_top_];
}

Value& VM::peek(size_t distance) {
    if (stack_top_ <= distance) {
        throw RuntimeError("Stack peek out of bounds");
    }
    return stack_[stack_top_ - 1 - distance];
}

void VM::run_loop() {
    size_t start_frame_count = frames_.size();

    while (!frames_.empty()) {
        CallFrame& frame = frames_.back();

        if (frame.ip >= frame.bytecode->size()) {
            // Frame ended without RET (e.g., break jumped past bytecode)
            // Push default return value so caller's stack stays balanced
            push(Value(nullptr));
            frames_.pop_back();
            if (frames_.size() < start_frame_count) {
                break;
            }
            continue;
        }

        execute_instruction(frame);
    }
}

void VM::execute_instruction(CallFrame& frame) {
    const Instruction& instr = (*frame.bytecode)[frame.ip];

    if (debug_mode_) {
        check_breakpoints(instr);
    }

    frame.ip++;

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
        
        // ... (rest of cases)
        
        case OpCode::LOAD_VAR: {
            std::visit([this, &frame](const auto& op) {
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
                    // Check if we're in a method and 'this' has the field
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
            }, instr.operand);
            break;
        }

        case OpCode::STORE_VAR: {
            Value val = peek();
            std::visit([this, &val, &frame](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    if (static_cast<size_t>(op) < globals_by_index_.size()) {
                        const std::string& name = globals_by_index_[op];
                        auto local_it = frame.locals.find(name);
                        if (local_it != frame.locals.end()) {
                            local_it->second = val;
                        } else {
                            globals_[name] = val;
                        }
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto local_it = frame.locals.find(op);
                    if (local_it != frame.locals.end()) {
                        local_it->second = val;
                    } else {
                        // Check if we're in a method and 'this' has the field
                        auto this_it = frame.locals.find("this");
                        if (this_it != frame.locals.end() && this_it->second.is_object()) {
                            ObjectPtr obj = this_it->second.as_object();
                            obj->fields[op] = std::make_shared<Value>(val);
                        } else {
                            globals_[op] = val;
                        }
                    }
                }
            }, instr.operand);
            break;
        }
        
        case OpCode::POP:
            pop();
            break;
        
        case OpCode::DUP:
            push(peek());
            break;
            
        case OpCode::LOOP_START:
            // Marker instruction -- no-op in VM, operand is loop start IP
            break;
            
        case OpCode::BREAK_JUMP:
        case OpCode::CONTINUE_JUMP: {
            // Jump unconditionally -- frame.ip is pre-incremented at top of
            // execute_instruction, so setting frame.ip = target means the
            // instruction at target is skipped. For BREAK_JUMP (patched to
            // bytecode_.size()), this correctly exits the loop. For CONTINUE_JUMP
            // (loop start), LOOP_START is a no-op so skipping it is fine.
            if (auto* target = std::get_if<int64_t>(&instr.operand)) {
                frame.ip = static_cast<size_t>(*target);
            }
            break;
        }
            
        case OpCode::ADD: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() + b.as_number()));
            } else if (a.is_string() && b.is_string()) {
                push(Value(a.as_string() + b.as_string()));
            } else if (a.is_string() && b.is_number()) {
                push(Value(a.as_string() + value_to_string(b)));
            } else if (a.is_number() && b.is_string()) {
                push(Value(value_to_string(a) + b.as_string()));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::SUB: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() - b.as_number()));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::MUL: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() * b.as_number()));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::DIV: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                if (b.as_number() != 0) {
                    push(Value(a.as_number() / b.as_number()));
                } else {
                    throw RuntimeError("Division by zero");
                }
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::PERCENT: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(std::fmod(a.as_number(), b.as_number())));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::EQ: {
            Value b = pop();
            Value a = pop();
            push(Value(a == b ? 1.0 : 0.0));
            break;
        }
        
        case OpCode::GT: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() > b.as_number() ? 1.0 : 0.0));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::LT: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() < b.as_number() ? 1.0 : 0.0));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::NE: {
            Value b = pop();
            Value a = pop();
            push(Value(a != b ? 1.0 : 0.0));
            break;
        }
        
        case OpCode::GE: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() >= b.as_number() ? 1.0 : 0.0));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::LE: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() <= b.as_number() ? 1.0 : 0.0));
            } else {
                push(Value(nullptr));
            }
            break;
        }
        
        case OpCode::NOT: {
            Value a = pop();
            bool is_false = a.is_null() || 
                           (a.is_number() && a.as_number() == 0) ||
                           (a.is_string() && a.as_string().empty());
            push(Value(is_false ? 1.0 : 0.0));
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
            bool is_false = cond.is_null() ||
                           (cond.is_number() && cond.as_number() == 0) ||
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
            bool is_true = !cond.is_null() &&
                          !(cond.is_number() && cond.as_number() == 0) &&
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
                // If this frame had a post-action value (e.g., constructor init), push it
                if (finished_frame.push_post_action_on_return) {
                    push(finished_frame.post_action_value);
                } else {
                    push(ret_val);
                }
            }
            break;
        }

        case OpCode::CALL: {
            std::visit([this](const auto& op) {
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
                        // Handle FFI dyn call directly (needs args in correct order)
                        if (method_name == "dyn" && args.size() >= 2) {
                            // args: [lib_path, func_name, arg0, arg1, ...]
                            if (sandbox_mode_) {
                                throw RuntimeError("FFI: z.dyn blocked in sandbox mode");
                            }
                            if (!args[0].is_string() || !args[1].is_string()) {
                                throw RuntimeError("z.dyn requires string library path and function name");
                            }
                            std::string lib_path = args[0].as_string();
                            std::string func_name_str = args[1].as_string();
                            
#ifdef _WIN32
                            HMODULE handle = LoadLibraryA(lib_path.c_str());
                            if (!handle) {
                                throw RuntimeError("FFI: Cannot load library " + lib_path);
                            }
                            FARPROC raw_func = GetProcAddress(handle, func_name_str.c_str());
#else
                            void* handle = dlopen(lib_path.c_str(), RTLD_NOW);
                            if (!handle) {
                                throw RuntimeError("FFI: Cannot load library " + lib_path + ": " + dlerror());
                            }
                            void* raw_func = dlsym(handle, func_name_str.c_str());
#endif
                            if (!raw_func) {
#ifndef _WIN32
                                std::string err = dlerror();
                                dlclose(handle);
                                throw RuntimeError("FFI: Function '" + func_name_str + "' not found: " + err);
#else
                                FreeLibrary(handle);
                                throw RuntimeError("FFI: Function '" + func_name_str + "' not found");
#endif
                            }
                            
                            int ffi_arg_count = static_cast<int>(args.size()) - 2;
                            int64_t result = 0;
                            if (ffi_arg_count == 0) {
                                typedef int64_t (*Func0)();
                                result = reinterpret_cast<Func0>(raw_func)();
                            } else if (ffi_arg_count == 1) {
                                typedef int64_t (*Func1)(int64_t);
                                int64_t a0 = args[2].is_number() ? static_cast<int64_t>(args[2].as_number()) : 0;
                                result = reinterpret_cast<Func1>(raw_func)(a0);
                            } else if (ffi_arg_count == 2) {
                                typedef int64_t (*Func2)(int64_t, int64_t);
                                int64_t a0 = args[2].is_number() ? static_cast<int64_t>(args[2].as_number()) : 0;
                                int64_t a1 = args[3].is_number() ? static_cast<int64_t>(args[3].as_number()) : 0;
                                result = reinterpret_cast<Func2>(raw_func)(a0, a1);
                            } else if (ffi_arg_count == 3) {
                                typedef int64_t (*Func3)(int64_t, int64_t, int64_t);
                                int64_t a0 = args[2].is_number() ? static_cast<int64_t>(args[2].as_number()) : 0;
                                int64_t a1 = args[3].is_number() ? static_cast<int64_t>(args[3].as_number()) : 0;
                                int64_t a2 = args[4].is_number() ? static_cast<int64_t>(args[4].as_number()) : 0;
                                result = reinterpret_cast<Func3>(raw_func)(a0, a1, a2);
                            } else if (ffi_arg_count == 4) {
                                typedef int64_t (*Func4)(int64_t, int64_t, int64_t, int64_t);
                                int64_t a0 = args[2].is_number() ? static_cast<int64_t>(args[2].as_number()) : 0;
                                int64_t a1 = args[3].is_number() ? static_cast<int64_t>(args[3].as_number()) : 0;
                                int64_t a2 = args[4].is_number() ? static_cast<int64_t>(args[4].as_number()) : 0;
                                int64_t a3 = args[5].is_number() ? static_cast<int64_t>(args[5].as_number()) : 0;
                                result = reinterpret_cast<Func4>(raw_func)(a0, a1, a2, a3);
                            } else {
                                throw RuntimeError("FFI: z.dyn supports up to 4 arguments");
                            }
                            
                            push(Value(static_cast<double>(result)));
#ifndef _WIN32
                            dlclose(handle);
#else
                            FreeLibrary(handle);
#endif
                            return;
                        }
                        // Other system calls: push args back and dispatch
                        for (auto& arg : args) {
                            push(arg);
                        }
                        system_call(method_name, arg_count);
                        return;
                    }

                    // Top-level function call (callee is the function name string)
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

                    if (callee.is_object()) {
                        ObjectPtr obj = callee.as_object();

                        auto class_it = classes_.find(obj->class_id);
                        if (class_it == classes_.end()) {
                            throw RuntimeError("Unknown class ID: " + std::to_string(obj->class_id));
                        }

                        const CompiledClass& cls = class_it->second;

                        // Search for method in class hierarchy
                        const CompiledClass* current_cls = &cls;
                        auto method_it = current_cls->methods.end();
                        while (current_cls) {
                            method_it = current_cls->methods.find(method_name);
                            if (method_it != current_cls->methods.end()) break;
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

                        if (method_it == current_cls->methods.end()) {
                            // If init is called and doesn't exist, just return the object
                            if (method_name == "init") {
                                push(callee);
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

                    push(Value(nullptr));
                }
            }, instr.operand);
            break;
        }

        case OpCode::NEW: {
            std::visit([this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::pair<std::string, int>>) {
                    const auto& [class_name, arg_count] = op;
                    uint16_t class_id = 0;
                    auto name_it = class_name_to_id_.find(class_name);
                    if (name_it != class_name_to_id_.end()) {
                        class_id = name_it->second;
                    }
                    ObjectPtr obj = std::make_shared<AlphabetObject>(class_id);

                    // Collect constructor arguments
                    std::vector<Value> args;
                    for (int i = 0; i < arg_count; ++i) {
                        args.push_back(pop());
                    }
                    std::reverse(args.begin(), args.end());

                    auto class_it = classes_.find(class_id);
                    if (class_it != classes_.end()) {
                        // Initialize field values (walks superclass chain)
                        run_field_init(obj, class_it->second);

                        // Call init method if it exists (searches hierarchy)
                        const CompiledClass* init_cls = &class_it->second;
                        bool found_init = false;
                        while (init_cls) {
                            auto it = init_cls->methods.find("init");
                            if (it != init_cls->methods.end()) {
                                found_init = true;
                                break;
                            }
                            if (!init_cls->superclass.empty()) {
                                auto sid = class_name_to_id_.find(init_cls->superclass);
                                if (sid != class_name_to_id_.end()) {
                                    auto sci = classes_.find(sid->second);
                                    if (sci != classes_.end()) { init_cls = &sci->second; continue; }
                                }
                            }
                            break;
                        }
                        if (found_init) {
                            auto& init_method = init_cls->methods.find("init")->second;
                            CallFrame init_frame(&init_method.bytecode);
                            init_frame.locals["this"] = Value(obj);
                            
                            // Pass arguments if provided
                            if (arg_count > 0) {
                                for (size_t i = 0; i < args.size() && i < init_method.param_names.size(); ++i) {
                                    init_frame.locals[init_method.param_names[i]] = args[i];
                                }
                            }
                            
                            // Set post-action to push the object after init completes
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

                    // Initialize field values (walks superclass chain)
                    auto class_it = classes_.find(class_id);
                    if (class_it != classes_.end()) {
                        run_field_init(obj, class_it->second);
                    }

                    push(Value(obj));
                } else if constexpr (std::is_same_v<T, int64_t>) {
                    ObjectPtr obj = std::make_shared<AlphabetObject>(static_cast<uint16_t>(op));
                    push(Value(obj));
                }
            }, instr.operand);
            break;
        }
        
        case OpCode::PRINT: {
            Value val = pop();
            Value obj = pop();
            std::cout << value_to_string(val) << std::endl;
            push(Value(nullptr));
            break;
        }
        
        case OpCode::HALT:
            frames_.clear();
            break;
            
        case OpCode::SETUP_TRY: {
            std::visit([&frame, this](const auto& op) {
                if constexpr (std::is_same_v<std::decay_t<decltype(op)>, int64_t>) {
                    frame.try_stack.emplace_back(static_cast<size_t>(op), stack_top_);
                }
            }, instr.operand);
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
            std::visit([this](const auto& op) {
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
            }, instr.operand);
            break;
        }
        
        case OpCode::SET_STATIC: {
            std::visit([this](const auto& op) {
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
            }, instr.operand);
            break;
        }
        
        case OpCode::LOAD_FIELD: {
            std::visit([this](const auto& op) {
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
            }, instr.operand);
            break;
        }

        case OpCode::STORE_FIELD: {
            std::visit([this](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    Value val = pop();
                    Value obj_val = pop();
                    if (obj_val.is_object()) {
                        ObjectPtr obj = obj_val.as_object();
                        obj->fields[op] = std::make_shared<Value>(val); // Store
                    }
                }
            }, instr.operand);
            break;
        }
        
        case OpCode::BUILD_LIST: {
            std::visit([this](const auto& op) {
                if constexpr (std::is_same_v<std::decay_t<decltype(op)>, int64_t>) {
                    size_t count = static_cast<size_t>(op);
                    Value::List items;
                    for (size_t i = 0; i < count; ++i) {
                        items.push_back(pop());
                    }
                    std::reverse(items.begin(), items.end());
                    push(Value(std::move(items)));
                }
            }, instr.operand);
            break;
        }
        
        case OpCode::BUILD_MAP: {
            std::visit([this](const auto& op) {
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
            }, instr.operand);
            break;
        }
        
        case OpCode::LOAD_INDEX: {
            Value idx = pop();
            Value obj = pop();
            
            if (obj.is_list() && idx.is_number()) {
                const auto& list = obj.as_list();
                double raw = idx.as_number();
                int64_t index = static_cast<int64_t>(raw);
                // Python-style negative indexing
                if (index < 0) index += static_cast<int64_t>(list.size());
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
                if (index < 0) index += static_cast<int64_t>(list.size());
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
        
        default:
            break;
    }
}

void VM::check_breakpoints(const Instruction& instr) {
    if (step_over_ || (breakpoints_.find(instr.line) != breakpoints_.end())) {
        // Output structured JSON event for debugger to parse
        std::cout << "{\"event\":\"stopped\",\"line\":" << instr.line
                  << ",\"reason\":\""
                  << (step_over_ ? "step" : "breakpoint")
                  << "\"}" << std::endl;
        step_over_ = false;
        wait_for_debugger_command();
    }
}

void VM::wait_for_debugger_command() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "continue" || line == "c") {
            break;
        } else if (line == "step" || line == "s") {
            step_over_ = true;
            break;
        } else if (line == "locals" || line == "l") {
            if (!frames_.empty()) {
                std::cout << get_locals_json(frames_.back()) << std::endl;
            } else {
                std::cout << "{}" << std::endl;
            }
        } else if (line == "stack" || line == "bt") {
            std::cout << get_stack_trace() << std::endl;
        } else if (line == "globals" || line == "g") {
            std::ostringstream oss;
            oss << "{";
            bool first = true;
            for (const auto& [name, val] : globals_) {
                if (!first) oss << ",";
                oss << "\"" << name << "\": \"" << value_to_string(val) << "\"";
                first = false;
            }
            oss << "}";
            std::cout << oss.str() << std::endl;
        } else if (line == "print" || line == "p") {
            // Print current stack
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < stack_top_; ++i) {
                if (i > 0) oss << ", ";
                oss << "\"" << value_to_string(stack_[i]) << "\"";
            }
            oss << "]";
            std::cout << oss.str() << std::endl;
        } else if (line.find("add_break ") == 0 || line.find("b ") == 0) {
            size_t space_pos = line.find(' ');
            int l = std::stoi(line.substr(space_pos + 1));
            add_breakpoint(l);
            std::cout << "{\"ok\":true,\"breakpoint\":" << l << "}" << std::endl;
        } else if (line.find("del_break ") == 0 || line.find("db ") == 0) {
            size_t space_pos = line.find(' ');
            int l = std::stoi(line.substr(space_pos + 1));
            remove_breakpoint(l);
            std::cout << "{\"ok\":true,\"removed\":" << l << "}" << std::endl;
        } else if (line == "breakpoints" || line == "bl") {
            std::ostringstream oss;
            oss << "[";
            bool first = true;
            for (int bp : breakpoints_) {
                if (!first) oss << ",";
                oss << bp;
                first = false;
            }
            oss << "]";
            std::cout << oss.str() << std::endl;
        } else if (line == "help" || line == "?") {
            std::cout << "Debugger commands:\n"
                      << "  continue (c)      Resume execution\n"
                      << "  step (s)          Step to next line\n"
                      << "  locals (l)        Show local variables\n"
                      << "  globals (g)       Show global variables\n"
                      << "  stack (bt)        Show call stack trace\n"
                      << "  print (p)         Show stack contents\n"
                      << "  add_break N (b N) Set breakpoint at line N\n"
                      << "  del_break N (db)  Remove breakpoint at line N\n"
                      << "  breakpoints (bl)  List all breakpoints\n"
                      << "  help (?)          Show this help\n";
        }
    }
}

std::string VM::get_stack_trace() {
    std::ostringstream oss;
    oss << "{\"frames\":[";
    for (size_t i = 0; i < frames_.size(); ++i) {
        if (i > 0) oss << ",";
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
        if (!first) oss << ",";
        oss << "\"" << name << "\": \"" << value_to_string(val) << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

void VM::run_field_init(ObjectPtr obj, const CompiledClass& cls) {
    // Collect superclass chain from root to leaf
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
    // Reverse so superclass runs first, subclass last
    std::reverse(chain.begin(), chain.end());

    for (const auto* c : chain) {
        if (c->field_init.empty()) continue;
        const auto& fi = c->field_init;
        for (size_t ip = 0; ip < fi.size(); ) {
            const auto& instr = fi[ip];
            if (instr.op == OpCode::RET) break;
            if (instr.op == OpCode::LOAD_VAR) {
                push(Value(obj));
            } else if (instr.op == OpCode::PUSH_CONST) {
                if (auto* d = std::get_if<double>(&instr.operand)) {
                    push(Value(*d));
                } else if (auto* s = std::get_if<std::string>(&instr.operand)) {
                    push(Value(*s));
                } else {
                    push(Value(nullptr));
                }
            } else if (instr.op == OpCode::STORE_FIELD) {
                Value val = pop();
                Value obj_val = pop();
                if (obj_val.is_object()) {
                    auto o = obj_val.as_object();
                    std::visit([&](const auto& f) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(f)>, std::string>) {
                            o->fields[f] = std::make_shared<Value>(val);
                        }
                    }, instr.operand);
                }
                push(val);
            } else if (instr.op == OpCode::POP) {
                pop();
            }
            ip++;
        }
    }
}

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
        } catch (...) {
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
            pop(); // discard path arg
            push(Value(std::string("")));
        } else {
            Value path_val = pop();
        if (path_val.is_string()) {
            std::string path = path_val.as_string();
            // Block directory traversal
            if (path.find("..") != std::string::npos) {
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
        } // close else (non-sandbox)
    } else if (method == "sqrt" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::sqrt(v.as_number()) : 0.0));
    } else if (method == "sin" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::sin(v.as_number()) : 0.0));
    } else if (method == "cos" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::cos(v.as_number()) : 0.0));
    } else if (method == "abs" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::fabs(v.as_number()) : 0.0));
    } else if (method == "floor" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::floor(v.as_number()) : 0.0));
    } else if (method == "ceil" && arg_count >= 1) {
        Value v = pop(); push(Value(v.is_number() ? std::ceil(v.as_number()) : 0.0));
    } else if (method == "pow" && arg_count >= 2) {
        Value b = pop(); Value a = pop();
        push(Value(a.is_number() && b.is_number() ? std::pow(a.as_number(), b.as_number()) : 0.0));
    } else if (method == "len" && arg_count >= 1) {
        Value v = pop();
        if (v.is_string()) push(Value(static_cast<double>(v.as_string().size())));
        else if (v.is_list()) push(Value(static_cast<double>(v.as_list().size())));
        else push(Value(0.0));
    } else if (method == "tostr" && arg_count >= 1) {
        Value v = pop(); push(Value(value_to_string(v)));
    } else if (method == "tonum" && arg_count >= 1) {
        Value v = pop();
        if (v.is_number()) { push(v); }
        else if (v.is_string()) {
            try { push(Value(std::stod(v.as_string()))); } catch (...) { push(Value(0.0)); }
        } else { push(Value(0.0)); }
    } else if (method == "type" && arg_count >= 1) {
        Value v = pop();
        if (v.is_null()) push(Value(std::string("null")));
        else if (v.is_number()) push(Value(std::string("number")));
        else if (v.is_string()) push(Value(std::string("string")));
        else push(Value(std::string("unknown")));
    } else if (method == "split" && arg_count >= 2) {
        // z.split(string, delimiter) -> list
        Value delim = pop();
        Value str = pop();
        if (str.is_string() && delim.is_string()) {
            std::vector<Value> result;
            std::string s = str.as_string();
            std::string d = delim.as_string();
            if (d.empty()) {
                for (char c : s) result.push_back(Value(std::string(1, c)));
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
        // z.join(list, separator) -> string
        Value sep = pop();
        Value list = pop();
        if (list.is_list() && sep.is_string()) {
            const auto& items = list.as_list();
            std::string separator = sep.as_string();
            std::ostringstream oss;
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) oss << separator;
                oss << value_to_string(items[i]);
            }
            push(Value(oss.str()));
        } else {
            push(Value(std::string("")));
        }
    } else if (method == "replace" && arg_count >= 3) {
        // z.replace(string, old, new) -> string
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
        // z.trim(string) -> string
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            size_t start = s.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) { push(Value(std::string(""))); return; }
            size_t end = s.find_last_not_of(" \t\n\r");
            push(Value(s.substr(start, end - start + 1)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "upper" && arg_count >= 1) {
        // z.upper(string) -> string
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            push(Value(std::move(s)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "lower" && arg_count >= 1) {
        // z.lower(string) -> string
        Value str = pop();
        if (str.is_string()) {
            std::string s = str.as_string();
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            push(Value(std::move(s)));
        } else {
            push(Value(value_to_string(str)));
        }
    } else if (method == "range") {
        // z.range(stop) -> list [0, 1, ..., stop-1]
        // z.range(start, stop) -> list [start, ..., stop-1]
        // z.range(start, stop, step) -> list [start, start+step, ...]
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
        if (step == 0) step = 1;
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
        // z.append(list, value) -> list (mutates and returns)
        Value val = pop();
        Value list_val = pop();
        if (list_val.is_list()) {
            list_val.as_list().push_back(val);
            push(list_val);
        } else {
            push(Value(std::vector<Value>{val}));
        }
    } else if (method == "pop_back" && arg_count >= 1) {
        // z.pop_back(list) -> value (removes and returns last element)
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
        // z.contains(collection, value) -> bool (1 or 0)
        Value needle = pop();
        Value haystack = pop();
        if (haystack.is_list()) {
            bool found = false;
            for (const auto& item : haystack.as_list()) {
                if (item == needle) { found = true; break; }
            }
            push(Value(found ? 1.0 : 0.0));
        } else if (haystack.is_string() && needle.is_string()) {
            push(Value(haystack.as_string().find(needle.as_string()) != std::string::npos ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    } else if (method == "keys" && arg_count >= 1) {
        // z.keys(map) -> list of keys
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
        // z.values(map) -> list of values
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
    } else if (method == "reverse" && arg_count >= 1) {
        // z.reverse(list) -> new reversed list
        Value list_val = pop();
        if (list_val.is_list()) {
            auto lst = list_val.as_list();  // copy
            std::reverse(lst.begin(), lst.end());
            push(Value(std::move(lst)));
        } else if (list_val.is_string()) {
            std::string s = list_val.as_string();
            std::reverse(s.begin(), s.end());
            push(Value(std::move(s)));
        } else {
            push(list_val);
        }
    } else if (method == "substr" && arg_count >= 2) {
        // z.substr(string, start, [length]) -> string
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
        // z.chr(code) -> single character string
        Value v = pop();
        if (v.is_number()) {
            push(Value(std::string(1, static_cast<char>(v.as_number()))));
        } else {
            push(Value(std::string("")));
        }
    } else if (method == "ord" && arg_count >= 1) {
        // z.ord(char) -> code point
        Value v = pop();
        if (v.is_string() && !v.as_string().empty()) {
            push(Value(static_cast<double>(static_cast<unsigned char>(v.as_string()[0]))));
        } else {
            push(Value(0.0));
        }
    } else if (method == "starts_with" && arg_count >= 2) {
        // z.starts_with(string, prefix) -> bool
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
        // z.ends_with(string, suffix) -> bool
        Value suffix = pop();
        Value str = pop();
        if (str.is_string() && suffix.is_string()) {
            const auto& s = str.as_string();
            const auto& suf = suffix.as_string();
            push(Value(s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0 ? 1.0 : 0.0));
        } else {
            push(Value(0.0));
        }
    }
}

void VM::throw_exception(const Value& value) {
    while (!frames_.empty()) {
        CallFrame& frame = frames_.back();
        
        if (!frame.try_stack.empty()) {
            auto [handler_ip, stack_depth] = frame.try_stack.back();
            frame.try_stack.pop_back();
            
            while (stack_top_ > stack_depth) {
                pop();
            }
            
            push(value);
            frame.ip = handler_ip;
            return;
        }
        
        frames_.pop_back();
    }
    
    std::cerr << "Unhandled exception: " << value_to_string(value) << std::endl;
}

const std::vector<Instruction>* VM::lookup_method(const CompiledClass& cls,
                                                   const std::string& name,
                                                   const std::string& /*caller_class*/) {
    const CompiledClass* current = &cls;
    while (current) {
        auto it = current->methods.find(name);
        if (it != current->methods.end()) {
            return &it->second.bytecode;
        }
        // Walk superclass chain
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

}
