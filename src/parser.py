from typing import List, Optional
from lexer import Token, TokenType
from alphabet_ast import *

class ParseError(RuntimeError):
    pass

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.current = 0

    def parse(self) -> List[Stmt]:
        statements = []
        while not self.is_at_end():
            stmt = self.declaration()
            if stmt:
                statements.append(stmt)
        return statements

    def declaration(self) -> Optional[Stmt]:
        try:
            if self.check(TokenType.INTERFACE):
                self.advance()
                return self.interface_declaration()
            if self.check(TokenType.CLASS) and self.check_next_is_identifier():
                self.advance()
                return self.class_declaration()
            return self.statement()
        except ParseError:
            self.synchronize()
            return None

    def interface_declaration(self) -> Stmt:
        name = self.consume_identifier("Expect interface name.")
        self.consume(TokenType.LBRACE, "Expect '{' before interface body.")
        methods = []
        while not self.check(TokenType.RBRACE) and not self.is_at_end():
            if self.match(TokenType.METHOD):
                return_type = self.consume(TokenType.NUMBER, "Expect return type ID.")
                method_name = self.consume_identifier("Expect method name.")
                self.consume(TokenType.LPAREN, "Expect '(' after method name.")
                parameters = []
                if not self.check(TokenType.RPAREN):
                    while True:
                        type_id = self.consume(TokenType.NUMBER, "Expect parameter type ID.")
                        param_name = self.consume_identifier("Expect parameter name.")
                        parameters.append(VarStmt(type_id, param_name, None, None))
                        if not self.match(TokenType.COMMA): break
                self.consume(TokenType.RPAREN, "Expect ')' after parameters.")
                methods.append(FunctionStmt(method_name, parameters, [], return_type, None))
            else:
                raise self.error(self.peek(), "Interfaces can only contain methods.")
        self.consume(TokenType.RBRACE, "Expect '}' after interface body.")
        return ClassStmt(name, None, methods, [], [], is_interface=True)

    def class_declaration(self) -> Stmt:
        name = self.consume_identifier("Expect class name.")
        superclass = None
        interfaces = []
        if self.match(TokenType.EXTENDS):
            superclass_name = self.consume_identifier("Expect superclass or interface name.")
            superclass = Variable(superclass_name)
            while self.match(TokenType.COMMA):
                if_name = self.consume_identifier("Expect interface name.")
                interfaces.append(Variable(if_name))
        self.consume(TokenType.LBRACE, "Expect '{' before class body.")
        methods = []
        fields = []
        while not self.check(TokenType.RBRACE) and not self.is_at_end():
            visibility = None
            is_static = False
            while True:
                if self.check(TokenType.PUBLIC) or self.check(TokenType.PRIVATE):
                    if visibility: break
                    visibility = self.advance()
                elif self.match(TokenType.STATIC):
                    if is_static: break
                    is_static = True
                else:
                    break
            if self.match(TokenType.METHOD):
                methods.append(self.method(visibility, is_static))
            elif self.check(TokenType.NUMBER):
                fields.append(self.var_declaration(visibility, is_static))
            else:
                raise self.error(self.peek(), "Expect method or field declaration.")
        self.consume(TokenType.RBRACE, "Expect '}' after class body.")
        return ClassStmt(name, superclass, methods, fields, interfaces)

    def method(self, visibility: Optional[Token], is_static: bool = False) -> FunctionStmt:
        return_type = self.consume(TokenType.NUMBER, "Expect return type ID.")
        name = self.consume_identifier("Expect method name.")
        self.consume(TokenType.LPAREN, "Expect '(' after method name.")
        parameters = []
        if not self.check(TokenType.RPAREN):
            while True:
                type_id = self.consume(TokenType.NUMBER, "Expect parameter type ID.")
                param_name = self.consume_identifier("Expect parameter name.")
                parameters.append(VarStmt(type_id, param_name, None, None))
                if not self.match(TokenType.COMMA): break
        self.consume(TokenType.RPAREN, "Expect ')' after parameters.")
        self.consume(TokenType.LBRACE, "Expect '{' before method body.")
        body = self.block()
        return FunctionStmt(name, parameters, body, return_type, visibility, is_static)

    def var_declaration(self, visibility: Optional[Token] = None, is_static: bool = False) -> Stmt:
        type_id = self.consume(TokenType.NUMBER, "Expect type ID.")
        name = self.consume_identifier("Expect variable name.")
        initializer = None
        if self.match(TokenType.EQUALS):
            initializer = self.expression()
        return VarStmt(type_id, name, initializer, visibility, is_static)

    def statement(self) -> Stmt:
        if self.match(TokenType.IF): return self.if_statement()
        if self.match(TokenType.RETURN): return self.return_statement()
        if self.match(TokenType.LOOP): return self.loop_statement()
        if self.match(TokenType.TRY): return self.try_statement()
        if self.match(TokenType.LBRACE): return Block(self.block())
        if self.check(TokenType.NUMBER): return self.var_declaration()
        return self.expression_statement()

    def if_statement(self) -> Stmt:
        self.consume(TokenType.LPAREN, "Expect '(' after 'i'.")
        condition = self.expression()
        self.consume(TokenType.RPAREN, "Expect ')' after if condition.")
        then_branch = self.statement()
        else_branch = None
        if self.match(TokenType.ELSE):
            else_branch = self.statement()
        return IfStmt(condition, then_branch, else_branch)

    def loop_statement(self) -> Stmt:
        self.consume(TokenType.LPAREN, "Expect '(' after 'l'.")
        condition = self.expression()
        self.consume(TokenType.RPAREN, "Expect ')' after loop condition.")
        body = self.statement()
        return LoopStmt(condition, body)

    def try_statement(self) -> Stmt:
        self.consume(TokenType.LBRACE, "Expect '{' before try block.")
        try_block = Block(self.block())
        self.consume(TokenType.HANDLE, "Expect 'h' after try block.")
        self.consume(TokenType.LPAREN, "Expect '(' after 'h'.")
        exception_type = self.consume(TokenType.NUMBER, "Expect exception type ID.")
        exception_var = self.consume_identifier("Expect exception variable name.")
        self.consume(TokenType.RPAREN, "Expect ')' after exception catch details.")
        self.consume(TokenType.LBRACE, "Expect '{' before handle block.")
        handle_block = Block(self.block())
        return TryStmt(try_block, exception_type, exception_var, handle_block)

    def return_statement(self) -> Stmt:
        keyword = self.previous()
        value = None
        if not self.check(TokenType.RBRACE):
             try:
                 value = self.expression()
             except:
                 value = None
        return ReturnStmt(keyword, value)

    def block(self) -> List[Stmt]:
        statements = []
        while not self.check(TokenType.RBRACE) and not self.is_at_end():
            statements.append(self.declaration())
        self.consume(TokenType.RBRACE, "Expect '}' after block.")
        return statements

    def expression_statement(self) -> Stmt:
        expr = self.expression()
        return ExpressionStmt(expr)

    def expression(self) -> Expr:
        return self.assignment()

    def assignment(self) -> Expr:
        expr = self.or_expr()
        if self.match(TokenType.EQUALS):
            equals = self.previous()
            value = self.assignment()
            if isinstance(expr, Variable):
                name = expr.name
                return Assign(name, value)
            elif isinstance(expr, Get):
                return Set(expr.obj, expr.name, value)
            self.error(equals, "Invalid assignment target.")
        return expr

    def or_expr(self) -> Expr:
        expr = self.and_expr()
        while self.match(TokenType.OR):
            operator = self.previous()
            right = self.and_expr()
            expr = Logical(expr, operator, right)
        return expr

    def and_expr(self) -> Expr:
        expr = self.equality()
        while self.match(TokenType.AND):
            operator = self.previous()
            right = self.equality()
            expr = Logical(expr, operator, right)
        return expr

    def equality(self) -> Expr:
        expr = self.comparison()
        while self.match(TokenType.DOUBLE_EQUALS, TokenType.NOT_EQUALS):
            operator = self.previous()
            right = self.comparison()
            expr = Binary(expr, operator, right)
        return expr

    def comparison(self) -> Expr:
        expr = self.term()
        while self.match(TokenType.GREATER, TokenType.GREATER_EQUALS, TokenType.LESS, TokenType.LESS_EQUALS):
            operator = self.previous()
            right = self.term()
            expr = Binary(expr, operator, right)
        return expr

    def term(self) -> Expr:
        expr = self.factor()
        while self.match(TokenType.MINUS, TokenType.PLUS):
            operator = self.previous()
            right = self.factor()
            expr = Binary(expr, operator, right)
        return expr

    def factor(self) -> Expr:
        expr = self.unary()
        while self.match(TokenType.SLASH, TokenType.STAR, TokenType.PERCENT):
            operator = self.previous()
            right = self.unary()
            expr = Binary(expr, operator, right)
        return expr

    def unary(self) -> Expr:
        if self.match(TokenType.NOT, TokenType.MINUS, TokenType.AT):
            operator = self.previous()
            right = self.unary()
            return Unary(operator, right)
        return self.call()

    def call(self) -> Expr:
        expr = self.primary()
        while True:
            if self.match(TokenType.LPAREN):
                expr = self.finish_call(expr)
            elif self.match(TokenType.DOT):
                name = self.consume_identifier("Expect property name after '.'.")
                expr = Get(expr, name)
            elif self.match(TokenType.LBRACKET):
                index = self.expression()
                self.consume(TokenType.RBRACKET, "Expect ']' after index.")
                expr = IndexExpr(expr, index)
            else:
                break
        return expr

    def finish_call(self, callee: Expr) -> Expr:
        arguments = []
        if not self.check(TokenType.RPAREN):
            while True:
                arguments.append(self.expression())
                if not self.match(TokenType.COMMA): break
        paren = self.consume(TokenType.RPAREN, "Expect ')' after arguments.")
        return Call(callee, paren, arguments)

    def primary(self) -> Expr:
        if self.match(TokenType.NUMBER, TokenType.STRING):
            return Literal(self.previous().literal)
        if self.match(TokenType.SYSTEM):
            return Variable(self.previous())
        if self.match(TokenType.NEW):
            name = self.consume_identifier("Expect class name after 'n'.")
            arguments = []
            if self.match(TokenType.LPAREN):
                if not self.check(TokenType.RPAREN):
                    while True:
                        arguments.append(self.expression())
                        if not self.match(TokenType.COMMA): break
                self.consume(TokenType.RPAREN, "Expect ')' after arguments.")
            return New(name, arguments)
        if self.is_identifier():
            return Variable(self.advance())
        if self.match(TokenType.LBRACKET):
            elements = []
            if not self.check(TokenType.RBRACKET):
                while True:
                    elements.append(self.expression())
                    if not self.match(TokenType.COMMA): break
            self.consume(TokenType.RBRACKET, "Expect ']' after list elements.")
            return ListLiteral(elements)
        if self.match(TokenType.LBRACE):
            keys = []
            values = []
            if not self.check(TokenType.RBRACE):
                while True:
                    keys.append(self.expression())
                    self.consume(TokenType.COLON, "Expect ':' after map key.")
                    values.append(self.expression())
                    if not self.match(TokenType.COMMA): break
            self.consume(TokenType.RBRACE, "Expect '}' after map elements.")
            return MapLiteral(keys, values)
        if self.match(TokenType.LPAREN):
            expr = self.expression()
            self.consume(TokenType.RPAREN, "Expect ')' after expression.")
            return Grouping(expr)
        raise self.error(self.peek(), "Expect expression.")

    def is_identifier(self) -> bool:
        if self.is_at_end(): return False
        t = self.peek().type
        return t == TokenType.IDENTIFIER or (len(self.peek().lexeme) == 1 and self.peek().lexeme.isalpha())

    def consume_identifier(self, message: str) -> Token:
        if self.is_identifier(): return self.advance()
        raise self.error(self.peek(), message)

    def match(self, *types: TokenType) -> bool:
        for type in types:
            if self.check(type):
                self.advance()
                return True
        return False

    def check(self, type: TokenType) -> bool:
        if self.is_at_end(): return False
        return self.peek().type == type

    def check_next_is_identifier(self) -> bool:
        if self.current + 1 >= len(self.tokens): return False
        t = self.tokens[self.current + 1].type
        if t == TokenType.IDENTIFIER: return True
        lexeme = self.tokens[self.current + 1].lexeme
        return len(lexeme) == 1 and lexeme.isalpha()

    def advance(self) -> Token:
        if not self.is_at_end(): self.current += 1
        return self.previous()

    def is_at_end(self) -> bool:
        return self.peek().type == TokenType.EOF

    def peek(self) -> Token:
        return self.tokens[self.current]

    def previous(self) -> Token:
        return self.tokens[self.current - 1]

    def consume(self, type: TokenType, message: str) -> Token:
        if self.check(type): return self.advance()
        raise self.error(self.peek(), message)

    def error(self, token: Token, message: str) -> ParseError:
        return ParseError()

    def synchronize(self):
        self.advance()
        while not self.is_at_end():
            if self.previous().type in [TokenType.CLASS, TokenType.METHOD, TokenType.IF, TokenType.LOOP, TokenType.RETURN]:
                return
            self.advance()
