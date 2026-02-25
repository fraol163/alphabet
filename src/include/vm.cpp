#include "vm.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

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

VM::VM(const Program& program) {
    init(program);
}

void VM::init(const Program& program) {
    classes_ = program.classes;
    globals_by_index_ = program.globals;

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
        throw RuntimeError("Stack overflow");
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
    const Instruction& instr = (*frame.bytecode)[frame.ip++];

    switch (instr.op) {
        case OpCode::PUSH_CONST: {
            if (auto* d = std::get_if<double>(&instr.operand)) {
                push(Value(*d));
            } else if (auto* s = std::get_if<std::string>(&instr.operand)) {
                push(Value(*s));
            } else if (std::holds_alternative<std::monostate>(instr.operand) ||
                       std::holds_alternative<std::nullptr_t>(instr.operand)) {
                push(Value(nullptr));
            }
            break;
        }
        
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
                    } else {
                        push(Value(nullptr));
                    }
                }
            }, instr.operand);
            break;
        }

        case OpCode::STORE_VAR: {
            Value val = peek();
            std::visit([this, &val](const auto& op) {
                using T = std::decay_t<decltype(op)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    if (static_cast<size_t>(op) < globals_by_index_.size()) {
                        const std::string& name = globals_by_index_[op];
                        globals_[name] = val;
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    globals_[op] = val;
                }
            }, instr.operand);
            break;
        }
        
        case OpCode::POP:
            pop();
            break;
            
        case OpCode::ADD: {
            Value b = pop();
            Value a = pop();
            if (a.is_number() && b.is_number()) {
                push(Value(a.as_number() + b.as_number()));
            } else if (a.is_string() && b.is_string()) {
                push(Value(a.as_string() + b.as_string()));
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
        
        case OpCode::RET: {
            Value ret_val = pop();
            frames_.pop_back();
            if (!frames_.empty()) {
                push(ret_val);
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
                        for (auto& arg : args) {
                            push(arg);
                        }
                        system_call(method_name, arg_count);
                        return;
                    }

                    if (callee.is_object()) {
                        ObjectPtr obj = callee.as_object();

                        auto class_it = classes_.find(obj->class_id);
                        if (class_it == classes_.end()) {
                            throw RuntimeError("Unknown class ID: " + std::to_string(obj->class_id));
                        }

                        const CompiledClass& cls = class_it->second;

                        auto method_it = cls.methods.find(method_name);
                        if (method_it == cls.methods.end()) {
                            throw RuntimeError("Method '" + method_name + "' not found in class '" + cls.name + "'");
                        }

                        const CompiledMethod& method_info = method_it->second;
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
                if constexpr (std::is_same_v<T, std::string>) {
                    uint16_t class_id = 0;
                    for (const auto& [id, cls] : classes_) {
                        if (cls.name == op) {
                            class_id = id;
                            break;
                        }
                    }
                    ObjectPtr obj = std::make_shared<AlphabetObject>(class_id);
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
                    Value class_id = pop();
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
                        }
                        push(Value(nullptr));
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
                size_t index = static_cast<size_t>(idx.as_number());
                if (index < list.size()) {
                    push(list[index]);
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
                size_t index = static_cast<size_t>(idx.as_number());
                if (index < list.size()) {
                    list[index] = val;
                    push(val);
                }
            }
            break;
        }
        
        default:
            break;
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
        throw_exception(Value("Custom Error 15"));
    } else if (method == "f" && arg_count >= 1) {
        Value path_val = pop();
        if (path_val.is_string()) {
            std::ifstream file(path_val.as_string());
            if (file.is_open()) {
                std::ostringstream oss;
                oss << file.rdbuf();
                push(Value(oss.str()));
            } else {
                push(Value(std::string("")));
            }
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
    auto it = cls.methods.find(name);
    if (it != cls.methods.end()) {
        return &it->second.bytecode;
    }
    return nullptr;
}

}
