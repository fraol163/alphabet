#ifndef ALPHABET_BYTECODE_H
#define ALPHABET_BYTECODE_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace alphabet {

enum class OpCode : uint8_t {
    PUSH_CONST = 1,
    LOAD_VAR = 2,
    STORE_VAR = 3,
    LOAD_FIELD = 4,
    STORE_FIELD = 5,
    ADD = 6,
    SUB = 7,
    MUL = 8,
    DIV = 9,
    PERCENT = 10,
    EQ = 11,
    NE = 12,
    GT = 13,
    GE = 14,
    LT = 15,
    LE = 16,
    AND = 17,
    OR = 18,
    NOT = 19,
    JUMP = 20,
    JUMP_IF_FALSE = 21,
    CALL = 22,
    RET = 23,
    NEW = 24,
    POP = 25,
    PRINT = 26,
    HALT = 27,
    SETUP_TRY = 28,
    POP_TRY = 29,
    THROW = 30,
    GET_STATIC = 31,
    SET_STATIC = 32,
    BUILD_LIST = 33,
    BUILD_MAP = 34,
    LOAD_INDEX = 35,
    STORE_INDEX = 36
};

using Operand = std::variant<std::monostate, int64_t, double, std::string,
                              std::nullptr_t, std::pair<std::string, int>>;

struct Instruction {
    OpCode op;
    Operand operand;

    Instruction() : op(OpCode::HALT), operand(std::monostate{}) {}
    Instruction(OpCode opcode) : op(opcode), operand(std::monostate{}) {}
    Instruction(OpCode opcode, Operand opnd) : op(opcode), operand(std::move(opnd)) {}
};

struct CompiledMethod {
    std::vector<Instruction> bytecode;
    std::vector<std::string> param_names;
};

struct CompiledClass {
    std::string name;
    std::string superclass;
    uint16_t id;
    std::unordered_map<std::string, CompiledMethod> methods;
    std::unordered_map<std::string, CompiledMethod> static_methods;
    std::vector<Instruction> static_init;
};

struct Program {
    std::vector<Instruction> main;
    std::vector<Instruction> static_init;
    std::unordered_map<uint16_t, CompiledClass> classes;
    std::vector<std::string> globals;
};

inline const char* opcode_to_string(OpCode op) {
    switch (op) {
        case OpCode::PUSH_CONST: return "PUSH_CONST";
        case OpCode::LOAD_VAR: return "LOAD_VAR";
        case OpCode::STORE_VAR: return "STORE_VAR";
        case OpCode::LOAD_FIELD: return "LOAD_FIELD";
        case OpCode::STORE_FIELD: return "STORE_FIELD";
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::PERCENT: return "PERCENT";
        case OpCode::EQ: return "EQ";
        case OpCode::NE: return "NE";
        case OpCode::GT: return "GT";
        case OpCode::GE: return "GE";
        case OpCode::LT: return "LT";
        case OpCode::LE: return "LE";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::CALL: return "CALL";
        case OpCode::RET: return "RET";
        case OpCode::NEW: return "NEW";
        case OpCode::POP: return "POP";
        case OpCode::PRINT: return "PRINT";
        case OpCode::HALT: return "HALT";
        case OpCode::SETUP_TRY: return "SETUP_TRY";
        case OpCode::POP_TRY: return "POP_TRY";
        case OpCode::THROW: return "THROW";
        case OpCode::GET_STATIC: return "GET_STATIC";
        case OpCode::SET_STATIC: return "SET_STATIC";
        case OpCode::BUILD_LIST: return "BUILD_LIST";
        case OpCode::BUILD_MAP: return "BUILD_MAP";
        case OpCode::LOAD_INDEX: return "LOAD_INDEX";
        case OpCode::STORE_INDEX: return "STORE_INDEX";
        default: return "UNKNOWN";
    }
}

}

#endif
