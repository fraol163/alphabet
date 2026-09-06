#include "ffi.h"
#include <algorithm>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#define strdup _strdup
#else
#include <dlfcn.h>
#endif

extern "C" {

// Global registry for ffi_register_function. Holds (name, function pointer,
// arg types, return type) for functions registered without dlopen — useful
// for embedding the alphabet VM in another host that wants to expose
// C/C++ functions directly without writing them to a shared library.
//
// Thread-safety: the registry is touched only at registration time (rare)
// and the lookup is read-only after that. A single global mutex protects
// the underlying map.
static std::unordered_map<std::string, std::tuple<void*, std::vector<FFIType>, FFIType>>& ffi_registry() {
    static std::unordered_map<std::string, std::tuple<void*, std::vector<FFIType>, FFIType>> reg;
    return reg;
}
static std::mutex& ffi_registry_mutex() {
    static std::mutex m;
    return m;
}

FFI_EXPORT int ffi_init(void) {
    return 1;
}

FFI_EXPORT void ffi_cleanup(void) {
    std::lock_guard<std::mutex> lock(ffi_registry_mutex());
    ffi_registry().clear();
}

FFI_EXPORT FFIResult ffi_call(const char* lib, const char* func, FFIValue* args, int arg_count) {
    FFIResult result = {0, ffi_make_null(), nullptr};

    if (!func) {
        result.error_message = "Function name is required";
        return result;
    }
    // lib is allowed to be nullptr when the function is in the registry.

    // First, check the registered-function registry. This lets embedders
    // expose C/C++ functions to the VM without compiling them into a
    // separate shared library. The registry stores function pointers
    // directly; we don't need dlopen to find them.
    {
        std::lock_guard<std::mutex> lock(ffi_registry_mutex());
        auto it = ffi_registry().find(func);
        if (it != ffi_registry().end()) {
            // For registered functions we ignore the `lib` argument and
            // just dispatch through the stored function pointer. Argument
            // and return type info is stored alongside but currently
            // unused — the registered function takes/returns FFIValue
            // and is expected to do its own marshalling.
            auto& entry = it->second;
            void* f = std::get<0>(entry);
            using RegisteredSig = FFIValue (*)(FFIValue*, int);
            auto fn = reinterpret_cast<RegisteredSig>(f);
            result.value = fn(args, arg_count);
            result.success = 1;
            return result;
        }
    }

    if (!lib) {
        result.error_message = "Function not found in registry and no library given";
        return result;
    }

#ifdef _WIN32
    HMODULE handle = LoadLibraryA(lib);
    if (!handle) {
        result.error_message = "Failed to load library";
        return result;
    }

    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(GetProcAddress(handle, func));
#else
    void* handle = dlopen(lib, RTLD_NOW);
    if (!handle) {
        result.error_message = dlerror();
        return result;
    }

    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(dlsym(handle, func));
#endif

    if (!f) {
#ifdef _WIN32
        result.error_message = "Function not found in library";
#else
        result.error_message = dlerror();
        dlclose(handle);
#endif
        return result;
    }

    result.value = f(args, arg_count);
    result.success = 1;

#ifndef _WIN32
    dlclose(handle);
#endif
    return result;
}

FFI_EXPORT void* ffi_load_library(const char* path) {
    if (!path)
        return nullptr;
#ifdef _WIN32
    return reinterpret_cast<void*>(LoadLibraryA(path));
#else
    return dlopen(path, RTLD_NOW | RTLD_GLOBAL);
#endif
}

FFI_EXPORT void ffi_unload_library(void* handle) {
    if (handle) {
#ifdef _WIN32
        FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
    }
}

FFI_EXPORT int ffi_register_function(const char* name, void* func_ptr, FFIType* arg_types, int arg_count,
                                     FFIType return_type) {
    if (!name || !func_ptr) return 0;
    std::lock_guard<std::mutex> lock(ffi_registry_mutex());
    std::vector<FFIType> args;
    if (arg_types && arg_count > 0) {
        args.assign(arg_types, arg_types + arg_count);
    }
    ffi_registry()[name] = std::make_tuple(func_ptr, std::move(args), return_type);
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
}

#ifdef __cplusplus

namespace alphabet {
namespace ffi {

struct FFIBridge::LibraryHandle {
    void* handle = nullptr;
    std::string path;

    ~LibraryHandle() {
        if (handle) {
#ifdef _WIN32
            FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
        }
    }
};

FFIBridge::FFIBridge() = default;

FFIBridge::~FFIBridge() {
    unload_all();
}

bool FFIBridge::load_library(const std::string& path) {
#ifdef _WIN32
    void* handle = reinterpret_cast<void*>(LoadLibraryA(path.c_str()));
#else
    void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
    if (!handle) {
        return false;
    }

    libraries_.push_back({handle, path});
    return true;
}

void FFIBridge::unload_all() {
    libraries_.clear();
}

FFIArg FFIBridge::call(const std::string& lib_path, const std::string& func_name, const std::vector<FFIArg>& args) {
    void* handle = nullptr;
    auto it = std::find_if(libraries_.begin(), libraries_.end(),
                           [&lib_path](const auto& lib) { return lib.path == lib_path; });
    if (it != libraries_.end()) {
        handle = it->handle;
    }
    if (!handle) {
        if (!load_library(lib_path)) {
            throw std::runtime_error("Failed to load library: " + lib_path);
        }
        handle = libraries_.back().handle;
    }

#ifdef _WIN32
    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(GetProcAddress(reinterpret_cast<HMODULE>(handle), func_name.c_str()));
#else
    typedef FFIValue (*FuncType)(FFIValue*, int);
    FuncType f = reinterpret_cast<FuncType>(dlsym(handle, func_name.c_str()));
#endif

    if (!f) {
        throw std::runtime_error("Function '" + func_name + "' not found in " + lib_path);
    }

    std::vector<FFIValue> ffi_args(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        ffi_args[i] = to_ffi_value(args[i]);
    }

    FFIValue result = f(ffi_args.data(), static_cast<int>(args.size()));

    for (auto& arg : ffi_args) {
        if (arg.type == FFI_TYPE_STRING && arg.data.string_val) {
            free(const_cast<char*>(arg.data.string_val));
        }
    }

    return from_ffi_value(result);
}

FFIValue to_ffi_value(const FFIArg& arg) {
    return std::visit(
        [](const auto& v) -> FFIValue {
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
            } else {
                return ffi_make_null();
            }
        },
        arg);
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

#endif
