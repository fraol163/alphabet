#!/usr/bin/env python3
import sys
import os

script_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(script_dir, 'src'))

from main import run
from lexer import Lexer
from parser import Parser
from compiler import Compiler
from vm import VM

VERSION = "1.0.0"
DEVELOPER = "Fraol Teshome (fraolteshome444@gmail.com)"

LOGO = r"""
            d8b            d8b                 d8b                     
           88P            ?88                 ?88                d8P  
          d88              88b                 88b            d888888P
 d888b8b  888  ?88,.d88b,  888888b  d888b8b    888888b  d8888b  ?88'  
d8P' ?88  ?88  `?88'  ?88  88P `?8bd8P' ?88    88P `?8bd8b_,dP  88P   
88b  ,88b  88b   88b  d8P d88   88P88b  ,88b  d88,  d8888b      88b   
`?88P'`88b  88b  888888P'd88'   88b`?88P'`88bd88'`?88P'`?888P'  `?8b  
                 88P'                                                 
                d88                                                   
                ?8P
"""

def start_repl():
    print(LOGO)
    print(f"Alphabet Language [{VERSION}]")
    print(f"Developed by {DEVELOPER}")
    print("Type 'q' to exit.")
    compiler = Compiler()
    vm = VM()
    
    while True:
        try:
            line = input(">>> ")
            if line.strip().lower() == 'q': break
            if not line.strip(): continue
            
            lexer = Lexer(line)
            tokens = lexer.scan_tokens()
            parser = Parser(tokens)
            statements = parser.parse()
            
            program = compiler.compile(statements)
            vm.run(program)
            
        except EOFError:
            print()
            break
        except Exception as e:
            print(f"Error: {e}")

def main():
    if len(sys.argv) < 2:
        start_repl()
        sys.exit(0)
    
    arg = sys.argv[1]
    
    if arg in ["-v", "--version"]:
        print(f"Alphabet {VERSION}")
        print(f"Developer: {DEVELOPER}")
        sys.exit(0)

    if not os.path.exists(arg):
        print(f"Error: File '{arg}' not found.")
        sys.exit(1)
        
    with open(arg, 'r') as f:
        source = f.read()
        
    try:
        run(source)
    except Exception as e:
        print(f"Runtime Error: {e}")

if __name__ == "__main__":
    main()
