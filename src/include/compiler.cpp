#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include <algorithm>
#include <sstream>
#include <fstream>

namespace alphabet {

inline std::string sv_to_str(std::string_view sv) {
    return std::string(sv);
}

void Compiler::validate_types(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        if (auto* var_stmt = dynamic_cast<const VarStmt*>(stmt.get())) {
            uint16_t declared_type = static_cast<uint16_t>(std::stoi(sv_to_str(var_stmt->type_id.lexeme)));
            std::string var_name = sv_to_str(var_stmt->name.lexeme);
            var_types_[var_name] = declared_type;
            if (var_stmt->initializer && declared_type != 0) {
                uint16_t inferred_type = infer_expression_type(var_stmt->initializer);
                if (!types_compatible(inferred_type, declared_type)) {
                    std::ostringstream oss;
                    oss << "Type mismatch: cannot assign type " << inferred_type 
                        << " to variable of type " << declared_type;
                    throw CompileError(oss.str());
                }
            }
        } else if (auto* class_stmt = dynamic_cast<const ClassStmt*>(stmt.get())) {
            // Track field types for type inference
            for (const auto& field : class_stmt->fields) {
                std::string field_name = sv_to_str(field.name.lexeme);
                uint16_t field_type = static_cast<uint16_t>(std::stoi(sv_to_str(field.type_id.lexeme)));
                var_types_[field_name] = field_type;
            }
            // Track parameter types for method return validation
            for (const auto& method : class_stmt->methods) {
                for (const auto& param : method.params) {
                    std::string param_name = sv_to_str(param.name.lexeme);
                    uint16_t param_type = static_cast<uint16_t>(std::stoi(sv_to_str(param.type_id.lexeme)));
                    var_types_[param_name] = param_type;
                }
            }
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
        } else if (auto* func_stmt = dynamic_cast<const FunctionStmt*>(stmt.get())) {
            // Track parameter types for return validation
            for (const auto& param : func_stmt->params) {
                std::string param_name = sv_to_str(param.name.lexeme);
                uint16_t param_type = static_cast<uint16_t>(std::stoi(sv_to_str(param.type_id.lexeme)));
                var_types_[param_name] = param_type;
            }
        }
    }
}

bool Compiler::types_compatible(uint16_t source, uint16_t target) {
    if (source == target) return true;
    
    // INT(5) as source means "unknown/any type" from function calls
    if (source == TypeManager::INT) return true;
    
    // All numeric types (I8-FLOAT, ids 1-8) are compatible with each other
    bool source_is_numeric = (source >= TypeManager::I8 && source <= TypeManager::FLOAT);
    bool target_is_numeric = (target >= TypeManager::I8 && target <= TypeManager::FLOAT);
    
    if (source_is_numeric && target_is_numeric) {
        return true;
    }
    
    // Custom types (id >= 15) are only compatible with same type or other custom types
    if (target >= 15) {
        return source >= 15;
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
            } else {
                return TypeManager::I32;
            }
        }, lit->value);
    }
    
    if (auto* bin = dynamic_cast<const Binary*>(expr.get())) {
        uint16_t left_type = infer_expression_type(bin->left);
        uint16_t right_type = infer_expression_type(bin->right);

        // String concatenation: if either side is STR, result is STR
        if (left_type == TypeManager::STR || right_type == TypeManager::STR) {
            return TypeManager::STR;
        }
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
        // Look up declared variable type
        auto type_it = var_types_.find(name);
        if (type_it != var_types_.end()) {
            return type_it->second;
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
        // Return type of function/method calls unknown at inference time.
        // Use the class_map_ entry if it's a known function, otherwise
        // return INT which signals "any" to types_compatible.
        return TypeManager::INT;
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

    // Pre-pass: process imports to register their classes before type validation
    for (const auto& stmt : statements) {
        if (auto* imps = dynamic_cast<const ImportStmt*>(stmt.get())) {
            load_module(imps->module_path);
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
    
    // Merge classes loaded from imported modules
    for (const auto& [name, cls] : pending_classes_) {
        if (program.classes.find(cls.id) == program.classes.end()) {
            program.classes[cls.id] = cls;
        }
    }
    pending_classes_.clear();

    program.globals = globals_;
    program.functions = std::move(pending_functions_);
    pending_functions_.clear();

    return program;
}

void Compiler::emit(OpCode op, Operand operand, int line) {
    bytecode_.emplace_back(op, std::move(operand), line);
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
    if (auto* rs = dynamic_cast<const ReturnStmt*>(stmt.get())) {
        visit_return(*rs);
    } else if (auto* vs = dynamic_cast<const VarStmt*>(stmt.get())) {
        visit_var(*vs);
    } else if (auto* es = dynamic_cast<const ExpressionStmt*>(stmt.get())) {
        visit_expression(*es);
    } else if (auto* is = dynamic_cast<const IfStmt*>(stmt.get())) {
        visit_if(*is);
    } else if (auto* ls = dynamic_cast<const LoopStmt*>(stmt.get())) {
        visit_loop(*ls);
    } else if (auto* fs = dynamic_cast<const ForStmt*>(stmt.get())) {
        visit_for(*fs);
    } else if (auto* ts = dynamic_cast<const TryStmt*>(stmt.get())) {
        visit_try(*ts);
    } else if (auto* bs = dynamic_cast<const Block*>(stmt.get())) {
        visit_block(*bs);
    } else if (auto* cs = dynamic_cast<const ClassStmt*>(stmt.get())) {
        visit_class(*cs);
    } else if (auto* imps = dynamic_cast<const ImportStmt*>(stmt.get())) {
        imported_modules_.push_back(imps->module_path);
        if (imps->alias) {
            module_aliases_[*imps->alias] = imps->module_path;
        }
        // Load the module file and compile its symbols
        load_module(imps->module_path);
    } else if (auto* ms = dynamic_cast<const MatchStmt*>(stmt.get())) {
        visit_match(*ms);
    } else if (auto* bs = dynamic_cast<const BreakStmt*>(stmt.get())) {
        visit_break(*bs);
    } else if (auto* cs = dynamic_cast<const ContinueStmt*>(stmt.get())) {
        visit_continue(*cs);
    } else if (auto* fs = dynamic_cast<const FunctionStmt*>(stmt.get())) {
        // Top-level function definition - compile and store
        std::string func_name = sv_to_str(fs->name.lexeme);
        CompiledMethod info;
        info.bytecode = compile_method(*fs);
        for (const auto& param : fs->params) {
            info.param_names.push_back(sv_to_str(param.name.lexeme));
        }
        pending_functions_[func_name] = std::move(info);
    }
}

void Compiler::visit_expr(const ExprPtr& expr) {
    if (auto* be = dynamic_cast<const Binary*>(expr.get())) {
        visit_binary(*be);
    } else if (auto* ue = dynamic_cast<const Unary*>(expr.get())) {
        visit_unary(*ue);
    } else if (auto* le = dynamic_cast<const Literal*>(expr.get())) {
        visit_literal(*le);
    } else if (auto* ge = dynamic_cast<const Grouping*>(expr.get())) {
        visit_grouping(*ge);
    } else if (auto* ve = dynamic_cast<const Variable*>(expr.get())) {
        visit_variable(*ve);
    } else if (auto* ae = dynamic_cast<const Assign*>(expr.get())) {
        visit_assign(*ae);
    } else if (auto* loe = dynamic_cast<const Logical*>(expr.get())) {
        visit_logical(*loe);
    } else if (auto* ce = dynamic_cast<const Call*>(expr.get())) {
        visit_call(*ce);
    } else if (auto* gete = dynamic_cast<const Get*>(expr.get())) {
        visit_get(*gete);
    } else if (auto* sete = dynamic_cast<const Set*>(expr.get())) {
        visit_set(*sete);
    } else if (auto* ne = dynamic_cast<const New*>(expr.get())) {
        visit_new(*ne);
    } else if (auto* liste = dynamic_cast<const ListLiteral*>(expr.get())) {
        visit_list(*liste);
    } else if (auto* mape = dynamic_cast<const MapLiteral*>(expr.get())) {
        visit_map(*mape);
    } else if (auto* ie = dynamic_cast<const IndexExpr*>(expr.get())) {
        visit_index(*ie);
    } else if (auto* iae = dynamic_cast<const IndexAssign*>(expr.get())) {
        visit_index_assign(*iae);
    }
}

void Compiler::visit_return(const ReturnStmt& stmt) {
    if (stmt.value) {
        visit_expr(stmt.value);
    } else {
        emit(OpCode::PUSH_CONST, nullptr, stmt.keyword.line);
    }
    emit(OpCode::RET, std::monostate{}, stmt.keyword.line);
}

void Compiler::visit_var(const VarStmt& stmt) {
    if (stmt.initializer) {
        visit_expr(stmt.initializer);
    } else {
        emit(OpCode::PUSH_CONST, nullptr, stmt.name.line);
    }

    std::string name = sv_to_str(stmt.name.lexeme);
    size_t idx = get_global_index(name);
    emit(OpCode::STORE_VAR, static_cast<int64_t>(idx), stmt.name.line);
    emit(OpCode::POP);

    if (stmt.is_const) {
        const_vars_.insert(name);
    }
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
    
    // Emit LOOP_START marker (VM uses this for continue)
    emit(OpCode::LOOP_START, static_cast<int64_t>(start_pos));
    
    visit_expr(stmt.condition);
    
    size_t exit_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
    
    // Push loop context for break/continue
    loop_stack_.push_back({start_pos, {}});
    
    visit(stmt.body);
    
    // Jump back to condition check
    emit(OpCode::JUMP, static_cast<int64_t>(start_pos));
    
    // Patch exit jump
    patch_jump(exit_jump, bytecode_.size());
    
    // Patch all break jumps to point after the loop
    LoopContext ctx = loop_stack_.back();
    loop_stack_.pop_back();
    for (size_t break_idx : ctx.break_jumps) {
        patch_jump(break_idx, bytecode_.size());
    }
}

void Compiler::visit_break(const BreakStmt& stmt) {
    if (loop_stack_.empty()) {
        throw CompileError("'break' outside of loop at line " + std::to_string(stmt.keyword.line));
    }
    // Emit BREAK_JUMP with placeholder -- will be patched to after-loop
    size_t idx = bytecode_.size();
    emit(OpCode::BREAK_JUMP, static_cast<int64_t>(0), stmt.keyword.line);
    loop_stack_.back().break_jumps.push_back(idx);
}

void Compiler::visit_continue(const ContinueStmt& stmt) {
    if (loop_stack_.empty()) {
        throw CompileError("'continue' outside of loop at line " + std::to_string(stmt.keyword.line));
    }
    // Emit CONTINUE_JUMP pointing to loop start
    emit(OpCode::CONTINUE_JUMP, static_cast<int64_t>(loop_stack_.back().loop_start_ip), stmt.keyword.line);
}

void Compiler::visit_for(const ForStmt& stmt) {
    // Compile: init; loop_start: cond; JUMP_IF_FALSE exit; body; incr; JUMP loop_start; exit:
    
    // Execute initializer
    if (stmt.initializer) {
        visit(stmt.initializer);
        emit(OpCode::POP);  // Discard init result
    }
    
    size_t loop_start = bytecode_.size();
    emit(OpCode::LOOP_START, static_cast<int64_t>(loop_start));
    
    // Evaluate condition
    if (stmt.condition) {
        visit_expr(stmt.condition);
    } else {
        emit(OpCode::PUSH_CONST, 1.0);  // Infinite loop if no condition
    }
    
    size_t exit_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
    
    // Push loop context for break/continue
    loop_stack_.push_back({loop_start, {}});
    
    // Execute body
    if (stmt.body) {
        visit(stmt.body);
    }
    
    // Execute increment
    if (stmt.increment) {
        visit_expr(stmt.increment);
        emit(OpCode::POP);  // Discard increment result
    }
    
    // Jump back to condition
    emit(OpCode::JUMP, static_cast<int64_t>(loop_start));
    
    // Patch exit jump
    patch_jump(exit_jump, bytecode_.size());
    
    // Patch break jumps
    LoopContext ctx = loop_stack_.back();
    loop_stack_.pop_back();
    for (size_t break_idx : ctx.break_jumps) {
        patch_jump(break_idx, bytecode_.size());
    }
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

void Compiler::visit_match(const MatchStmt& stmt) {
    // Evaluate the match expression
    visit_expr(stmt.expression);
    
    // Compile each case
    std::vector<size_t> jump_patches;
    
    for (size_t i = 0; i < stmt.cases.size(); i++) {
        const auto& case_stmt = stmt.cases[i];
        
        // Duplicate the value for comparison
        emit(OpCode::DUP);
        
        // Compile pattern comparison
        visit_expr(case_stmt.pattern);
        emit(OpCode::EQ);
        
        // Jump to body if not equal (will patch later)
        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
        jump_patches.push_back(bytecode_.size() - 1);
        
        // Pop the duplicated value
        emit(OpCode::POP);
        
        // Compile case body
        visit(case_stmt.body);
        
        // Jump to end after this case (will patch later)
        if (i < stmt.cases.size() - 1 || stmt.default_case) {
            emit(OpCode::JUMP, static_cast<int64_t>(0));
            jump_patches.push_back(bytecode_.size() - 1);
        }
        
        // Patch the JUMP_IF_FALSE to point to next case
        patch_jump(jump_patches[jump_patches.size() - (i == stmt.cases.size() - 1 && !stmt.default_case ? 1 : 2)], 
                   bytecode_.size());
    }
    
    // Handle default case
    if (stmt.default_case) {
        // Pop the value
        emit(OpCode::POP);
        visit(stmt.default_case);
    }
    
    // Patch final jumps
    if (!jump_patches.empty()) {
        patch_jump(jump_patches.back(), bytecode_.size());
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
        case TokenType::NOT_EQUALS: emit(OpCode::NE); break;
        case TokenType::GREATER: emit(OpCode::GT); break;
        case TokenType::GREATER_EQUALS: emit(OpCode::GE); break;
        case TokenType::LESS: emit(OpCode::LT); break;
        case TokenType::LESS_EQUALS: emit(OpCode::LE); break;
        default: break;
    }
}

void Compiler::visit_unary(const Unary& expr) {
    switch (expr.op.type) {
        case TokenType::NOT: 
            visit_expr(expr.right);
            emit(OpCode::NOT); 
            break;
        case TokenType::MINUS:
            emit(OpCode::PUSH_CONST, 0.0);  // 0 - right
            visit_expr(expr.right);
            emit(OpCode::SUB);
            break;
        default: 
            visit_expr(expr.right);
            break;
    }
}

void Compiler::visit_logical(const Logical& expr) {
    visit_expr(expr.left);

    if (expr.op.type == TokenType::AND) {
        // AND: left is falsy → result 0; left truthy → result = right
        // [left] JUMP_IF_FALSE→push_0 [right] JUMP→end push_0: 0 end:
        size_t false_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
        // Left truthy: evaluate right (replaces left on stack)
        visit_expr(expr.right);
        size_t to_end = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));
        // Left falsy: push 0
        patch_jump(false_jump, bytecode_.size());
        emit(OpCode::PUSH_CONST, 0.0);
        patch_jump(to_end, bytecode_.size());
    } else if (expr.op.type == TokenType::OR) {
        // OR: left is truthy → result 1; left falsy → result = right
        // [left] JUMP_IF_TRUE→push_1 [right] JUMP→end push_1: 1 end:
        size_t true_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_TRUE, static_cast<int64_t>(0));
        // Left falsy: evaluate right (replaces left on stack)
        visit_expr(expr.right);
        size_t to_end = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));
        // Left truthy: push 1
        patch_jump(true_jump, bytecode_.size());
        emit(OpCode::PUSH_CONST, 1.0);
        patch_jump(to_end, bytecode_.size());
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
    std::string name = sv_to_str(expr.name.lexeme);

    // Check for const reassignment
    if (const_vars_.count(name)) {
        throw CompileError("Cannot reassign const variable '" + name + "'");
    }

    visit_expr(expr.value);

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
    // Push constructor args
    for (const auto& arg : expr.arguments) {
        visit_expr(arg);
    }
    // NEW creates object, inits fields, calls init with args, pushes result
    emit(OpCode::NEW, std::make_pair(sv_to_str(expr.name.lexeme),
                                      static_cast<int>(expr.arguments.size())));
}

void Compiler::visit_call(const Call& expr) {
    if (auto* get = dynamic_cast<const Get*>(expr.callee.get())) {
        // Method call on object: obj.method(args)
        // Stack order: [obj, arg1, arg2, ...] - VM pops args then obj
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
        std::string var_name = sv_to_str(var->name.lexeme);

        // Push callee first, then args
        // VM pops args (from top), then pops callee (below args)
        if (var_name == "z") {
            emit(OpCode::PUSH_CONST, std::string("SYSTEM_Z"));
        } else {
            // Top-level function call: push function name as callee
            emit(OpCode::PUSH_CONST, var_name);
        }

        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
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

void Compiler::visit_index_assign(const IndexAssign& expr) {
    visit_expr(expr.obj);
    visit_expr(expr.index);
    visit_expr(expr.value);
    emit(OpCode::STORE_INDEX);
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
    bytecode_.clear();

    // Generate instance field initialization bytecode
    for (const auto& field : stmt.fields) {
        if (!field.is_static && field.initializer) {
            // Load 'this' (the object being initialized)
            emit(OpCode::LOAD_VAR, std::string("this"));
            // Evaluate the initializer expression
            visit_expr(field.initializer);
            // Store as field
            emit(OpCode::STORE_FIELD, sv_to_str(field.name.lexeme));
            emit(OpCode::POP);
        }
    }
    if (!bytecode_.empty()) {
        emit(OpCode::PUSH_CONST, nullptr);
        emit(OpCode::RET);
    }
    cls.field_init = std::move(bytecode_);
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

void Compiler::load_module(const std::string& path) {
    // Avoid loading the same module twice
    if (loaded_modules_.count(path)) return;
    loaded_modules_.insert(path);
    
    // Resolve path relative to source file directory
    std::string resolved_path = path;
    if (!source_dir_.empty() && path.size() > 0 && path[0] != '/') {
        resolved_path = source_dir_ + "/" + path;
    }
    
    // If file not found, try ALPHABET_PATH environment variable
    {
        std::ifstream test(resolved_path);
        if (!test.good()) {
            const char* env_path = std::getenv("ALPHABET_PATH");
            if (env_path) {
                std::string env_str(env_path);
                size_t start = 0;
                while (start < env_str.size()) {
                    size_t end = env_str.find(':', start);
                    if (end == std::string::npos) end = env_str.size();
                    std::string dir = env_str.substr(start, end - start);
                    std::string candidate = dir + "/" + path;
                    std::ifstream t2(candidate);
                    if (t2.good()) { resolved_path = candidate; break; }
                    start = end + 1;
                }
            }
        }
    }
    
    // Read the module file
    std::ifstream file(resolved_path);
    if (!file.is_open()) {
        throw CompileError("Cannot import module: " + path);
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string source = oss.str();
    
    // Lex and parse the module
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, source);
    auto statements = parser.parse();
    
    if (parser.had_errors()) {
        throw CompileError("Syntax errors in imported module: " + path);
    }
    
    // Process the module's statements
    // Register classes first
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
    
    // Compile classes and store them
    for (const auto& stmt : statements) {
        if (auto* class_stmt = dynamic_cast<ClassStmt*>(stmt.get())) {
            if (!class_stmt->is_interface) {
                std::string name = sv_to_str(class_stmt->name.lexeme);
                CompiledClass cls = compile_class_def(*class_stmt);
                pending_classes_[name] = cls;
                // Emit static init into current bytecode
                if (!cls.static_init.empty()) {
                    for (const auto& instr : cls.static_init) {
                        bytecode_.push_back(instr);
                    }
                }
            }
        }
    }
    
    // Compile top-level functions and variables (skip classes, already handled)
    for (const auto& stmt : statements) {
        if (dynamic_cast<ClassStmt*>(stmt.get())) continue;
        
        if (auto* fs = dynamic_cast<const FunctionStmt*>(stmt.get())) {
            std::string func_name = sv_to_str(fs->name.lexeme);
            CompiledMethod info;
            info.bytecode = compile_method(*fs);
            for (const auto& param : fs->params) {
                info.param_names.push_back(sv_to_str(param.name.lexeme));
            }
            pending_functions_[func_name] = std::move(info);
        } else if (auto* vs = dynamic_cast<const VarStmt*>(stmt.get())) {
            visit_var(*vs);
        }
    }
}

}
