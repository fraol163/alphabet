#include "compiler.h"
#include <algorithm>
#include <sstream>

namespace alphabet {

inline std::string sv_to_str(std::string_view sv) {
    return std::string(sv);
}

void Compiler::validate_types(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        if (auto* var_stmt = dynamic_cast<const VarStmt*>(stmt.get())) {
            if (var_stmt->initializer) {
                uint16_t declared_type = static_cast<uint16_t>(std::stoi(sv_to_str(var_stmt->type_id.lexeme)));
                uint16_t inferred_type = infer_expression_type(var_stmt->initializer);
                if (!types_compatible(inferred_type, declared_type)) {
                    std::ostringstream oss;
                    oss << "Type mismatch: cannot assign type " << inferred_type 
                        << " to variable of type " << declared_type;
                    throw CompileError(oss.str());
                }
            }
        } else if (auto* class_stmt = dynamic_cast<const ClassStmt*>(stmt.get())) {
            for (const auto& method : class_stmt->methods) {
                for (const auto& body_stmt : method.body) {
                    if (auto* ret_stmt = dynamic_cast<const ReturnStmt*>(body_stmt.get())) {
                        if (ret_stmt->value) {
                            uint16_t return_type = static_cast<uint16_t>(std::stoi(sv_to_str(method.return_type.lexeme)));
                            uint16_t expr_type = infer_expression_type(ret_stmt->value);
                            if (!types_compatible(expr_type, return_type)) {
                                std::ostringstream oss;
                                oss << "Method '" << sv_to_str(method.name.lexeme) 
                                    << "': return type mismatch";
                                throw CompileError(oss.str());
                            }
                        }
                    }
                }
            }
        }
    }
}

bool Compiler::types_compatible(uint16_t source, uint16_t target) {
    if (source == target) return true;

    if (target == TypeManager::INT) {
        return true;
    }

    if (target >= TypeManager::I8 && target <= TypeManager::INT) {
        if (source >= TypeManager::I8 && source <= TypeManager::F64) {
            return true;
        }
    }

    if (target >= TypeManager::F32 && target <= TypeManager::FLOAT) {
        if (source >= TypeManager::I8 && source <= TypeManager::F64) {
            return true;
        }
    }
    
    if (target >= 15) {
        return source == target || source >= 15;
    }

    return false;
}

uint16_t Compiler::infer_expression_type(const ExprPtr& expr) {
    if (!expr) return TypeManager::I32;

    if (auto* lit = dynamic_cast<const Literal*>(expr.get())) {
        return std::visit([](const auto& value) -> uint16_t {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return TypeManager::I32;
            } else if constexpr (std::is_same_v<T, double>) {
                return TypeManager::F64;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return TypeManager::STR;
            }
            return TypeManager::I32;
        }, lit->value);
    }
    
    if (auto* bin = dynamic_cast<const Binary*>(expr.get())) {
        uint16_t left_type = infer_expression_type(bin->left);
        uint16_t right_type = infer_expression_type(bin->right);

        if (left_type >= TypeManager::I8 && left_type <= TypeManager::I64 &&
            right_type >= TypeManager::I8 && right_type <= TypeManager::I64) {
            return std::max(left_type, right_type);
        }
        if (left_type == TypeManager::F32 || left_type == TypeManager::F64 ||
            right_type == TypeManager::F32 || right_type == TypeManager::F64) {
            return TypeManager::F64;
        }
        return TypeManager::I32;
    }
    
    if (auto* var = dynamic_cast<const Variable*>(expr.get())) {
        std::string name = sv_to_str(var->name.lexeme);
        if (name == "z") return TypeManager::I32;
        if (class_map_.find(name) != class_map_.end()) {
            return class_map_[name];
        }
        return TypeManager::I32;
    }

    if (auto* new_expr = dynamic_cast<const New*>(expr.get())) {
        std::string class_name = sv_to_str(new_expr->name.lexeme);
        if (class_map_.find(class_name) != class_map_.end()) {
            return class_map_[class_name];
        }
        return TypeManager::I32;
    }

    if (auto* call = dynamic_cast<const Call*>(expr.get())) {
        if (auto* get = dynamic_cast<const Get*>(call->callee.get())) {
            std::string method_name = sv_to_str(get->name.lexeme);
            if (method_name == "o") return TypeManager::I32;
        }
        return TypeManager::I32;
    }

    if (auto* list = dynamic_cast<const ListLiteral*>(expr.get())) {
        (void)list;
        return TypeManager::LIST;
    }

    if (auto* map = dynamic_cast<const MapLiteral*>(expr.get())) {
        (void)map;
        return TypeManager::MAP;
    }

    return TypeManager::I32;
}

void Compiler::validate_expression_type(const ExprPtr& expr, uint16_t expected_type) {
    uint16_t actual_type = infer_expression_type(expr);
    if (!types_compatible(actual_type, expected_type)) {
        std::ostringstream oss;
        oss << "Type error: expected type " << expected_type << " but got " << actual_type;
        throw CompileError(oss.str());
    }
}

Compiler::Compiler() = default;

Program Compiler::compile(const std::vector<StmtPtr>& statements) {
    Program program;

    for (const auto& stmt : statements) {
        if (auto* class_stmt = dynamic_cast<ClassStmt*>(stmt.get())) {
            if (!class_stmt->is_interface) {
                std::string name = sv_to_str(class_stmt->name.lexeme);
                if (class_map_.find(name) == class_map_.end()) {
                    class_map_[name] = next_class_id_++;
                }
            }
        }
    }

    validate_types(statements);

    std::unordered_map<std::string, CompiledClass> classes;
    for (const auto& stmt : statements) {
        if (auto* class_stmt = dynamic_cast<ClassStmt*>(stmt.get())) {
            if (!class_stmt->is_interface) {
                std::string name = sv_to_str(class_stmt->name.lexeme);
                classes[name] = compile_class_def(*class_stmt);
            }
        }
    }

    bytecode_.clear();
    for (const auto& stmt : statements) {
        if (!dynamic_cast<ClassStmt*>(stmt.get())) {
            visit(stmt);
        }
    }

    emit(OpCode::HALT);
    program.main = std::move(bytecode_);

    for (const auto& [name, cls] : classes) {
        if (!cls.static_init.empty()) {
            program.static_init.insert(program.static_init.end(),
                                       cls.static_init.begin(), cls.static_init.end());
        }
        program.classes[cls.id] = cls;
    }

    program.globals = globals_;

    return program;
}

void Compiler::emit(OpCode op, Operand operand) {
    bytecode_.emplace_back(op, std::move(operand));
}

void Compiler::patch_jump(size_t index, size_t target) {
    if (index < bytecode_.size()) {
        bytecode_[index].operand = static_cast<int64_t>(target);
    }
}

size_t Compiler::get_global_index(const std::string& name) {
    auto it = std::find(globals_.begin(), globals_.end(), name);
    if (it != globals_.end()) {
        return it - globals_.begin();
    }
    globals_.push_back(name);
    return globals_.size() - 1;
}

void Compiler::visit(const StmtPtr& stmt) {
    if (auto* s = dynamic_cast<const ReturnStmt*>(stmt.get())) {
        visit_return(*s);
    } else if (auto* s = dynamic_cast<const VarStmt*>(stmt.get())) {
        visit_var(*s);
    } else if (auto* s = dynamic_cast<const ExpressionStmt*>(stmt.get())) {
        visit_expression(*s);
    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt.get())) {
        visit_if(*s);
    } else if (auto* s = dynamic_cast<const LoopStmt*>(stmt.get())) {
        visit_loop(*s);
    } else if (auto* s = dynamic_cast<const TryStmt*>(stmt.get())) {
        visit_try(*s);
    } else if (auto* s = dynamic_cast<const Block*>(stmt.get())) {
        visit_block(*s);
    } else if (auto* s = dynamic_cast<const ClassStmt*>(stmt.get())) {
        visit_class(*s);
    }
}

void Compiler::visit_expr(const ExprPtr& expr) {
    if (auto* e = dynamic_cast<const Binary*>(expr.get())) {
        visit_binary(*e);
    } else if (auto* e = dynamic_cast<const Unary*>(expr.get())) {
        visit_unary(*e);
    } else if (auto* e = dynamic_cast<const Literal*>(expr.get())) {
        visit_literal(*e);
    } else if (auto* e = dynamic_cast<const Grouping*>(expr.get())) {
        visit_grouping(*e);
    } else if (auto* e = dynamic_cast<const Variable*>(expr.get())) {
        visit_variable(*e);
    } else if (auto* e = dynamic_cast<const Assign*>(expr.get())) {
        visit_assign(*e);
    } else if (auto* e = dynamic_cast<const Logical*>(expr.get())) {
        visit_logical(*e);
    } else if (auto* e = dynamic_cast<const Call*>(expr.get())) {
        visit_call(*e);
    } else if (auto* get = dynamic_cast<const Get*>(expr.get())) {
        visit_get(*get);
    } else if (auto* set = dynamic_cast<const Set*>(expr.get())) {
        visit_set(*set);
    } else if (auto* new_expr = dynamic_cast<const New*>(expr.get())) {
        visit_new(*new_expr);
    } else if (auto* list = dynamic_cast<const ListLiteral*>(expr.get())) {
        visit_list(*list);
    } else if (auto* map = dynamic_cast<const MapLiteral*>(expr.get())) {
        visit_map(*map);
    } else if (auto* index = dynamic_cast<const IndexExpr*>(expr.get())) {
        visit_index(*index);
    }
}

void Compiler::visit_return(const ReturnStmt& stmt) {
    if (stmt.value) {
        visit_expr(stmt.value);
    } else {
        emit(OpCode::PUSH_CONST, nullptr);
    }
    emit(OpCode::RET);
}

void Compiler::visit_var(const VarStmt& stmt) {
    if (stmt.initializer) {
        visit_expr(stmt.initializer);
    } else {
        emit(OpCode::PUSH_CONST, nullptr);
    }

    size_t idx = get_global_index(sv_to_str(stmt.name.lexeme));
    emit(OpCode::STORE_VAR, static_cast<int64_t>(idx));
}

void Compiler::visit_expression(const ExpressionStmt& stmt) {
    visit_expr(stmt.expression);
    emit(OpCode::POP);
}

void Compiler::visit_if(const IfStmt& stmt) {
    visit_expr(stmt.condition);
    
    size_t false_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
    
    visit(stmt.then_branch);
    
    if (stmt.else_branch) {
        size_t exit_jump = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));
        
        patch_jump(false_jump, bytecode_.size());
        visit(stmt.else_branch);
        
        patch_jump(exit_jump, bytecode_.size());
    } else {
        patch_jump(false_jump, bytecode_.size());
    }
}

void Compiler::visit_loop(const LoopStmt& stmt) {
    size_t start_pos = bytecode_.size();
    
    visit_expr(stmt.condition);
    
    size_t exit_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
    
    visit(stmt.body);
    
    emit(OpCode::JUMP, static_cast<int64_t>(start_pos));
    patch_jump(exit_jump, bytecode_.size());
}

void Compiler::visit_try(const TryStmt& stmt) {
    size_t setup_try_idx = bytecode_.size();
    emit(OpCode::SETUP_TRY, static_cast<int64_t>(0));
    
    visit_block(stmt.try_block);
    emit(OpCode::POP_TRY);

    size_t exit_jump_idx = bytecode_.size();
    emit(OpCode::JUMP, static_cast<int64_t>(0));

    patch_jump(setup_try_idx, bytecode_.size());

    size_t exc_idx = get_global_index(sv_to_str(stmt.exception_var.lexeme));
    emit(OpCode::STORE_VAR, static_cast<int64_t>(exc_idx));
    emit(OpCode::POP);
    
    visit_block(stmt.handle_block);
    
    patch_jump(exit_jump_idx, bytecode_.size());
}

void Compiler::visit_block(const Block& stmt) {
    for (const auto& s : stmt.statements) {
        visit(s);
    }
}

void Compiler::visit_class(const ClassStmt& /*stmt*/) {
}

void Compiler::visit_binary(const Binary& expr) {
    visit_expr(expr.left);
    visit_expr(expr.right);
    
    switch (expr.op.type) {
        case TokenType::PLUS: emit(OpCode::ADD); break;
        case TokenType::MINUS: emit(OpCode::SUB); break;
        case TokenType::STAR: emit(OpCode::MUL); break;
        case TokenType::SLASH: emit(OpCode::DIV); break;
        case TokenType::PERCENT: emit(OpCode::PERCENT); break;
        case TokenType::DOUBLE_EQUALS: emit(OpCode::EQ); break;
        case TokenType::GREATER: emit(OpCode::GT); break;
        case TokenType::LESS: emit(OpCode::LT); break;
        default: break;
    }
}

void Compiler::visit_unary(const Unary& expr) {
    visit_expr(expr.right);

    switch (expr.op.type) {
        case TokenType::NOT: emit(OpCode::NOT); break;
        case TokenType::MINUS:
            emit(OpCode::PUSH_CONST, 0.0);
            emit(OpCode::SUB);
            break;
        default: break;
    }
}

void Compiler::visit_logical(const Logical& expr) {
    visit_expr(expr.left);

    if (expr.op.type == TokenType::AND) {
        size_t false_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
        visit_expr(expr.right);
        patch_jump(false_jump, bytecode_.size());
    } else if (expr.op.type == TokenType::OR) {
        size_t true_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
        emit(OpCode::NOT);
        size_t skip = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));
        patch_jump(true_jump, bytecode_.size());
        visit_expr(expr.right);
        patch_jump(skip, bytecode_.size());
    }
}

void Compiler::visit_literal(const Literal& expr) {
    std::visit([this](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            emit(OpCode::PUSH_CONST, nullptr);
        } else if constexpr (std::is_same_v<T, double>) {
            emit(OpCode::PUSH_CONST, value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            emit(OpCode::PUSH_CONST, value);
        }
    }, expr.value);
}

void Compiler::visit_grouping(const Grouping& expr) {
    visit_expr(expr.expression);
}

void Compiler::visit_variable(const Variable& expr) {
    std::string name = sv_to_str(expr.name.lexeme);

    if (name == "z") {
        emit(OpCode::PUSH_CONST, std::string("SYSTEM_Z"));
        return;
    }

    auto global_it = std::find(globals_.begin(), globals_.end(), name);
    if (global_it != globals_.end()) {
        size_t idx = global_it - globals_.begin();
        emit(OpCode::LOAD_VAR, static_cast<int64_t>(idx));
    } else {
        auto class_it = class_map_.find(name);
        if (class_it != class_map_.end()) {
            emit(OpCode::PUSH_CONST, static_cast<int64_t>(class_it->second));
        } else {
            emit(OpCode::LOAD_VAR, name);
        }
    }
}

void Compiler::visit_assign(const Assign& expr) {
    visit_expr(expr.value);

    std::string name = sv_to_str(expr.name.lexeme);
    auto global_it = std::find(globals_.begin(), globals_.end(), name);
    if (global_it != globals_.end()) {
        size_t idx = global_it - globals_.begin();
        emit(OpCode::STORE_VAR, static_cast<int64_t>(idx));
    } else {
        emit(OpCode::STORE_VAR, name);
    }
}

void Compiler::visit_set(const Set& expr) {
    if (auto* var = dynamic_cast<const Variable*>(expr.obj.get())) {
        std::string var_name = sv_to_str(var->name.lexeme);
        auto class_it = class_map_.find(var_name);
        if (class_it != class_map_.end()) {
            visit_expr(expr.obj);
            visit_expr(expr.value);
            emit(OpCode::SET_STATIC, sv_to_str(expr.name.lexeme));
            return;
        }
    }

    visit_expr(expr.obj);
    visit_expr(expr.value);
    emit(OpCode::STORE_FIELD, sv_to_str(expr.name.lexeme));
}

void Compiler::visit_new(const New& expr) {
    for (const auto& arg : expr.arguments) {
        visit_expr(arg);
    }
    emit(OpCode::NEW, sv_to_str(expr.name.lexeme));
}

void Compiler::visit_call(const Call& expr) {
    if (auto* get = dynamic_cast<const Get*>(expr.callee.get())) {
        visit_expr(get->obj);
        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
        }

        std::string method_name = sv_to_str(get->name.lexeme);
        if (method_name == "o") {
            emit(OpCode::PRINT);
        } else {
            emit(OpCode::CALL, std::make_pair(method_name,
                                              static_cast<int>(expr.arguments.size())));
        }
    } else if (auto* var = dynamic_cast<const Variable*>(expr.callee.get())) {
        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
        }

        std::string var_name = sv_to_str(var->name.lexeme);
        if (var_name == "z") {
            emit(OpCode::PUSH_CONST, std::string("SYSTEM_Z"));
        }

        emit(OpCode::CALL, std::make_pair(var_name,
                                          static_cast<int>(expr.arguments.size())));
    }
}

void Compiler::visit_get(const Get& expr) {
    if (auto* var = dynamic_cast<const Variable*>(expr.obj.get())) {
        std::string var_name = sv_to_str(var->name.lexeme);
        auto class_it = class_map_.find(var_name);
        if (class_it != class_map_.end()) {
            visit_expr(expr.obj);
            emit(OpCode::GET_STATIC, sv_to_str(expr.name.lexeme));
            return;
        }
    }

    visit_expr(expr.obj);
    emit(OpCode::LOAD_FIELD, sv_to_str(expr.name.lexeme));
}

void Compiler::visit_list(const ListLiteral& expr) {
    for (const auto& elem : expr.elements) {
        visit_expr(elem);
    }
    emit(OpCode::BUILD_LIST, static_cast<int64_t>(expr.elements.size()));
}

void Compiler::visit_map(const MapLiteral& expr) {
    for (size_t i = 0; i < expr.keys.size(); ++i) {
        visit_expr(expr.keys[i]);
        visit_expr(expr.values[i]);
    }
    emit(OpCode::BUILD_MAP, static_cast<int64_t>(expr.keys.size()));
}

void Compiler::visit_index(const IndexExpr& expr) {
    visit_expr(expr.obj);
    visit_expr(expr.index);
    emit(OpCode::LOAD_INDEX);
}

CompiledClass Compiler::compile_class_def(const ClassStmt& stmt) {
    CompiledClass cls;
    cls.name = sv_to_str(stmt.name.lexeme);
    cls.id = class_map_[sv_to_str(stmt.name.lexeme)];

    if (stmt.superclass) {
        cls.superclass = sv_to_str(stmt.superclass->name.lexeme);
    }

    for (const auto& method : stmt.methods) {
        CompiledMethod info;
        info.bytecode = compile_method(method);

        for (const auto& param : method.params) {
            info.param_names.push_back(sv_to_str(param.name.lexeme));
        }
        
        std::string method_name = sv_to_str(method.name.lexeme);
        if (method.is_static) {
            cls.static_methods[method_name] = std::move(info);
        } else {
            cls.methods[method_name] = std::move(info);
        }
    }

    std::vector<Instruction> old_bytecode = std::move(bytecode_);
    bytecode_.clear();

    for (const auto& field : stmt.fields) {
        if (field.is_static && field.initializer) {
            emit(OpCode::PUSH_CONST, static_cast<int64_t>(cls.id));
            visit_expr(field.initializer);
            emit(OpCode::SET_STATIC, sv_to_str(field.name.lexeme));
            emit(OpCode::POP);
        }
    }

    cls.static_init = std::move(bytecode_);
    bytecode_ = std::move(old_bytecode);

    return cls;
}

std::vector<Instruction> Compiler::compile_method(const FunctionStmt& method) {
    std::vector<Instruction> old_bytecode = std::move(bytecode_);
    bytecode_.clear();
    
    for (const auto& stmt : method.body) {
        visit(stmt);
    }

    if (bytecode_.empty() || bytecode_.back().op != OpCode::RET) {
        emit(OpCode::PUSH_CONST, nullptr);
        emit(OpCode::RET);
    }
    
    std::vector<Instruction> result = std::move(bytecode_);
    bytecode_ = std::move(old_bytecode);
    
    return result;
}

}
