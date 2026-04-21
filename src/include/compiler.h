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
    void set_source_dir(const std::string& dir) { source_dir_ = dir; }

private:
    std::vector<Instruction> bytecode_;
    std::unordered_map<std::string, uint16_t> class_map_;
    uint16_t next_class_id_ = 15;
    std::vector<std::string> globals_;
    std::unordered_set<std::string> const_vars_;  // Track const variables
    std::unordered_map<std::string, uint16_t> var_types_;  // Track declared variable types
    std::unordered_map<std::string, CompiledMethod> pending_functions_;
    std::unordered_map<std::string, CompiledClass> pending_classes_;  // Classes from imports
    TypeManager type_manager_;
    
    // Loop context tracking for break/continue
    struct LoopContext {
        size_t loop_start_ip;           // IP of the LOOP_START instruction
        std::vector<size_t> break_jumps; // Indices of BREAK_JUMP instructions to patch
    };
    std::vector<LoopContext> loop_stack_;
    
    // Module system
    std::vector<std::string> imported_modules_;
    std::unordered_map<std::string, std::string> module_aliases_;
    std::unordered_set<std::string> loaded_modules_;  // Track loaded modules to avoid duplicates
    std::string source_dir_;  // Directory of the source file for resolving imports
    
    void load_module(const std::string& path);

    void validate_types(const std::vector<StmtPtr>& statements);
    void validate_expression_type(const ExprPtr& expr, uint16_t expected_type);
    uint16_t infer_expression_type(const ExprPtr& expr);
    bool types_compatible(uint16_t source, uint16_t target);

    void emit(OpCode op, Operand operand = std::monostate{}, int line = 0);

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
    void visit_match(const MatchStmt& stmt);
    void visit_break(const BreakStmt& stmt);
    void visit_continue(const ContinueStmt& stmt);
    void visit_for(const ForStmt& stmt);

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
    void visit_index_assign(const IndexAssign& expr);

    CompiledClass compile_class_def(const ClassStmt& stmt);
    std::vector<Instruction> compile_method(const FunctionStmt& method);

public:
    // Debug: dump bytecode in human-readable form
    static std::string dump_program(const Program& program);
};

}

#endif
