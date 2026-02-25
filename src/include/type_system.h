#ifndef ALPHABET_TYPE_SYSTEM_H
#define ALPHABET_TYPE_SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

namespace alphabet {

struct TypeInfo {
    uint16_t id;
    std::string name;
    bool is_primitive;
    std::vector<uint16_t> interfaces;  // Implemented interface IDs
    
    TypeInfo(uint16_t i, std::string n, bool prim)
        : id(i), name(std::move(n)), is_primitive(prim) {}
};

class TypeError : public std::runtime_error {
public:
    explicit TypeError(const std::string& msg) : std::runtime_error(msg) {}
};

class TypeManager {
public:
    TypeManager();

    const TypeInfo* get_type(uint16_t id) const;
    uint16_t register_type(const std::string& name,
                          const std::vector<uint16_t>& interfaces = {});
    bool is_compatible(uint16_t source_type, uint16_t target_type) const;
    bool implements_interface(uint16_t type_id, uint16_t interface_id) const;
    [[nodiscard]] uint16_t next_custom_id() const { return next_custom_id_; }
    
    // Primitive type IDs (1-14)
    static constexpr uint16_t I8 = 1;
    static constexpr uint16_t I16 = 2;
    static constexpr uint16_t I32 = 3;
    static constexpr uint16_t I64 = 4;
    static constexpr uint16_t INT = 5;
    static constexpr uint16_t F32 = 6;
    static constexpr uint16_t F64 = 7;
    static constexpr uint16_t FLOAT = 8;
    static constexpr uint16_t DEC = 9;
    static constexpr uint16_t CPX = 10;
    static constexpr uint16_t BOOL = 11;
    static constexpr uint16_t STR = 12;
    static constexpr uint16_t LIST = 13;
    static constexpr uint16_t MAP = 14;

private:
    std::vector<TypeInfo> types_;
    std::unordered_map<std::string, uint16_t> name_to_id_;
    uint16_t next_custom_id_ = 15;

    void register_primitive(uint16_t id, const std::string& name);
};

} // namespace alphabet

#endif // ALPHABET_TYPE_SYSTEM_H
