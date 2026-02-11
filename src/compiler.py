from typing import List, Dict, Any, Optional
from alphabet_ast import *
from lexer import Token, TokenType
from bytecode import OpCode

class Compiler:
    def __init__(self):
        self.bytecode: List[tuple] = []
        self.class_map: Dict[str, int] = {}
        self.next_class_id = 15
        self.globals: List[str] = []
        
    def compile(self, statements: List[Stmt]) -> dict:
        for stmt in statements:
            if isinstance(stmt, ClassStmt) and not stmt.is_interface:
                if stmt.name.lexeme not in self.class_map:
                    self.class_map[stmt.name.lexeme] = self.next_class_id
                    self.next_class_id += 1
        classes = {}
        main_bytecode = []
        self.bytecode = [] 
        for stmt in statements:
            if isinstance(stmt, ClassStmt):
                if not stmt.is_interface:
                    classes[stmt.name.lexeme] = self.compile_class(stmt)
            else:
                self.visit(stmt)
        main_bytecode = self.bytecode
        main_bytecode.append((OpCode.HALT, None))
        return {"classes": classes, "main": main_bytecode}

    def compile_class(self, stmt: ClassStmt):
        methods = {}
        static_methods = {}
        method_nodes = {}
        for method in stmt.methods:
            method_nodes[method.name.lexeme] = method
            if method.is_static:
                static_methods[method.name.lexeme] = self.compile_method(method)
            else:
                methods[method.name.lexeme] = self.compile_method(method)
        old_bytecode = self.bytecode
        self.bytecode = []
        for field in stmt.fields:
            if field.is_static and field.initializer:
                self.emit(OpCode.PUSH_CONST, self.class_map[stmt.name.lexeme]) 
                self.visit(field.initializer) 
                self.emit(OpCode.SET_STATIC, field.name.lexeme)
                self.emit(OpCode.POP)
        static_init = self.bytecode
        self.bytecode = old_bytecode
        return {
            "name": stmt.name.lexeme,
            "superclass": stmt.superclass.name.lexeme if stmt.superclass else None,
            "methods": methods,
            "method_nodes": method_nodes,
            "static_methods": static_methods,
            "static_init": static_init,
            "id": self.class_map[stmt.name.lexeme],
            "fields": {f.name.lexeme: f for f in stmt.fields if not f.is_static},
            "static_fields": {f.name.lexeme: f for f in stmt.fields if f.is_static}
        }

    def compile_method(self, method: FunctionStmt):
        old_bytecode = self.bytecode
        self.bytecode = []
        for stmt in method.body:
            self.visit(stmt)
        if not self.bytecode or self.bytecode[-1][0] != OpCode.RET:
            self.emit(OpCode.PUSH_CONST, None)
            self.emit(OpCode.RET)
        method_bytecode = self.bytecode
        self.bytecode = old_bytecode
        return method_bytecode

    def emit(self, opcode: OpCode, operand: Any = None):
        self.bytecode.append((opcode, operand))

    def visit(self, node: Any):
        method_name = f"visit_{type(node).__name__}"
        visitor = getattr(self, method_name, self.no_visit)
        return visitor(node)

    def no_visit(self, node):
        raise Exception(f"No visitor for {type(node).__name__}")

    def visit_ReturnStmt(self, stmt: ReturnStmt):
        if stmt.value:
            self.visit(stmt.value)
        else:
            self.emit(OpCode.PUSH_CONST, None)
        self.emit(OpCode.RET)

    def visit_VarStmt(self, stmt: VarStmt):
        if stmt.initializer:
            self.visit(stmt.initializer)
        else:
            self.emit(OpCode.PUSH_CONST, None)
        if stmt.name.lexeme not in self.globals:
            self.globals.append(stmt.name.lexeme)
        idx = self.globals.index(stmt.name.lexeme)
        self.emit(OpCode.STORE_VAR, idx)

    def visit_ExpressionStmt(self, stmt: ExpressionStmt):
        self.visit(stmt.expression)
        self.emit(OpCode.POP)

    def visit_IfStmt(self, stmt: IfStmt):
        self.visit(stmt.condition)
        false_jump = len(self.bytecode)
        self.emit(OpCode.JUMP_IF_FALSE, 0)
        self.visit(stmt.then_branch)
        if stmt.else_branch:
            exit_jump = len(self.bytecode)
            self.emit(OpCode.JUMP, 0)
            self.bytecode[false_jump] = (OpCode.JUMP_IF_FALSE, len(self.bytecode))
            self.visit(stmt.else_branch)
            self.bytecode[exit_jump] = (OpCode.JUMP, len(self.bytecode))
        else:
            self.bytecode[false_jump] = (OpCode.JUMP_IF_FALSE, len(self.bytecode))

    def visit_LoopStmt(self, stmt: LoopStmt):
        start_pos = len(self.bytecode)
        self.visit(stmt.condition)
        exit_jump = len(self.bytecode)
        self.emit(OpCode.JUMP_IF_FALSE, 0)
        self.visit(stmt.body)
        self.emit(OpCode.JUMP, start_pos)
        self.bytecode[exit_jump] = (OpCode.JUMP_IF_FALSE, len(self.bytecode))

    def visit_TryStmt(self, stmt: TryStmt):
        setup_try_idx = len(self.bytecode)
        self.emit(OpCode.SETUP_TRY, 0)
        self.visit(stmt.try_block)
        self.emit(OpCode.POP_TRY)
        exit_jump_idx = len(self.bytecode)
        self.emit(OpCode.JUMP, 0)
        self.bytecode[setup_try_idx] = (OpCode.SETUP_TRY, len(self.bytecode))
        if stmt.exception_var.lexeme not in self.globals:
            self.globals.append(stmt.exception_var.lexeme)
        idx = self.globals.index(stmt.exception_var.lexeme)
        self.emit(OpCode.STORE_VAR, idx)
        self.emit(OpCode.POP)
        self.visit(stmt.handle_block)
        self.bytecode[exit_jump_idx] = (OpCode.JUMP, len(self.bytecode))

    def visit_Block(self, stmt: Block):
        for s in stmt.statements:
            self.visit(s)

    def visit_Binary(self, expr: Binary):
        self.visit(expr.left)
        self.visit(expr.right)
        op_map = {
            TokenType.PLUS: OpCode.ADD, TokenType.MINUS: OpCode.SUB,
            TokenType.STAR: OpCode.MUL, TokenType.SLASH: OpCode.DIV,
            TokenType.PERCENT: OpCode.PERCENT, TokenType.DOUBLE_EQUALS: OpCode.EQ,
            TokenType.GREATER: OpCode.GT, TokenType.LESS: OpCode.LT,
        }
        self.emit(op_map[expr.operator.type])

    def visit_Literal(self, expr: Literal):
        self.emit(OpCode.PUSH_CONST, expr.value)

    def visit_Variable(self, expr: Variable):
        if expr.name.lexeme == 'z':
            self.emit(OpCode.PUSH_CONST, 'SYSTEM_Z')
            return
        if expr.name.lexeme in self.globals:
            idx = self.globals.index(expr.name.lexeme)
            self.emit(OpCode.LOAD_VAR, idx)
        elif expr.name.lexeme in self.class_map:
            self.emit(OpCode.PUSH_CONST, self.class_map[expr.name.lexeme])
        else:
            self.emit(OpCode.LOAD_VAR, expr.name.lexeme)

    def visit_Assign(self, expr: Assign):
        self.visit(expr.value)
        if expr.name.lexeme in self.globals:
            idx = self.globals.index(expr.name.lexeme)
            self.emit(OpCode.STORE_VAR, idx)
        else:
             self.emit(OpCode.STORE_VAR, expr.name.lexeme)

    def visit_Set(self, expr: Set):
        if isinstance(expr.obj, Variable) and expr.obj.name.lexeme in self.class_map:
            self.visit(expr.obj)
            self.visit(expr.value)
            self.emit(OpCode.SET_STATIC, expr.name.lexeme)
        else:
            self.visit(expr.obj)
            self.visit(expr.value)
            self.emit(OpCode.STORE_FIELD, expr.name.lexeme)

    def visit_New(self, expr: New):
        for arg in expr.arguments:
            self.visit(arg)
        self.emit(OpCode.NEW, expr.name.lexeme)

    def visit_Call(self, expr: Call):
        if isinstance(expr.callee, Get):
            self.visit(expr.callee.obj)
            for arg in expr.arguments:
                self.visit(arg)
            if expr.callee.name.lexeme == 'o':
                self.emit(OpCode.PRINT)
            else:
                self.emit(OpCode.CALL, (expr.callee.name.lexeme, len(expr.arguments)))
        elif isinstance(expr.callee, Variable):
            for arg in expr.arguments:
                self.visit(arg)
            self.emit(OpCode.CALL, (expr.callee.name.lexeme, len(expr.arguments)))

    def visit_Get(self, expr: Get):
        if isinstance(expr.obj, Variable) and expr.obj.name.lexeme in self.class_map:
            self.visit(expr.obj)
            self.emit(OpCode.GET_STATIC, expr.name.lexeme)
        else:
            self.visit(expr.obj)
            self.emit(OpCode.LOAD_FIELD, expr.name.lexeme)

    def visit_ListLiteral(self, expr: ListLiteral):
        for element in expr.elements:
            self.visit(element)
        self.emit(OpCode.BUILD_LIST, len(expr.elements))

    def visit_MapLiteral(self, expr: MapLiteral):
        for i in range(len(expr.keys)):
            self.visit(expr.keys[i])
            self.visit(expr.values[i])
        self.emit(OpCode.BUILD_MAP, len(expr.keys))

    def visit_IndexExpr(self, expr: IndexExpr):
        self.visit(expr.obj)
        self.visit(expr.index)
        self.emit(OpCode.LOAD_INDEX)
