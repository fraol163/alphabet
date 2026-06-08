#ifndef ALPHABET_TRACE_UTILS_H
#define ALPHABET_TRACE_UTILS_H

#include "alphabet_ast.h"
#include "lexer.h"
#include <sstream>
#include <string>
#include <vector>

namespace alphabet {

inline std::string token_type_to_str(TokenType type) {
    switch (type) {
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::LOOP:
        return "LOOP";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::CONTINUE:
        return "CONTINUE";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::CLASS:
        return "CLASS";
    case TokenType::ABSTRACT:
        return "ABSTRACT";
    case TokenType::INTERFACE:
        return "INTERFACE";
    case TokenType::NEW:
        return "NEW";
    case TokenType::PUBLIC:
        return "PUBLIC";
    case TokenType::PRIVATE:
        return "PRIVATE";
    case TokenType::STATIC:
        return "STATIC";
    case TokenType::METHOD:
        return "METHOD";
    case TokenType::TRY:
        return "TRY";
    case TokenType::HANDLE:
        return "HANDLE";
    case TokenType::SYSTEM:
        return "SYSTEM";
    case TokenType::IMPORT:
        return "IMPORT";
    case TokenType::MATCH:
        return "MATCH";
    case TokenType::TOK_CONST:
        return "CONST";
    case TokenType::QUESTION:
        return "QUESTION";
    case TokenType::EXTENDS:
        return "EXTENDS";
    case TokenType::EXPORT:
        return "EXPORT";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::STRING:
        return "STRING";
    case TokenType::FSTRING:
        return "FSTRING";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::PERCENT:
        return "PERCENT";
    case TokenType::EQUALS:
        return "EQUALS";
    case TokenType::DOUBLE_EQUALS:
        return "DOUBLE_EQUALS";
    case TokenType::NOT_EQUALS:
        return "NOT_EQUALS";
    case TokenType::GREATER:
        return "GREATER";
    case TokenType::LESS:
        return "LESS";
    case TokenType::GREATER_EQUALS:
        return "GREATER_EQUALS";
    case TokenType::LESS_EQUALS:
        return "LESS_EQUALS";
    case TokenType::AND:
        return "AND";
    case TokenType::OR:
        return "OR";
    case TokenType::NOT:
        return "NOT";
    case TokenType::DOT:
        return "DOT";
    case TokenType::DOTDOT:
        return "DOTDOT";
    case TokenType::QUESTION_DOT:
        return "QUESTION_DOT";
    case TokenType::LBRACE:
        return "LBRACE";
    case TokenType::RBRACE:
        return "RBRACE";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::LBRACKET:
        return "LBRACKET";
    case TokenType::RBRACKET:
        return "RBRACKET";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::COLON:
        return "COLON";
    case TokenType::EOF_TOKEN:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

inline std::string expr_to_string(const ExprPtr& expr) {
    if (!expr)
        return "null";
    std::ostringstream oss;

    if (auto lit = std::dynamic_pointer_cast<Literal>(expr)) {
        if (std::holds_alternative<int64_t>(lit->value)) {
            oss << "Lit(" << std::get<int64_t>(lit->value) << ")";
        } else if (std::holds_alternative<double>(lit->value)) {
            oss << "Lit(" << std::get<double>(lit->value) << ")";
        } else if (std::holds_alternative<std::string>(lit->value)) {
            oss << "Lit(\"" << std::get<std::string>(lit->value) << "\")";
        } else {
            oss << "Lit(null)";
        }
        return oss.str();
    }

    if (auto var = std::dynamic_pointer_cast<Variable>(expr)) {
        return "Var(" + std::string(var->name.lexeme) + ")";
    }

    if (auto bin = std::dynamic_pointer_cast<Binary>(expr)) {
        return std::string(bin->op.lexeme) + "(" + expr_to_string(bin->left) + ", " + expr_to_string(bin->right) + ")";
    }

    if (auto un = std::dynamic_pointer_cast<Unary>(expr)) {
        return std::string(un->op.lexeme) + "(" + expr_to_string(un->right) + ")";
    }

    if (auto tern = std::dynamic_pointer_cast<TernaryExpr>(expr)) {
        return "Ternary(" + expr_to_string(tern->condition) + ", " + expr_to_string(tern->true_expr) + ", " +
               expr_to_string(tern->false_expr) + ")";
    }

    if (auto fstr = std::dynamic_pointer_cast<FString>(expr)) {
        oss << "FString(";
        for (size_t i = 0; i < fstr->parts.size(); ++i) {
            if (i > 0)
                oss << ", ";
            if (fstr->parts[i].is_literal) {
                oss << "\"" << fstr->parts[i].literal << "\"";
            } else {
                oss << expr_to_string(fstr->parts[i].expr);
            }
        }
        oss << ")";
        return oss.str();
    }

    if (auto grp = std::dynamic_pointer_cast<Grouping>(expr)) {
        return "Group(" + expr_to_string(grp->expression) + ")";
    }

    if (auto ass = std::dynamic_pointer_cast<Assign>(expr)) {
        return "Assign(" + std::string(ass->name.lexeme) + ", " + expr_to_string(ass->value) + ")";
    }

    if (auto log = std::dynamic_pointer_cast<Logical>(expr)) {
        return std::string(log->op.lexeme) + "(" + expr_to_string(log->left) + ", " + expr_to_string(log->right) + ")";
    }

    if (auto call = std::dynamic_pointer_cast<Call>(expr)) {
        oss << "Call(" << expr_to_string(call->callee) << ", [";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << expr_to_string(call->arguments[i]);
        }
        oss << "])";
        return oss.str();
    }

    if (auto get = std::dynamic_pointer_cast<Get>(expr)) {
        return "Get(" + expr_to_string(get->obj) + "." + std::string(get->name.lexeme) + ")";
    }

    if (auto nsg = std::dynamic_pointer_cast<NullSafeGet>(expr)) {
        return "NullSafe(" + expr_to_string(nsg->obj) + "." + std::string(nsg->name.lexeme) + ")";
    }

    if (auto set = std::dynamic_pointer_cast<Set>(expr)) {
        return "Set(" + expr_to_string(set->obj) + "." + std::string(set->name.lexeme) + ", " +
               expr_to_string(set->value) + ")";
    }

    if (auto nw = std::dynamic_pointer_cast<New>(expr)) {
        oss << "New(" << std::string(nw->name.lexeme) << ", [";
        for (size_t i = 0; i < nw->arguments.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << expr_to_string(nw->arguments[i]);
        }
        oss << "])";
        return oss.str();
    }

    if (auto ll = std::dynamic_pointer_cast<ListLiteral>(expr)) {
        oss << "[";
        for (size_t i = 0; i < ll->elements.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << expr_to_string(ll->elements[i]);
        }
        oss << "]";
        return oss.str();
    }

    if (auto ml = std::dynamic_pointer_cast<MapLiteral>(expr)) {
        oss << "Map({";
        for (size_t i = 0; i < ml->keys.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << expr_to_string(ml->keys[i]) << ": " << expr_to_string(ml->values[i]);
        }
        oss << "})";
        return oss.str();
    }

    if (auto idx = std::dynamic_pointer_cast<IndexExpr>(expr)) {
        return "Index(" + expr_to_string(idx->obj) + "[" + expr_to_string(idx->index) + "])";
    }

    if (auto ia = std::dynamic_pointer_cast<IndexAssign>(expr)) {
        return "IndexAssign(" + expr_to_string(ia->obj) + "[" + expr_to_string(ia->index) + "], " +
               expr_to_string(ia->value) + ")";
    }

    if (auto lam = std::dynamic_pointer_cast<LambdaExpr>(expr)) {
        oss << "Lambda([";
        for (size_t i = 0; i < lam->params.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << std::string(lam->params[i].name.lexeme);
        }
        oss << "], ...)";
        return oss.str();
    }

    return "Expr(?)";
}

inline std::string stmt_to_string(const StmtPtr& stmt) {
    if (!stmt)
        return "null";
    std::ostringstream oss;

    if (auto expr_stmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        return "ExprStmt(" + expr_to_string(expr_stmt->expression) + ")";
    }

    if (auto var = std::dynamic_pointer_cast<VarStmt>(stmt)) {
        oss << "VarDecl(" << std::string(var->name.lexeme) << ": " << std::string(var->type_id.lexeme);
        if (var->initializer) {
            oss << " = " << expr_to_string(var->initializer);
        }
        if (var->is_const)
            oss << " [const]";
        if (var->is_static)
            oss << " [static]";
        oss << ")";
        return oss.str();
    }

    if (auto block = std::dynamic_pointer_cast<Block>(stmt)) {
        oss << "Block(" << block->statements.size() << " stmts)";
        return oss.str();
    }

    if (auto if_stmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        oss << "If(" << expr_to_string(if_stmt->condition) << ", " << stmt_to_string(if_stmt->then_branch);
        if (if_stmt->else_branch) {
            oss << ", " << stmt_to_string(if_stmt->else_branch);
        }
        oss << ")";
        return oss.str();
    }

    if (auto loop = std::dynamic_pointer_cast<LoopStmt>(stmt)) {
        oss << (loop->is_do_while ? "DoWhile(" : "Loop(");
        oss << expr_to_string(loop->condition) << ", " << stmt_to_string(loop->body);
        if (!loop->label.empty()) {
            oss << ", label=" << loop->label;
        }
        oss << ")";
        return oss.str();
    }

    if (auto for_stmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        oss << "For(";
        if (for_stmt->initializer)
            oss << stmt_to_string(for_stmt->initializer);
        oss << "; ";
        if (for_stmt->condition)
            oss << expr_to_string(for_stmt->condition);
        oss << "; ";
        if (for_stmt->increment)
            oss << expr_to_string(for_stmt->increment);
        oss << ", " << stmt_to_string(for_stmt->body);
        if (!for_stmt->label.empty()) {
            oss << ", label=" << for_stmt->label;
        }
        oss << ")";
        return oss.str();
    }

    if (auto try_stmt = std::dynamic_pointer_cast<TryStmt>(stmt)) {
        oss << "Try(" << try_stmt->try_block.statements.size() << " stmts, "
            << std::string(try_stmt->exception_var.lexeme) << ", " << try_stmt->handle_block.statements.size()
            << " stmts)";
        return oss.str();
    }

    if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        oss << "Return(";
        if (ret->value) {
            oss << expr_to_string(ret->value);
        }
        oss << ")";
        return oss.str();
    }

    if (auto brk = std::dynamic_pointer_cast<BreakStmt>(stmt)) {
        oss << "Break";
        if (!brk->label.empty()) {
            oss << "(" << brk->label << ")";
        }
        return oss.str();
    }

    if (auto cont = std::dynamic_pointer_cast<ContinueStmt>(stmt)) {
        oss << "Continue";
        if (!cont->label.empty()) {
            oss << "(" << cont->label << ")";
        }
        return oss.str();
    }

    if (auto func = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
        oss << "FuncDecl(" << std::string(func->name.lexeme) << ", [";
        for (size_t i = 0; i < func->params.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << std::string(func->params[i].name.lexeme);
        }
        oss << "], return " << std::string(func->return_type.lexeme);
        if (func->is_static)
            oss << ", [static]";
        if (func->is_abstract)
            oss << ", [abstract]";
        oss << ")";
        return oss.str();
    }

    if (auto cls = std::dynamic_pointer_cast<ClassStmt>(stmt)) {
        oss << "ClassDecl(" << std::string(cls->name.lexeme);
        if (cls->superclass) {
            oss << " extends " << std::string(cls->superclass->name.lexeme);
        }
        oss << ", " << cls->methods.size() << " methods, " << cls->fields.size() << " fields";
        if (cls->is_interface)
            oss << ", [interface]";
        if (cls->is_abstract)
            oss << ", [abstract]";
        oss << ")";
        return oss.str();
    }

    if (auto imp = std::dynamic_pointer_cast<ImportStmt>(stmt)) {
        oss << "Import(" << imp->module_path;
        if (imp->alias) {
            oss << " as " << *imp->alias;
        }
        oss << ")";
        return oss.str();
    }

    if (auto exp = std::dynamic_pointer_cast<ExportStmt>(stmt)) {
        oss << "Export([";
        for (size_t i = 0; i < exp->names.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << std::string(exp->names[i].lexeme);
        }
        oss << "])";
        return oss.str();
    }

    if (auto match = std::dynamic_pointer_cast<MatchStmt>(stmt)) {
        oss << "Match(" << expr_to_string(match->expression) << ", " << match->cases.size() << " cases";
        if (match->default_case) {
            oss << ", default";
        }
        oss << ")";
        return oss.str();
    }

    return "Stmt(?)";
}

inline std::string format_tokens(const std::vector<Token>& tokens) {
    std::ostringstream oss;
    for (const auto& tok : tokens) {
        oss << "  TOKENS: " << token_type_to_str(tok.type) << "(" << tok.lexeme << ")\n";
    }
    return oss.str();
}

inline std::string format_ast(const std::vector<StmtPtr>& stmts) {
    std::ostringstream oss;
    for (const auto& stmt : stmts) {
        oss << "  AST: " << stmt_to_string(stmt) << "\n";
    }
    return oss.str();
}

} // namespace alphabet

#endif
