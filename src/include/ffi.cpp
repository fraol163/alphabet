#include "ffi.h"
#include <dlfcn.h>
#include <cstring>
#include <stdexcept>

extern "C" {

FFI_EXPORT int ffi_init(void) {
    return 1;
}

FFI_EXPORT void ffi_cleanup(void) {
}

FFI_EXPORT FFIResult ffi_call(const char* lib, const char* func,
                              FFIValue* args, int arg_count) {
    FFIResult result = {0, ffi_make_null(), nullptr};
    
    if (!lib || !func) {
        result.error_message = "Invalid library or function name";
        return result;
    }
    
    // Load the library
    void* handle = dlopen(lib, RTLD_NOW);
    if (!handle) {
        result.error_message = dlerror();
        return result;
    }
    
    // Get function pointer
    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(dlsym(handle, func));
    if (!f) {
        result.error_message = dlerror();
        dlclose(handle);
        return result;
    }
    
    // Call the function
    result.value = f(args, arg_count);
    result.success = 1;
    
    dlclose(handle);
    return result;
}

FFI_EXPORT void* ffi_load_library(const char* path) {
    if (!path) return nullptr;
    return dlopen(path, RTLD_NOW | RTLD_GLOBAL);
}

FFI_EXPORT void ffi_unload_library(void* handle) {
    if (handle) {
        dlclose(handle);
    }
}

FFI_EXPORT int ffi_register_function(const char* name, void* func_ptr,
                                     FFIType* arg_types, int arg_count,
                                     FFIType return_type) {
    (void)name;
    (void)func_ptr;
    (void)arg_types;
    (void)arg_count;
    (void)return_type;
    return 1;
}

FFI_EXPORT FFIValue ffi_make_int(int64_t val) {
    FFIValue v;
    v.type = FFI_TYPE_INT;
    v.data.int_val = val;
    return v;
}

FFI_EXPORT FFIValue ffi_make_float(double val) {
    FFIValue v;
    v.type = FFI_TYPE_FLOAT;
    v.data.float_val = val;
    return v;
}

FFI_EXPORT FFIValue ffi_make_string(const char* val) {
    FFIValue v;
    v.type = FFI_TYPE_STRING;
    v.data.string_val = val ? strdup(val) : nullptr;
    return v;
}

FFI_EXPORT FFIValue ffi_make_bool(int val) {
    FFIValue v;
    v.type = FFI_TYPE_BOOL;
    v.data.bool_val = val;
    return v;
}

FFI_EXPORT FFIValue ffi_make_null(void) {
    FFIValue v;
    v.type = FFI_TYPE_NULL;
    v.data.int_val = 0;
    return v;
}

FFI_EXPORT void ffi_free_value(FFIValue* val) {
    if (val && val->type == FFI_TYPE_STRING && val->data.string_val) {
        free(const_cast<char*>(val->data.string_val));
        val->data.string_val = nullptr;
    }
}

} // extern "C"

// ============================================================================
// C++ API Implementation
// ============================================================================

#ifdef __cplusplus

namespace alphabet {
namespace ffi {

struct FFIBridge::LibraryHandle {
    void* handle = nullptr;
    std::string path;
    
    ~LibraryHandle() {
        if (handle) {
            dlclose(handle);
        }
    }
};

FFIBridge::FFIBridge() = default;

FFIBridge::~FFIBridge() {
    unload_all();
}

bool FFIBridge::load_library(const std::string& path) {
    void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        return false;
    }
    
    libraries_.push_back({handle, path});
    return true;
}

void FFIBridge::unload_all() {
    libraries_.clear();
}

FFIArg FFIBridge::call(const std::string& lib_path, const std::string& func_name,
                       const std::vector<FFIArg>& args) {
    // Load library temporarily
    void* handle = dlopen(lib_path.c_str(), RTLD_NOW);
    if (!handle) {
        throw std::runtime_error(dlerror());
    }
    
    // Get function - assuming standard FFI signature
    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(dlsym(handle, func_name.c_str()));
    if (!f) {
        dlclose(handle);
        throw std::runtime_error(dlerror());
    }
    
    // Convert arguments
    std::vector<FFIValue> ffi_args(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        ffi_args[i] = to_ffi_value(args[i]);
    }
    
    // Call function
    FFIValue result = f(ffi_args.data(), static_cast<int>(args.size()));
    
    // Cleanup argument strings
    for (auto& arg : ffi_args) {
        if (arg.type == FFI_TYPE_STRING && arg.data.string_val) {
            free(const_cast<char*>(arg.data.string_val));
        }
    }
    
    dlclose(handle);
    
    return from_ffi_value(result);
}

FFIValue to_ffi_value(const FFIArg& arg) {
    return std::visit([](const auto& v) -> FFIValue {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return ffi_make_null();
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return ffi_make_int(v);
        } else if constexpr (std::is_same_v<T, double>) {
            return ffi_make_float(v);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return ffi_make_string(v.c_str());
        } else if constexpr (std::is_same_v<T, bool>) {
            return ffi_make_bool(v ? 1 : 0);
        }
        return ffi_make_null();
    }, arg);
}

FFIArg from_ffi_value(const FFIValue& val) {
    switch (val.type) {
        case FFI_TYPE_NULL:
            return std::monostate{};
        case FFI_TYPE_INT:
            return val.data.int_val;
        case FFI_TYPE_FLOAT:
            return val.data.float_val;
        case FFI_TYPE_STRING:
            return val.data.string_val ? std::string(val.data.string_val) : std::string();
        case FFI_TYPE_BOOL:
            return val.data.bool_val != 0;
        default:
            return std::monostate{};
    }
}

} // namespace ffi
} // namespace alphabet

#endif // __cplusplus
