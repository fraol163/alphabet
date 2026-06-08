#include "bytecode.h"
#include <cstring>
#include <fstream>
#include <sstream>

namespace alphabet {

static void write_u8(std::ostream& os, uint8_t v) {
    os.write(reinterpret_cast<const char*>(&v), 1);
}
static void write_u16(std::ostream& os, uint16_t v) {
    os.write(reinterpret_cast<const char*>(&v), 2);
}
static void write_u32(std::ostream& os, uint32_t v) {
    os.write(reinterpret_cast<const char*>(&v), 4);
}
static void write_u64(std::ostream& os, uint64_t v) {
    os.write(reinterpret_cast<const char*>(&v), 8);
}
static void write_i64(std::ostream& os, int64_t v) {
    os.write(reinterpret_cast<const char*>(&v), 8);
}
static void write_f64(std::ostream& os, double v) {
    os.write(reinterpret_cast<const char*>(&v), 8);
}

static uint8_t read_u8(std::istream& is) {
    uint8_t v;
    is.read(reinterpret_cast<char*>(&v), 1);
    return v;
}
static uint16_t read_u16(std::istream& is) {
    uint16_t v;
    is.read(reinterpret_cast<char*>(&v), 2);
    return v;
}
static uint32_t read_u32(std::istream& is) {
    uint32_t v;
    is.read(reinterpret_cast<char*>(&v), 4);
    return v;
}
static int64_t read_i64(std::istream& is) {
    int64_t v;
    is.read(reinterpret_cast<char*>(&v), 8);
    return v;
}
static double read_f64(std::istream& is) {
    double v;
    is.read(reinterpret_cast<char*>(&v), 8);
    return v;
}

static void write_string(std::ostream& os, const std::string& s) {
    write_u32(os, static_cast<uint32_t>(s.size()));
    os.write(s.data(), s.size());
}

static std::string read_string(std::istream& is) {
    uint32_t len = read_u32(is);
    std::string s(len, '\0');
    is.read(&s[0], len);
    return s;
}

static void write_operand(std::ostream& os, const Operand& op) {
    std::visit(
        [&os](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                write_u8(os, 0);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                write_u8(os, 1);
                write_i64(os, v);
            } else if constexpr (std::is_same_v<T, double>) {
                write_u8(os, 2);
                write_f64(os, v);
            } else if constexpr (std::is_same_v<T, std::string>) {
                write_u8(os, 3);
                write_string(os, v);
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                write_u8(os, 4);
            } else if constexpr (std::is_same_v<T, std::pair<std::string, int>>) {
                write_u8(os, 5);
                write_string(os, v.first);
                write_u32(os, static_cast<uint32_t>(v.second));
            }
        },
        op);
}

static Operand read_operand(std::istream& is) {
    uint8_t tag = read_u8(is);
    switch (tag) {
    case 0:
        return std::monostate{};
    case 1:
        return read_i64(is);
    case 2:
        return read_f64(is);
    case 3:
        return read_string(is);
    case 4:
        return nullptr;
    case 5: {
        std::string s = read_string(is);
        int n = static_cast<int>(read_u32(is));
        return std::make_pair(s, n);
    }
    default:
        return std::monostate{};
    }
}

static void write_bytecode(std::ostream& os, const std::vector<Instruction>& bc) {
    write_u32(os, static_cast<uint32_t>(bc.size()));
    for (const auto& instr : bc) {
        write_u8(os, static_cast<uint8_t>(instr.op));
        write_operand(os, instr.operand);
        write_i64(os, static_cast<int64_t>(instr.line));
    }
}

static std::vector<Instruction> read_bytecode(std::istream& is) {
    uint32_t count = read_u32(is);
    std::vector<Instruction> bc;
    bc.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        OpCode op = static_cast<OpCode>(read_u8(is));
        Operand operand = read_operand(is);
        int line = static_cast<int>(read_i64(is));
        bc.push_back(Instruction(op, std::move(operand), line));
    }
    return bc;
}

static void write_method(std::ostream& os, const CompiledMethod& m) {
    write_bytecode(os, m.bytecode);
    write_u32(os, static_cast<uint32_t>(m.param_names.size()));
    for (const auto& p : m.param_names)
        write_string(os, p);
    write_u32(os, static_cast<uint32_t>(m.default_value_bytecodes.size()));
    for (const auto& dv : m.default_value_bytecodes) {
        write_bytecode(os, dv);
    }
}

static CompiledMethod read_method(std::istream& is) {
    CompiledMethod m;
    m.bytecode = read_bytecode(is);
    uint32_t pc = read_u32(is);
    for (uint32_t i = 0; i < pc; ++i)
        m.param_names.push_back(read_string(is));
    uint32_t dc = read_u32(is);
    for (uint32_t i = 0; i < dc; ++i)
        m.default_value_bytecodes.push_back(read_bytecode(is));
    return m;
}

static void write_class(std::ostream& os, const CompiledClass& cls) {
    write_string(os, cls.name);
    write_string(os, cls.superclass);
    write_u16(os, cls.id);
    write_u8(os, cls.is_abstract ? 1 : 0);

    write_u32(os, static_cast<uint32_t>(cls.methods.size()));
    for (const auto& [name, m] : cls.methods) {
        write_string(os, name);
        write_method(os, m);
    }
    write_u32(os, static_cast<uint32_t>(cls.static_methods.size()));
    for (const auto& [name, m] : cls.static_methods) {
        write_string(os, name);
        write_method(os, m);
    }
    write_bytecode(os, cls.static_init);
    write_bytecode(os, cls.field_init);

    write_u32(os, static_cast<uint32_t>(cls.private_fields.size()));
    for (const auto& f : cls.private_fields)
        write_string(os, f);
    write_u32(os, static_cast<uint32_t>(cls.private_methods.size()));
    for (const auto& m : cls.private_methods)
        write_string(os, m);
}

static CompiledClass read_class(std::istream& is) {
    CompiledClass cls;
    cls.name = read_string(is);
    cls.superclass = read_string(is);
    cls.id = read_u16(is);
    cls.is_abstract = read_u8(is) != 0;

    uint32_t mc = read_u32(is);
    for (uint32_t i = 0; i < mc; ++i) {
        std::string name = read_string(is);
        cls.methods[name] = read_method(is);
    }
    uint32_t smc = read_u32(is);
    for (uint32_t i = 0; i < smc; ++i) {
        std::string name = read_string(is);
        cls.static_methods[name] = read_method(is);
    }
    cls.static_init = read_bytecode(is);
    cls.field_init = read_bytecode(is);

    uint32_t pfc = read_u32(is);
    for (uint32_t i = 0; i < pfc; ++i)
        cls.private_fields.insert(read_string(is));
    uint32_t pmc = read_u32(is);
    for (uint32_t i = 0; i < pmc; ++i)
        cls.private_methods.insert(read_string(is));

    return cls;
}

bool Program::save_to_file(const std::string& path) const {
    std::ofstream os(path, std::ios::binary);
    if (!os.is_open())
        return false;

    // Header: "ALPH" + version
    os.write("ALPH", 4);
    write_u16(os, version);

    // Main bytecode
    write_bytecode(os, main);
    write_bytecode(os, static_init);

    // Constant pool
    write_u32(os, static_cast<uint32_t>(constant_pool.size()));
    for (const auto& op : constant_pool)
        write_operand(os, op);

    // Globals
    write_u32(os, static_cast<uint32_t>(globals.size()));
    for (const auto& g : globals)
        write_string(os, g);

    // Classes
    write_u32(os, static_cast<uint32_t>(classes.size()));
    for (const auto& [id, cls] : classes) {
        write_u16(os, id);
        write_class(os, cls);
    }

    // Functions
    write_u32(os, static_cast<uint32_t>(functions.size()));
    for (const auto& [name, fn] : functions) {
        write_string(os, name);
        write_method(os, fn);
    }

    return os.good();
}

Program Program::load_from_file(const std::string& path) {
    std::ifstream is(path, std::ios::binary);
    if (!is.is_open())
        throw std::runtime_error("Cannot open bytecode file: " + path);

    // Header
    char magic[4];
    is.read(magic, 4);
    if (std::memcmp(magic, "ALPH", 4) != 0)
        throw std::runtime_error("Invalid bytecode file format");

    Program prog;
    prog.version = read_u16(is);
    if (prog.version != Program::VERSION)
        throw std::runtime_error("Bytecode version mismatch: expected " + std::to_string(Program::VERSION) + ", got " +
                                 std::to_string(prog.version));

    prog.main = read_bytecode(is);
    prog.static_init = read_bytecode(is);

    uint32_t pc = read_u32(is);
    for (uint32_t i = 0; i < pc; ++i)
        prog.constant_pool.push_back(read_operand(is));

    uint32_t gc = read_u32(is);
    for (uint32_t i = 0; i < gc; ++i)
        prog.globals.push_back(read_string(is));

    uint32_t cc = read_u32(is);
    for (uint32_t i = 0; i < cc; ++i) {
        uint16_t id = read_u16(is);
        prog.classes[id] = read_class(is);
    }

    uint32_t fc = read_u32(is);
    for (uint32_t i = 0; i < fc; ++i) {
        std::string name = read_string(is);
        prog.functions[name] = read_method(is);
    }

    return prog;
}

} // namespace alphabet
