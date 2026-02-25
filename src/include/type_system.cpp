#include "type_system.h"

namespace alphabet {

TypeManager::TypeManager() {
    register_primitive(I8, "i8");
    register_primitive(I16, "i16");
    register_primitive(I32, "i32");
    register_primitive(I64, "i64");
    register_primitive(INT, "int");
    register_primitive(F32, "f32");
    register_primitive(F64, "f64");
    register_primitive(FLOAT, "float");
    register_primitive(DEC, "dec");
    register_primitive(CPX, "cpx");
    register_primitive(BOOL, "bool");
    register_primitive(STR, "str");
    register_primitive(LIST, "list");
    register_primitive(MAP, "map");
}

void TypeManager::register_primitive(uint16_t id, const std::string& name) {
    types_.emplace_back(id, name, true);
    name_to_id_[name] = id;
}

const TypeInfo* TypeManager::get_type(uint16_t id) const {
    if (id == 0 || id > types_.size()) {
        return nullptr;
    }
    return &types_[id - 1];
}

uint16_t TypeManager::register_type(const std::string& name,
                                    const std::vector<uint16_t>& interfaces) {
    auto it = name_to_id_.find(name);
    if (it != name_to_id_.end()) {
        throw TypeError("Type '" + name + "' already registered with ID " + 
                       std::to_string(it->second));
    }
    
    uint16_t id = next_custom_id_++;
    types_.emplace_back(id, name, false);
    types_.back().interfaces = interfaces;
    name_to_id_[name] = id;
    
    return id;
}

bool TypeManager::is_compatible(uint16_t source_type, uint16_t target_type) const {
    if (source_type == target_type) return true;

    const TypeInfo* source = get_type(source_type);
    const TypeInfo* target = get_type(target_type);

    if (!source || !target) return false;

    if (source->is_primitive && target->is_primitive) {
        if (source_type <= I64 && target_type <= I64) {
            return source_type <= target_type;
        }
        if ((source_type == F32 || source_type == FLOAT) &&
            (target_type == F64 || target_type == FLOAT)) {
            return true;
        }
        return false;
    }

    if (!target->is_primitive) {
        return implements_interface(source_type, target_type);
    }

    return false;
}

bool TypeManager::implements_interface(uint16_t type_id, uint16_t interface_id) const {
    const TypeInfo* type = get_type(type_id);
    if (!type) return false;

    for (uint16_t iface : type->interfaces) {
        if (iface == interface_id) return true;
    }

    return false;
}

}
