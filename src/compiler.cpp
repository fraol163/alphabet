#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace alphabet {

inline std::string sv_to_str(std::string_view sv) {
    return std::string(sv);
}

static const std::unordered_map<std::string, std::string> TYPE_NAME_TO_ID = {
    {"null", "0"},  {"void", "0"},   {"integer", "5"}, {"int", "5"},      {"number", "5"},  {"num", "5"},
    {"float", "8"}, {"double", "8"}, {"f32", "6"},     {"f64", "7"},      {"i8", "1"},      {"i16", "2"},
    {"i32", "3"},   {"i64", "4"},    {"bool", "11"},   {"boolean", "11"}, {"string", "12"}, {"str", "12"},
    {"list", "13"}, {"array", "13"}, {"map", "14"},    {"dict", "14"},    {"dec", "9"},     {"cpx", "10"}};

uint16_t Compiler::resolve_type_id(const Token& type_id) {
    std::string s = sv_to_str(type_id.lexeme);
    auto it = TYPE_NAME_TO_ID.find(s);
    if (it != TYPE_NAME_TO_ID.end()) {
        return static_cast<uint16_t>(std::stoi(it->second));
    }
    try {
        return static_cast<uint16_t>(std::stoi(s));
    } catch (...) {
        return 0;
    }
}

void Compiler::validate_types(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        if (auto* var_stmt = dynamic_cast<const VarStmt*>(stmt.get())) {
            uint16_t declared_type = resolve_type_id(var_stmt->type_id);
            std::string var_name = sv_to_str(var_stmt->name.lexeme);
            var_types_[var_name] = declared_type;
            if (var_stmt->initializer && declared_type != 0) {
                uint16_t inferred_type = infer_expression_type(var_stmt->initializer);
                if (!types_compatible(inferred_type, declared_type)) {
                    std::ostringstream oss;
                    oss << "Type mismatch: cannot assign type " << inferred_type << " to variable of type "
                        << declared_type;
                    throw CompileError(oss.str());
                }
            }
        } else if (auto* class_stmt = dynamic_cast<const ClassStmt*>(stmt.get())) {
            for (const auto& field : class_stmt->fields) {
                std::string field_name = sv_to_str(field.name.lexeme);
                uint16_t field_type = resolve_type_id(field.type_id);
                var_types_[field_name] = field_type;
            }
            std::string class_name = sv_to_str(class_stmt->name.lexeme);
            if (class_map_.find(class_name) == class_map_.end()) {
                class_map_[class_name] = next_class_id_++;
            }
            var_types_["this"] = class_map_[class_name];

            for (const auto& method : class_stmt->methods) {
                for (const auto& param : method.params) {
                    std::string param_name = sv_to_str(param.name.lexeme);
                    uint16_t param_type = resolve_type_id(param.type_id);
                    var_types_[param_name] = param_type;
                }
                std::string method_key = sv_to_str(class_stmt->name.lexeme) + "." + sv_to_str(method.name.lexeme);
                method_return_types_[method_key] = resolve_type_id(method.return_type);
            }
            for (const auto& method : class_stmt->methods) {
                for (const auto& body_stmt : method.body) {
                    if (auto* ret_stmt = dynamic_cast<const ReturnStmt*>(body_stmt.get())) {
                        if (ret_stmt->value) {
                            uint16_t return_type = resolve_type_id(method.return_type);
                            uint16_t expr_type = infer_expression_type(ret_stmt->value);
                            if (!types_compatible(expr_type, return_type)) {
                                std::ostringstream oss;
                                oss << "Method '" << sv_to_str(method.name.lexeme) << "': return type mismatch";
                                throw CompileError(oss.str());
                            }
                        }
                    }
                }
            }
        } else if (auto* func_stmt = dynamic_cast<const FunctionStmt*>(stmt.get())) {
            for (const auto& param : func_stmt->params) {
                std::string param_name = sv_to_str(param.name.lexeme);
                uint16_t param_type = resolve_type_id(param.type_id);
                var_types_[param_name] = param_type;
            }
        }
    }
}

bool Compiler::types_compatible(uint16_t source, uint16_t target) {
    if (source == target)
        return true;

    if (source == TypeManager::TYPE_VOID)
        return true;

    if (source == TypeManager::TYPE_INT || target == TypeManager::TYPE_INT)
        return true;

    bool source_is_numeric = (source >= TypeManager::I8 && source <= TypeManager::FLOAT);
    bool target_is_numeric = (target >= TypeManager::I8 && target <= TypeManager::FLOAT);

    if (source_is_numeric && target_is_numeric) {
        return true;
    }

    if (target >= 15) {
        return source >= 15;
    }

    return false;
}

uint16_t Compiler::infer_expression_type(const ExprPtr& expr) {
    if (!expr)
        return TypeManager::I32;

    if (auto* lit = dynamic_cast<const Literal*>(expr.get())) {
        return std::visit(
            [](const auto& value) -> uint16_t {
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
            },
            lit->value);
    }

    if (auto* bin = dynamic_cast<const Binary*>(expr.get())) {
        uint16_t left_type = infer_expression_type(bin->left);
        uint16_t right_type = infer_expression_type(bin->right);

        if (left_type == TypeManager::STR || right_type == TypeManager::STR) {
            return TypeManager::STR;
        }
        if (left_type >= TypeManager::I8 && left_type <= TypeManager::I64 && right_type >= TypeManager::I8 &&
            right_type <= TypeManager::I64) {
            return std::max(left_type, right_type);
        }
        if (left_type == TypeManager::F32 || left_type == TypeManager::F64 || right_type == TypeManager::F32 ||
            right_type == TypeManager::F64) {
            return TypeManager::F64;
        }
        return TypeManager::I32;
    }

    if (auto* var = dynamic_cast<const Variable*>(expr.get())) {
        std::string name = sv_to_str(var->name.lexeme);
        if (name == "z")
            return TypeManager::I32;
        if (class_map_.find(name) != class_map_.end()) {
            return class_map_[name];
        }

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
            if (method_name == "o")
                return TypeManager::I32;
            if (auto* var = dynamic_cast<const Variable*>(get->obj.get())) {
                std::string obj_name = sv_to_str(var->name.lexeme);
                auto type_it = var_types_.find(obj_name);
                if (type_it != var_types_.end()) {
                    for (const auto& [cn, cid] : class_map_) {
                        if (cid == type_it->second) {
                            std::string key = cn + "." + method_name;
                            auto mit = method_return_types_.find(key);
                            if (mit != method_return_types_.end()) {
                                return mit->second;
                            }
                        }
                    }
                }
            }
        }
        return TypeManager::TYPE_VOID;
    }

    if (auto* get = dynamic_cast<const Get*>(expr.get())) {
        std::string field_name = sv_to_str(get->name.lexeme);
        if (auto* var = dynamic_cast<const Variable*>(get->obj.get())) {
            std::string obj_name = sv_to_str(var->name.lexeme);
            auto type_it = var_types_.find(obj_name);
            if (type_it != var_types_.end()) {
                for (const auto& [cn, cid] : class_map_) {
                    if (cid == type_it->second) {
                        auto field_it = var_types_.find(field_name);
                        if (field_it != var_types_.end()) {
                            return field_it->second;
                        }
                    }
                }
            }
        }
        return TypeManager::TYPE_VOID;
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
            program.static_init.insert(program.static_init.end(), cls.static_init.begin(), cls.static_init.end());
        }
        program.classes[cls.id] = cls;
    }

    for (const auto& [name, cls] : pending_classes_) {
        if (program.classes.find(cls.id) == program.classes.end()) {
            program.classes[cls.id] = cls;
        }
    }
    pending_classes_.clear();

    program.globals = globals_;
    program.functions = std::move(pending_functions_);
    pending_functions_.clear();

    optimize_bytecode(program.main);
    for (auto& [id, cls] : program.classes) {
        for (auto& [mname, method] : cls.methods) {
            optimize_bytecode(method.bytecode);
        }
        for (auto& [mname, method] : cls.static_methods) {
            optimize_bytecode(method.bytecode);
        }
    }
    for (auto& [name, func] : program.functions) {
        optimize_bytecode(func.bytecode);
    }

    return program;
}

void Compiler::optimize_bytecode(std::vector<Instruction>& code) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i + 2 < code.size(); ++i) {
            if (code[i].op != OpCode::PUSH_CONST)
                continue;
            if (code[i + 1].op != OpCode::PUSH_CONST)
                continue;
            OpCode op3 = code[i + 2].op;
            if (op3 != OpCode::ADD && op3 != OpCode::SUB && op3 != OpCode::MUL && op3 != OpCode::DIV)
                continue;

            auto* a_d = std::get_if<double>(&code[i].operand);
            auto* b_d = std::get_if<double>(&code[i + 1].operand);
            if (!a_d || !b_d)
                continue;

            double result = 0;
            if (op3 == OpCode::ADD)
                result = *a_d + *b_d;
            else if (op3 == OpCode::SUB)
                result = *a_d - *b_d;
            else if (op3 == OpCode::MUL)
                result = *a_d * *b_d;
            else if (op3 == OpCode::DIV) {
                if (*b_d == 0.0)
                    continue;
                result = *a_d / *b_d;
            }

            code[i].operand = result;
            code.erase(code.begin() + static_cast<ptrdiff_t>(i + 1), code.begin() + static_cast<ptrdiff_t>(i + 3));
            changed = true;
        }
    }
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

        load_module(imps->module_path);
    } else if (auto* ms = dynamic_cast<const MatchStmt*>(stmt.get())) {
        visit_match(*ms);
    } else if (auto* bs = dynamic_cast<const BreakStmt*>(stmt.get())) {
        visit_break(*bs);
    } else if (auto* cs = dynamic_cast<const ContinueStmt*>(stmt.get())) {
        visit_continue(*cs);
    } else if (auto* fs = dynamic_cast<const FunctionStmt*>(stmt.get())) {
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
    } else if (auto* le = dynamic_cast<const LambdaExpr*>(expr.get())) {
        visit_lambda(*le);
    } else if (auto* fe = dynamic_cast<const FString*>(expr.get())) {
        visit_fstring(*fe);
    } else if (auto* te = dynamic_cast<const TernaryExpr*>(expr.get())) {
        visit_ternary(*te);
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

    emit(OpCode::LOOP_START, static_cast<int64_t>(start_pos));

    visit_expr(stmt.condition);

    size_t exit_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));

    loop_stack_.push_back({start_pos, start_pos, {}, {}, stmt.label});

    visit(stmt.body);

    emit(OpCode::JUMP, static_cast<int64_t>(start_pos));

    patch_jump(exit_jump, bytecode_.size());

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

    size_t idx = bytecode_.size();
    emit(OpCode::BREAK_JUMP, static_cast<int64_t>(0), stmt.keyword.line);

    if (!stmt.label.empty()) {
        bool found = false;
        for (int i = static_cast<int>(loop_stack_.size()) - 1; i >= 0; --i) {
            if (loop_stack_[i].label == stmt.label) {
                loop_stack_[i].break_jumps.push_back(idx);
                found = true;
                break;
            }
        }
        if (!found) {
            throw CompileError("break label '" + stmt.label + "' not found at line " +
                               std::to_string(stmt.keyword.line));
        }
    } else {
        loop_stack_.back().break_jumps.push_back(idx);
    }
}

void Compiler::visit_continue(const ContinueStmt& stmt) {
    if (loop_stack_.empty()) {
        throw CompileError("'continue' outside of loop at line " + std::to_string(stmt.keyword.line));
    }

    size_t jump_idx = bytecode_.size();

    if (!stmt.label.empty()) {
        bool found = false;
        for (int i = static_cast<int>(loop_stack_.size()) - 1; i >= 0; --i) {
            if (loop_stack_[i].label == stmt.label) {
                emit(OpCode::CONTINUE_JUMP, static_cast<int64_t>(loop_stack_[i].continue_ip), stmt.keyword.line);
                loop_stack_[i].continue_jumps.push_back(jump_idx);
                found = true;
                break;
            }
        }
        if (!found) {
            throw CompileError("continue label '" + stmt.label + "' not found at line " +
                               std::to_string(stmt.keyword.line));
        }
    } else {
        emit(OpCode::CONTINUE_JUMP, static_cast<int64_t>(loop_stack_.back().continue_ip), stmt.keyword.line);
        loop_stack_.back().continue_jumps.push_back(jump_idx);
    }
}

void Compiler::visit_for(const ForStmt& stmt) {
    if (stmt.initializer) {
        visit(stmt.initializer);
    }

    size_t loop_start = bytecode_.size();
    emit(OpCode::LOOP_START, static_cast<int64_t>(loop_start));

    if (stmt.condition) {
        visit_expr(stmt.condition);
    } else {
        emit(OpCode::PUSH_CONST, 1.0);
    }

    size_t exit_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));

    loop_stack_.push_back({loop_start, 0, {}, {}, stmt.label});

    if (stmt.body) {
        visit(stmt.body);
    }

    size_t increment_start = bytecode_.size();
    loop_stack_.back().continue_ip = increment_start;
    for (size_t cj : loop_stack_.back().continue_jumps) {
        patch_jump(cj, increment_start);
    }

    if (stmt.increment) {
        visit_expr(stmt.increment);
        emit(OpCode::POP);
    }

    emit(OpCode::JUMP, static_cast<int64_t>(loop_start));

    patch_jump(exit_jump, bytecode_.size());

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
    visit_expr(stmt.expression);

    std::vector<size_t> false_patches;
    std::vector<size_t> exit_patches;
    std::vector<size_t> case_starts;

    for (size_t i = 0; i < stmt.cases.size(); i++) {
        const auto& case_stmt = stmt.cases[i];

        case_starts.push_back(bytecode_.size());

        emit(OpCode::DUP);

        visit_expr(case_stmt.pattern);
        emit(OpCode::EQ);

        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));
        false_patches.push_back(bytecode_.size() - 1);

        emit(OpCode::POP);

        visit(case_stmt.body);

        emit(OpCode::JUMP, static_cast<int64_t>(0));
        exit_patches.push_back(bytecode_.size() - 1);
    }

    size_t default_or_end = bytecode_.size();

    if (stmt.default_case) {
        emit(OpCode::POP);
        visit(stmt.default_case);
    }

    size_t end_of_match = bytecode_.size();

    for (size_t idx : exit_patches) {
        patch_jump(idx, end_of_match);
    }

    for (size_t i = 0; i < false_patches.size(); i++) {
        if (i + 1 < case_starts.size()) {
            patch_jump(false_patches[i], case_starts[i + 1]);
        } else {
            patch_jump(false_patches[i], default_or_end);
        }
    }
}

void Compiler::visit_class(const ClassStmt&) {}

void Compiler::visit_binary(const Binary& expr) {
    visit_expr(expr.left);
    visit_expr(expr.right);

    switch (expr.op.type) {
    case TokenType::PLUS:
        emit(OpCode::ADD);
        break;
    case TokenType::MINUS:
        emit(OpCode::SUB);
        break;
    case TokenType::STAR:
        emit(OpCode::MUL);
        break;
    case TokenType::SLASH:
        emit(OpCode::DIV);
        break;
    case TokenType::PERCENT:
        emit(OpCode::PERCENT);
        break;
    case TokenType::DOUBLE_EQUALS:
        emit(OpCode::EQ);
        break;
    case TokenType::NOT_EQUALS:
        emit(OpCode::NE);
        break;
    case TokenType::GREATER:
        emit(OpCode::GT);
        break;
    case TokenType::GREATER_EQUALS:
        emit(OpCode::GE);
        break;
    case TokenType::LESS:
        emit(OpCode::LT);
        break;
    case TokenType::LESS_EQUALS:
        emit(OpCode::LE);
        break;
    default:
        break;
    }
}

void Compiler::visit_unary(const Unary& expr) {
    switch (expr.op.type) {
    case TokenType::NOT:
        visit_expr(expr.right);
        emit(OpCode::NOT);
        break;
    case TokenType::MINUS:
        emit(OpCode::PUSH_CONST, 0.0);
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
        size_t false_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));

        visit_expr(expr.right);
        size_t to_end = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));

        patch_jump(false_jump, bytecode_.size());
        emit(OpCode::PUSH_CONST, 0.0);
        patch_jump(to_end, bytecode_.size());
    } else if (expr.op.type == TokenType::OR) {
        size_t true_jump = bytecode_.size();
        emit(OpCode::JUMP_IF_TRUE, static_cast<int64_t>(0));

        visit_expr(expr.right);
        size_t to_end = bytecode_.size();
        emit(OpCode::JUMP, static_cast<int64_t>(0));

        patch_jump(true_jump, bytecode_.size());
        emit(OpCode::PUSH_CONST, 1.0);
        patch_jump(to_end, bytecode_.size());
    }
}

void Compiler::visit_literal(const Literal& expr) {
    std::visit(
        [this](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                emit(OpCode::PUSH_CONST, nullptr);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                emit(OpCode::PUSH_CONST, value);
            } else if constexpr (std::is_same_v<T, double>) {
                emit(OpCode::PUSH_CONST, value);
            } else if constexpr (std::is_same_v<T, std::string>) {
                emit(OpCode::PUSH_CONST, value);
            }
        },
        expr.value);
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
    for (const auto& arg : expr.arguments) {
        visit_expr(arg);
    }

    emit(OpCode::NEW, std::make_pair(sv_to_str(expr.name.lexeme), static_cast<int>(expr.arguments.size())));
}

void Compiler::visit_call(const Call& expr) {
    if (auto* get = dynamic_cast<const Get*>(expr.callee.get())) {
        if (auto* var = dynamic_cast<const Variable*>(get->obj.get())) {
            std::string obj_name = sv_to_str(var->name.lexeme);
            if (obj_name == "super") {
                emit(OpCode::LOAD_SUPER);
                for (const auto& arg : expr.arguments) {
                    visit_expr(arg);
                }
                std::string method_name = sv_to_str(get->name.lexeme);
                emit(OpCode::CALL, std::make_pair(method_name, static_cast<int>(expr.arguments.size())));
                return;
            }
        }

        visit_expr(get->obj);
        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
        }

        std::string method_name = sv_to_str(get->name.lexeme);
        if (method_name == "o") {
            emit(OpCode::PRINT);
        } else {
            emit(OpCode::CALL, std::make_pair(method_name, static_cast<int>(expr.arguments.size())));
        }
    } else if (auto* var = dynamic_cast<const Variable*>(expr.callee.get())) {
        std::string var_name = sv_to_str(var->name.lexeme);

        if (var_name == "super") {
            emit(OpCode::LOAD_SUPER);
            for (const auto& arg : expr.arguments) {
                visit_expr(arg);
            }
            emit(OpCode::CALL, std::make_pair(std::string("super"), static_cast<int>(expr.arguments.size())));
            return;
        }

        if (var_name == "z") {
            emit(OpCode::PUSH_CONST, std::string("SYSTEM_Z"));
        } else {
            visit_expr(expr.callee);
        }

        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
        }

        emit(OpCode::CALL, std::make_pair(var_name, static_cast<int>(expr.arguments.size())));
    } else {
        visit_expr(expr.callee);
        for (const auto& arg : expr.arguments) {
            visit_expr(arg);
        }
        emit(OpCode::CALL, std::make_pair(std::string("lambda"), static_cast<int>(expr.arguments.size())));
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

void Compiler::visit_lambda(const LambdaExpr& expr) {
    std::string lambda_name = "__lambda_" + std::to_string(lambda_counter_++);

    std::vector<std::string> param_names;
    for (const auto& param : expr.params) {
        param_names.push_back(sv_to_str(param.name.lexeme));
    }

    std::vector<Instruction> old_bytecode = std::move(bytecode_);
    bytecode_.clear();

    for (const auto& stmt : expr.body) {
        visit(stmt);
    }

    if (bytecode_.empty() || bytecode_.back().op != OpCode::RET) {
        emit(OpCode::PUSH_CONST, nullptr);
        emit(OpCode::RET);
    }

    CompiledMethod info;
    info.bytecode = std::move(bytecode_);
    info.param_names = std::move(param_names);
    pending_functions_[lambda_name] = std::move(info);

    bytecode_ = std::move(old_bytecode);

    emit(OpCode::PUSH_CONST, lambda_name);
}

void Compiler::visit_ternary(const TernaryExpr& expr) {
    visit_expr(expr.condition);

    size_t false_jump = bytecode_.size();
    emit(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(0));

    visit_expr(expr.true_expr);

    size_t end_jump = bytecode_.size();
    emit(OpCode::JUMP, static_cast<int64_t>(0));

    patch_jump(false_jump, bytecode_.size());

    visit_expr(expr.false_expr);

    patch_jump(end_jump, bytecode_.size());
}

void Compiler::visit_fstring(const FString& expr) {
    if (expr.parts.empty()) {
        emit(OpCode::PUSH_CONST, std::string(""));
        return;
    }

    const auto& first = expr.parts[0];
    if (first.is_literal) {
        emit(OpCode::PUSH_CONST, first.literal);
    } else {
        visit_expr(first.expr);
    }

    for (size_t i = 1; i < expr.parts.size(); ++i) {
        const auto& part = expr.parts[i];
        if (part.is_literal) {
            emit(OpCode::PUSH_CONST, part.literal);
        } else {
            visit_expr(part.expr);
        }
        emit(OpCode::ADD);
    }
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

    for (const auto& field : stmt.fields) {
        if (!field.is_static && field.initializer) {
            emit(OpCode::LOAD_VAR, std::string("this"));

            visit_expr(field.initializer);

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
    if (loaded_modules_.count(path))
        return;
    loaded_modules_.insert(path);

    std::string resolved_path = path;
    if (!source_dir_.empty() && path.size() > 0 && path[0] != '/') {
        resolved_path = source_dir_ + "/" + path;
    }

    {
        std::ifstream test(resolved_path);
        if (!test.good()) {
            const char* env_path = std::getenv("ALPHABET_PATH");
            if (env_path) {
                std::string env_str(env_path);
                size_t start = 0;
                while (start < env_str.size()) {
                    size_t end = env_str.find(':', start);
                    if (end == std::string::npos)
                        end = env_str.size();
                    std::string dir = env_str.substr(start, end - start);
                    std::string candidate = dir + "/" + path;
                    std::ifstream t2(candidate);
                    if (t2.good()) {
                        resolved_path = candidate;
                        break;
                    }
                    start = end + 1;
                }
            }
        }
    }

    std::ifstream file(resolved_path);
    if (!file.is_open()) {
        throw CompileError("Cannot import module: " + path);
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string source = oss.str();

    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, source);
    auto statements = parser.parse();

    if (parser.had_errors()) {
        throw CompileError("Syntax errors in imported module: " + path);
    }

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

    for (const auto& stmt : statements) {
        if (auto* class_stmt = dynamic_cast<ClassStmt*>(stmt.get())) {
            if (!class_stmt->is_interface) {
                std::string name = sv_to_str(class_stmt->name.lexeme);
                CompiledClass cls = compile_class_def(*class_stmt);
                pending_classes_[name] = cls;

                if (!cls.static_init.empty()) {
                    for (const auto& instr : cls.static_init) {
                        bytecode_.push_back(instr);
                    }
                }
            }
        }
    }

    for (const auto& stmt : statements) {
        if (dynamic_cast<ClassStmt*>(stmt.get()))
            continue;

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

std::string Compiler::dump_program(const Program& program) {
    std::ostringstream oss;

    auto dump_instructions = [&](const std::vector<Instruction>& bytecode, const std::string& label) {
        oss << "=== " << label << " ===\n";
        for (size_t i = 0; i < bytecode.size(); ++i) {
            const auto& instr = bytecode[i];
            oss << "  " << i << ": " << opcode_to_string(instr.op);
            std::visit(
                [&oss](const auto& op) {
                    using T = std::decay_t<decltype(op)>;
                    if constexpr (std::is_same_v<T, int64_t>) {
                        oss << " " << op;
                    } else if constexpr (std::is_same_v<T, double>) {
                        oss << " " << op;
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        oss << " \"" << op << "\"";
                    } else if constexpr (std::is_same_v<T, std::pair<std::string, int>>) {
                        oss << " " << op.first << "/" << op.second;
                    }
                },
                instr.operand);
            if (instr.line > 0)
                oss << "  (line " << instr.line << ")";
            oss << "\n";
        }
    };

    dump_instructions(program.main, "MAIN");

    if (!program.static_init.empty()) {
        dump_instructions(program.static_init, "STATIC_INIT");
    }

    for (const auto& [name, func] : program.functions) {
        std::string params;
        for (size_t i = 0; i < func.param_names.size(); ++i) {
            if (i > 0)
                params += ", ";
            params += func.param_names[i];
        }
        dump_instructions(func.bytecode, "FUNCTION " + name + "(" + params + ")");
    }

    for (const auto& [id, cls] : program.classes) {
        oss << "\n=== CLASS " << cls.name << " (id=" << id << ") ===\n";
        if (!cls.superclass.empty()) {
            oss << "  extends " << cls.superclass << "\n";
        }
        for (const auto& [mname, method] : cls.methods) {
            std::string params;
            for (size_t i = 0; i < method.param_names.size(); ++i) {
                if (i > 0)
                    params += ", ";
                params += method.param_names[i];
            }
            dump_instructions(method.bytecode, "  METHOD " + mname + "(" + params + ")");
        }
        for (const auto& [mname, method] : cls.static_methods) {
            dump_instructions(method.bytecode, "  STATIC " + mname);
        }
    }

    if (!program.globals.empty()) {
        oss << "\n=== GLOBALS ===\n";
        for (size_t i = 0; i < program.globals.size(); ++i) {
            oss << "  " << i << ": " << program.globals[i] << "\n";
        }
    }

    return oss.str();
}

} // namespace alphabet
