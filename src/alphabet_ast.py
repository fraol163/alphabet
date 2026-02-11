from dataclasses import dataclass
from typing import List, Optional, Any
from lexer import Token

@dataclass
class Expr:
    pass

@dataclass
class Stmt:
    pass

@dataclass
class Binary(Expr):
    left: Expr
    operator: Token
    right: Expr

@dataclass
class Unary(Expr):
    operator: Token
    right: Expr

@dataclass
class Literal(Expr):
    value: Any

@dataclass
class Grouping(Expr):
    expression: Expr

@dataclass
class Variable(Expr):
    name: Token

@dataclass
class Assign(Expr):
    name: Token
    value: Expr

@dataclass
class Logical(Expr):
    left: Expr
    operator: Token
    right: Expr

@dataclass
class Call(Expr):
    callee: Expr
    paren: Token
    arguments: List[Expr]

@dataclass
class Get(Expr):
    obj: Expr
    name: Token

@dataclass
class Set(Expr):
    obj: Expr
    name: Token
    value: Expr

@dataclass
class New(Expr):
    name: Token
    arguments: List[Expr]

@dataclass
class ListLiteral(Expr):
    elements: List[Expr]

@dataclass
class MapLiteral(Expr):
    keys: List[Expr]
    values: List[Expr]

@dataclass
class IndexExpr(Expr):
    obj: Expr
    index: Expr

@dataclass
class ExpressionStmt(Stmt):
    expression: Expr

@dataclass
class VarStmt(Stmt):
    type_id: Token # The numeric type (1-50)
    name: Token
    initializer: Optional[Expr]
    visibility: Optional[Token] # 'v' or 'p'
    is_static: bool = False

@dataclass
class Block(Stmt):
    statements: List[Stmt]

@dataclass
class IfStmt(Stmt):
    condition: Expr
    then_branch: Stmt
    else_branch: Optional[Stmt]

@dataclass
class LoopStmt(Stmt):
    condition: Expr
    body: Stmt

@dataclass
class TryStmt(Stmt):
    try_block: Block
    exception_type: Token # Numeric type ID
    exception_var: Token # Variable name to store exception
    handle_block: Block

@dataclass
class ReturnStmt(Stmt):
    keyword: Token
    value: Optional[Expr]

@dataclass
class FunctionStmt(Stmt):
    name: Token
    params: List[VarStmt]
    body: List[Stmt]
    return_type: Token
    visibility: Optional[Token]
    is_static: bool = False

@dataclass
class ClassStmt(Stmt):
    name: Token
    superclass: Optional[Variable]
    methods: List[FunctionStmt]
    fields: List[VarStmt]
    interfaces: List[Variable]
    is_interface: bool = False
