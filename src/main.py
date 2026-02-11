from lexer import Lexer
from parser import Parser
from compiler import Compiler
from vm import VM

def run(source: str):
    lexer = Lexer(source)
    tokens = lexer.scan_tokens()
    parser = Parser(tokens)
    statements = parser.parse()
    compiler = Compiler()
    program = compiler.compile(statements)
    vm = VM(program)
    vm.run()

if __name__ == "__main__":
    pass
