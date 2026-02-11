import enum
from dataclasses import dataclass
from typing import List, Optional

class TokenType(enum.Enum):
    IF = "i"
    ELSE = "e"
    LOOP = "l"
    BREAK = "b"
    CONTINUE = "k"
    RETURN = "r"
    CLASS = "c"
    ABSTRACT = "a"
    INTERFACE = "j"
    NEW = "n"
    EXTENDS = "^"
    PUBLIC = "v"
    PRIVATE = "p"
    STATIC = "s"
    METHOD = "m"
    TRY = "t"
    HANDLE = "h"
    SYSTEM = "z"
    IDENTIFIER = "IDENTIFIER"
    NUMBER = "NUMBER"
    STRING = "STRING"
    PLUS = "+"
    MINUS = "-"
    STAR = "*"
    SLASH = "/"
    PERCENT = "%"
    EQUALS = "="
    DOUBLE_EQUALS = "=="
    NOT_EQUALS = "!="
    GREATER = ">"
    LESS = "<"
    GREATER_EQUALS = ">="
    LESS_EQUALS = "<="
    AND = "&&"
    OR = "||"
    NOT = "!"
    DOT = "."
    AT = "@" 
    LBRACE = "{"
    RBRACE = "}"
    LPAREN = "("
    RPAREN = ")"
    LBRACKET = "["
    RBRACKET = "]"
    COMMA = ","
    COLON = ":"
    EOF = "EOF"

@dataclass
class Token:
    type: TokenType
    lexeme: str
    literal: any
    line: int

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.tokens: List[Token] = []
        self.start = 0
        self.current = 0
        self.line = 1
        self.keywords = {k.value: k for k in TokenType if len(k.value) == 1 and k.value.isalpha()}

    def scan_tokens(self) -> List[Token]:
        if self.source.startswith("#!"):
            while self.peek() != '\n' and not self.is_at_end():
                self.advance()
        while not self.is_at_end():
            self.start = self.current
            self.scan_token()
        self.tokens.append(Token(TokenType.EOF, "", None, self.line))
        return self.tokens

    def is_at_end(self) -> bool:
        return self.current >= len(self.source)

    def scan_token(self):
        c = self.advance()
        if c == '(': self.add_token(TokenType.LPAREN)
        elif c == ')': self.add_token(TokenType.RPAREN)
        elif c == '{': self.add_token(TokenType.LBRACE)
        elif c == '}': self.add_token(TokenType.RBRACE)
        elif c == '[': self.add_token(TokenType.LBRACKET)
        elif c == ']': self.add_token(TokenType.RBRACKET)
        elif c == ',': self.add_token(TokenType.COMMA)
        elif c == ':': self.add_token(TokenType.COLON)
        elif c == '.': self.add_token(TokenType.DOT)
        elif c == '-': self.add_token(TokenType.MINUS)
        elif c == '+': self.add_token(TokenType.PLUS)
        elif c == '*': self.add_token(TokenType.STAR)
        elif c == '%': self.add_token(TokenType.PERCENT)
        elif c == '^': self.add_token(TokenType.EXTENDS)
        elif c == '@': self.add_token(TokenType.AT)
        elif c == '!':
            self.add_token(TokenType.NOT_EQUALS if self.match('=') else TokenType.NOT)
        elif c == '=':
            self.add_token(TokenType.DOUBLE_EQUALS if self.match('=') else TokenType.EQUALS)
        elif c == '<':
            self.add_token(TokenType.LESS_EQUALS if self.match('=') else TokenType.LESS)
        elif c == '>':
            self.add_token(TokenType.GREATER_EQUALS if self.match('=') else TokenType.GREATER)
        elif c == '/':
            if self.match('/'):
                while self.peek() != '\n' and not self.is_at_end():
                    self.advance()
            else:
                self.add_token(TokenType.SLASH)
        elif c == '&':
            if self.match('&'): self.add_token(TokenType.AND)
        elif c == '|':
            if self.match('|'): self.add_token(TokenType.OR)
        elif c in [' ', '\r', '\t']:
            pass
        elif c == '\n':
            self.line += 1
        elif c == '"':
            self.string()
        else:
            if c.isdigit():
                self.number()
            elif c.isalpha():
                self.identifier()
            else:
                print(f"Error: Unexpected character '{c}' at line {self.line}")

    def identifier(self):
        while self.peek().isalnum():
            self.advance()
        text = self.source[self.start:self.current]
        type = self.keywords.get(text, TokenType.IDENTIFIER)
        self.add_token(type)

    def number(self):
        while self.peek().isdigit():
            self.advance()
        if self.peek() == '.' and self.peek_next().isdigit():
            self.advance()
            while self.peek().isdigit():
                self.advance()
        val = float(self.source[self.start:self.current])
        self.add_token(TokenType.NUMBER, val)

    def string(self):
        while self.peek() != '"' and not self.is_at_end():
            if self.peek() == '\n': self.line += 1
            self.advance()
        if self.is_at_end():
            return
        self.advance()
        value = self.source[self.start + 1:self.current - 1]
        self.add_token(TokenType.STRING, value)

    def advance(self) -> str:
        self.current += 1
        return self.source[self.current - 1]

    def add_token(self, type: TokenType, literal: any = None):
        text = self.source[self.start:self.current]
        self.tokens.append(Token(type, text, literal, self.line))

    def match(self, expected: str) -> bool:
        if self.is_at_end(): return False
        if self.source[self.current] != expected: return False
        self.current += 1
        return True

    def peek(self) -> str:
        if self.is_at_end(): return '\0'
        return self.source[self.current]

    def peek_next(self) -> str:
        if self.current + 1 >= len(self.source): return '\0'
        return self.source[self.current + 1]
