# Alphabet Language — BNF Grammar

> Formal grammar for the Alphabet programming language (v2.3.5).
> Compiled pipeline: Lexer → Parser → AST → Compiler → Bytecode → VM

---

## Notation

- `::=` defines a production rule
- `|` separates alternatives
- `[ ]` denotes optional
- `{ }` denotes zero or more
- Terminals are in `"quotes"` or UPPER_CASE
- Non-terminals are in *italics*

---

## 1. Program Structure

```
program        ::= header declaration*
header         ::= "#alphabet" "<" LANG_CODE ">"
LANG_CODE      ::= "en" | "am" | "es" | "fr" | "de"
```

## 2. Declarations

```
declaration    ::= import_decl
                 | export_decl
                 | var_decl
                 | func_decl
                 | class_decl
                 | interface_decl
                 | abstract_decl
                 | statement

import_decl    ::= IMPORT_KW STRING
                 | IMPORT_KW STRING AS_KW IDENT

export_decl    ::= "@" export_list
export_list    ::= IDENT ( "," IDENT )*

class_decl     ::= CLASS_KW IDENT [ ":" IDENT ] "{" class_body "}"
class_body     ::= ( access_mod? member_decl )*

abstract_decl  ::= ABSTRACT_KW IDENT [ ":" IDENT ] "{" class_body "}"

interface_decl ::= INTERFACE_KW IDENT "{" interface_body "}"
interface_body ::= func_sig ";"

access_mod     ::= PUBLIC_KW | PRIVATE_KW
```

## 3. Class Members

```
member_decl    ::= var_decl
                 | func_decl
                 | constructor_decl
                 | static_decl

constructor_decl ::= METHOD_KW IDENT "(" param_list? ")" block

static_decl    ::= STATIC_KW ( var_decl | func_decl )

func_decl      ::= METHOD_KW type_expr IDENT "(" param_list? ")" block
func_sig       ::= METHOD_KW type_expr IDENT "(" param_list? ")"

param_list     ::= parameter ( "," parameter )*
parameter      ::= type_expr IDENT

type_expr      ::= base_type
                 | IDENT
                 | type_expr "[" "]"

base_type      ::= "integer" | "float" | "string" | "boolean" | "void"
                 | "null" | "any"
```

## 4. Statements

```
statement      ::= var_decl
                 | assignment
                 | expr_stmt
                 | if_stmt
                 | loop_stmt
                 | return_stmt
                 | break_stmt
                 | continue_stmt
                 | try_stmt
                 | match_stmt
                 | block

block          ::= "{" statement* "}"

var_decl       ::= type_expr IDENT "=" expression
                 | LET_KW IDENT "=" expression
                 | type_expr IDENT

assignment     ::= IDENT "=" expression
                 | expr "." IDENT "=" expression
                 | expr "[" expression "]" "=" expression
                 | destructuring "=" expression

destructuring  ::= "[" IDENT ( "," IDENT )* "]"

expr_stmt      ::= expression

if_stmt        ::= IF_KW "(" expression ")" block [ ELSE_KW ( if_stmt | block ) ]

loop_stmt      ::= for_loop | foreach_loop | while_loop

for_loop       ::= LOOP_KW "(" var_decl ":" expression ":" expression ")" block
foreach_loop   ::= LOOP_KW "(" IDENT ":" expression ")" block
while_loop     ::= LOOP_KW "(" expression ")" block

return_stmt    ::= RETURN_KW [ expression ]

break_stmt     ::= BREAK_KW

continue_stmt  ::= CONTINUE_KW

try_stmt       ::= TRY_KW block HANDLE_KW "(" IDENT ")" block

match_stmt     ::= MATCH_KW "(" expression ")" "{" match_case* [ default_case ] "}"
match_case     ::= expression ":" ( block | statement )
default_case   ::= "_" ":" ( block | statement )
```

## 5. Expressions

```
expression     ::= ternary_expr

ternary_expr   ::= null_coalesce [ "?" expression ":" expression ]

null_coalesce  ::= or_expr [ "??" or_expr ]

or_expr        ::= and_expr { OR_KW and_expr }
and_expr       ::= equality { AND_KW equality }

equality       ::= comparison { ( "==" | "!=" ) comparison }
comparison     ::= addition { ( "<" | ">" | "<=" | ">=" ) addition }
addition       ::= multiply { ( "+" | "-" ) multiply }
multiply       ::= unary { ( "*" | "/" | "%" ) unary }

unary          ::= ( "-" | "!" | NOT_KW ) unary
                 | postfix

postfix        ::= primary { call_suffix | index_suffix | field_suffix | null_safe_suffix }

call_suffix    ::= "(" arg_list? ")"
index_suffix   ::= "[" expression "]"
field_suffix   ::= "." IDENT
null_safe_suffix ::= "?." IDENT

arg_list       ::= expression ( "," expression )*

primary        ::= INTEGER
                 | FLOAT
                 | STRING
                 | BOOLEAN
                 | "null"
                 | IDENT
                 | "(" expression ")"
                 | list_literal
                 | map_literal
                 | range_expr
                 | lambda_expr
                 | new_expr
                 | system_call

list_literal   ::= "[" [ expression ( "," expression )* [ "," ] ] "]"
map_literal    ::= "{" [ map_entry ( "," map_entry )* [ "," ] ] "}"
map_entry      ::= expression ":" expression

range_expr     ::= expression ".." expression

lambda_expr    ::= METHOD_KW "(" param_list? ")" block

new_expr       ::= NEW_KW IDENT "(" arg_list? ")"

system_call    ::= SYSTEM_KW "." IDENT "(" arg_list? ")"
```

## 6. Keywords

| English | Keyword | Description |
|---------|---------|-------------|
| if | `i` | Conditional branch |
| else | `e` | Else branch |
| loop | `l` | Loop construct |
| break | `b` | Break out of loop |
| continue | `k` | Skip to next iteration |
| return | `r` | Return from function |
| class | `c` | Class declaration |
| abstract | `a` | Abstract class |
| interface | `j` | Interface declaration |
| new | `n` | Object instantiation |
| public | `v` | Public access modifier |
| private | `p` | Private access modifier |
| static | `s` | Static member |
| method | `m` | Function/method declaration |
| try | `t` | Try block |
| handle | `h` | Exception handler |
| system | `z` | System built-in namespace |
| import | `x` | Import statement |
| match | `q` | Pattern matching |

## 7. Literals

```
INTEGER        ::= DIGIT+ [ "_" DIGIT+ ]*
FLOAT          ::= DIGIT+ "." DIGIT+ [ [eE] [+-]? DIGIT+ ]?
STRING         ::= '"' ( CHAR | ESCAPE )* '"'
                 | "'" ( CHAR | ESCAPE )* "'"
BOOLEAN        ::= "true" | "false"

DIGIT          ::= [0-9]
CHAR           ::= any Unicode character except unescaped " or \
ESCAPE         ::= "\" [nrtb"'\]
IDENT          ::= ( LETTER | "_" ) ( LETTER | DIGIT | "_" )*
LETTER         ::= [a-zA-Z]
```

## 8. Comments

```
comment        ::= single_line | multi_line
single_line    ::= "//" ( any character until end of line )
multi_line     ::= "/*" ( any character until "*/" ) "*/"
```

## 9. Operator Precedence (highest to lowest)

| Precedence | Operators | Associativity |
|-----------|-----------|---------------|
| 1 (highest) | `()` `[]` `.` `?.` | Left |
| 2 | `-` `!` (unary) | Right |
| 3 | `*` `/` `%` | Left |
| 4 | `+` `-` | Left |
| 5 | `<` `>` `<=` `>=` | Left |
| 6 | `==` `!=` | Left |
| 7 | `..` | Left |
| 8 | `&&` | Left |
| 9 | `\|\|` | Left |
| 10 | `??` | Left |
| 11 (lowest) | `? :` (ternary) | Right |

## 10. Built-in Functions (shorthand)

These are available without the `z.` prefix:

```
o(...)         — print to stdout (alias for z.o)
len(...)       — collection length (alias for z.len)
sqrt(...)      — square root (alias for z.sqrt)
input(...)     — read input (alias for z.input)
type(...)      — type name string (alias for z.type)
str(...)       — convert to string (alias for z.str)
int(...)       — convert to integer (alias for z.int)
float(...)     — convert to float (alias for z.float)
```

## 11. Bytecode Opcodes

| Opcode | Value | Description |
|--------|-------|-------------|
| HALT | 0 | Stop execution |
| PUSH_CONST | 1 | Push constant to stack |
| ADD | 2 | Add top two stack values |
| SUB | 3 | Subtract |
| MUL | 4 | Multiply |
| DIV | 5 | Divide |
| MOD | 6 | Modulo |
| NEG | 7 | Negate |
| NOT | 8 | Logical NOT |
| EQ | 9 | Equal |
| NEQ | 10 | Not equal |
| LT | 11 | Less than |
| GT | 12 | Greater than |
| LE | 13 | Less or equal |
| GE | 14 | Greater or equal |
| AND | 15 | Logical AND |
| OR | 16 | Logical OR |
| JUMP | 17 | Unconditional jump |
| JUMP_IF_FALSE | 18 | Jump if false |
| STORE_VAR | 19 | Store to variable |
| LOAD_VAR | 20 | Load variable |
| POP | 21 | Pop stack top |
| CALL | 22 | Call function |
| RET | 23 | Return from function |
| LOAD_FIELD | 24 | Load object field |
| STORE_FIELD | 25 | Store to object field |
| NEW | 26 | Create new instance |
| DUP | 27 | Duplicate stack top |
| SWAP | 28 | Swap top two |
| PRINT | 29 | Print value |
| INDEX | 30 | Index into list/map |
| STORE_INDEX | 31 | Store to index |
| LIST | 32 | Create list |
| MAP | 33 | Create map |
| PUSH_NULL | 34 | Push null |
| PUSH_TRUE | 35 | Push true |
| PUSH_FALSE | 36 | Push false |
| IS_NULL | 37 | Check if null |
| COALESCE | 38 | Null coalescing |
| BREAK_JUMP | 39 | Break from loop |
| CONTINUE_JUMP | 40 | Continue loop |
| JUMP_IF_TRUE | 41 | Jump if true |
| NOP | 51 | No operation |
| PUSH_CONST_POOL | 52 | Push from constant pool |
