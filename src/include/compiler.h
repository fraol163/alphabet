#ifndef ALPHABET_COMPILER_H
#define ALPHABET_COMPILER_H

#include "alphabet_ast.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "bytecode.h"
#include "type_system.h"

namespace alphabet {

class CompileError : public std::runtime_error
{
  public:
    explicit CompileError(const std::string &msg) : std::runtime_error(msg) {}
};

using CompiledClass = alphabet::CompiledClass;
using CompiledMethod = alphabet::CompiledMethod;

class Compiler
{
  public:
    Compiler();

    Program compile(const std::vector<StmtPtr> &statements);
    void set_source_dir(const std::string &dir)
    {
        source_dir_ = dir;
    }

  private:
    std::vector<Instruction> bytecode_;
    std::unordered_map<std::string, uint16_t> class_map_;
    uint16_t next_class_id_ = 15;
    int lambda_counter_ = 0;
    std::vector<std::string> globals_;
    std::unordered_set<std::string> const_vars_;          
    std::unordered_map<std::string, uint16_t> var_types_; 
    std::unordered_map<std::string, CompiledMethod> pending_functions_;
    std::unordered_map<std::string, CompiledClass> pending_classes_; 
    TypeManager type_manager_;

    struct LoopContext
    {
        size_t loop_start_ip;            
        std::vector<size_t> break_jumps; 
    };
    std::vector<LoopContext> loop_stack_;

    std::vector<std::string> imported_modules_;
    std::unordered_map<std::string, std::string> module_aliases_;
    std::unordered_set<std::string> loaded_modules_; 
    std::string source_dir_; 

    void load_module(const std::string &path);

    void validate_types(const std::vector<StmtPtr> &statements);
    void validate_expression_type(const ExprPtr &expr, uint16_t expected_type);
    uint16_t infer_expression_type(const ExprPtr &expr);
    bool types_compatible(uint16_t source, uint16_t target);

    void emit(OpCode op, Operand operand = std::monostate{}, int line = 0);

    void patch_jump(size_t index, size_t target);
    size_t get_global_index(const std::string &name);
    void visit(const StmtPtr &stmt);
    void visit_expr(const ExprPtr &expr);

    void visit_class(const ClassStmt &stmt);
    void visit_var(const VarStmt &stmt);
    void visit_expression(const ExpressionStmt &stmt);
    void visit_if(const IfStmt &stmt);
    void visit_loop(const LoopStmt &stmt);
    void visit_try(const TryStmt &stmt);
    void visit_block(const Block &stmt);
    void visit_return(const ReturnStmt &stmt);
    void visit_match(const MatchStmt &stmt);
    void visit_break(const BreakStmt &stmt);
    void visit_continue(const ContinueStmt &stmt);
    void visit_for(const ForStmt &stmt);

    void visit_binary(const Binary &expr);
    void visit_unary(const Unary &expr);
    void visit_literal(const Literal &expr);
    void visit_grouping(const Grouping &expr);
    void visit_variable(const Variable &expr);
    void visit_assign(const Assign &expr);
    void visit_logical(const Logical &expr);
    void visit_call(const Call &expr);
    void visit_get(const Get &expr);
    void visit_set(const Set &expr);
    void visit_new(const New &expr);
    void visit_list(const ListLiteral &expr);
    void visit_map(const MapLiteral &expr);
    void visit_index(const IndexExpr &expr);
    void visit_index_assign(const IndexAssign &expr);
    void visit_lambda(const LambdaExpr &expr);
    void visit_fstring(const FString &expr);

    CompiledClass compile_class_def(const ClassStmt &stmt);
    std::vector<Instruction> compile_method(const FunctionStmt &method);

  public:

    static std::string dump_program(const Program &program);
};

} 

#endif
