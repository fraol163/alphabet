#ifndef ALPHABET_COMPILER_H
#define ALPHABET_COMPILER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "bytecode.h"
#include "alphabet_ast.h"
#include "type_system.h"

namespace alphabet {

class CompileError : public std::runtime_error {
public:
    explicit CompileError(const std::string& msg) : std::runtime_error(msg) {}
};

using CompiledClass = alphabet::CompiledClass;
using CompiledMethod = alphabet::CompiledMethod;

class Compiler {
public:
    Compiler();

    Program compile(const std::vector<StmtPtr>& statements);

private:
    std::vector<Instruction> bytecode_;
    std::unordered_map<std::string, uint16_t> class_map_;
    uint16_t next_class_id_ = 15;
    std::vector<std::string> globals_;
    TypeManager type_manager_;

    void validate_types(const std::vector<StmtPtr>& statements);
    void validate_expression_type(const ExprPtr& expr, uint16_t expected_type);
    uint16_t infer_expression_type(const ExprPtr& expr);
    bool types_compatible(uint16_t source, uint16_t target);

    void emit(OpCode op, Operand operand = std::monostate{});

    void patch_jump(size_t index, size_t target);
    size_t get_global_index(const std::string& name);
    void visit(const StmtPtr& stmt);
    void visit_expr(const ExprPtr& expr);

    void visit_class(const ClassStmt& stmt);
    void visit_var(const VarStmt& stmt);
    void visit_expression(const ExpressionStmt& stmt);
    void visit_if(const IfStmt& stmt);
    void visit_loop(const LoopStmt& stmt);
    void visit_try(const TryStmt& stmt);
    void visit_block(const Block& stmt);
    void visit_return(const ReturnStmt& stmt);

    void visit_binary(const Binary& expr);
    void visit_unary(const Unary& expr);
    void visit_literal(const Literal& expr);
    void visit_grouping(const Grouping& expr);
    void visit_variable(const Variable& expr);
    void visit_assign(const Assign& expr);
    void visit_logical(const Logical& expr);
    void visit_call(const Call& expr);
    void visit_get(const Get& expr);
    void visit_set(const Set& expr);
    void visit_new(const New& expr);
    void visit_list(const ListLiteral& expr);
    void visit_map(const MapLiteral& expr);
    void visit_index(const IndexExpr& expr);

    CompiledClass compile_class_def(const ClassStmt& stmt);
    std::vector<Instruction> compile_method(const FunctionStmt& method);
};

}

#endif
