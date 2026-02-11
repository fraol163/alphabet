import enum

class OpCode(enum.IntEnum):
    PUSH_CONST = 1
    LOAD_VAR = 2
    STORE_VAR = 3
    LOAD_FIELD = 4
    STORE_FIELD = 5
    ADD = 6
    SUB = 7
    MUL = 8
    DIV = 9
    PERCENT = 10
    EQ = 11
    NE = 12
    GT = 13
    GE = 14
    LT = 15
    LE = 16
    AND = 17
    OR = 18
    NOT = 19
    JUMP = 20
    JUMP_IF_FALSE = 21
    CALL = 22
    RET = 23
    NEW = 24
    POP = 25
    PRINT = 26
    HALT = 27
    SETUP_TRY = 28
    POP_TRY = 29
    THROW = 30
    GET_STATIC = 31
    SET_STATIC = 32
    BUILD_LIST = 33
    BUILD_MAP = 34
    LOAD_INDEX = 35
    STORE_INDEX = 36

# Instructions are typically (OpCode, operand)
