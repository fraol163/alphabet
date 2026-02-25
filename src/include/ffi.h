#ifndef ALPHABET_FFI_H
#define ALPHABET_FFI_H

#include <string>
#include <vector>
#include <variant>
#include <cstdint>

#ifdef _WIN32
    #define FFI_EXPORT __declspec(dllexport)
#else
    #define FFI_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FFI_TYPE_NULL = 0,
    FFI_TYPE_INT = 1,
    FFI_TYPE_FLOAT = 2,
    FFI_TYPE_STRING = 3,
    FFI_TYPE_BOOL = 4
} FFIType;

typedef struct {
    FFIType type;
    union {
        int64_t int_val;
        double float_val;
        const char* string_val;
        int bool_val;
    } data;
} FFIValue;

typedef struct {
    int success;
    FFIValue value;
    const char* error_message;
} FFIResult;

FFI_EXPORT int ffi_init(void);
FFI_EXPORT void ffi_cleanup(void);

FFI_EXPORT FFIResult ffi_call(const char* lib, const char* func,
                              FFIValue* args, int arg_count);
FFI_EXPORT void* ffi_load_library(const char* path);
FFI_EXPORT void ffi_unload_library(void* handle);
FFI_EXPORT int ffi_register_function(const char* name, void* func_ptr,
                                     FFIType* arg_types, int arg_count,
                                     FFIType return_type);
FFI_EXPORT FFIValue ffi_make_int(int64_t val);
FFI_EXPORT FFIValue ffi_make_float(double val);
FFI_EXPORT FFIValue ffi_make_string(const char* val);
FFI_EXPORT FFIValue ffi_make_bool(int val);
FFI_EXPORT FFIValue ffi_make_null(void);
FFI_EXPORT void ffi_free_value(FFIValue* val);

#ifdef __cplusplus
}

namespace alphabet {
namespace ffi {

using FFIArg = std::variant<std::monostate, int64_t, double, std::string, bool>;

class FFIBridge {
public:
    FFIBridge();
    ~FFIBridge();

    bool load_library(const std::string& path);
    void unload_all();
    FFIArg call(const std::string& lib_path, const std::string& func_name,
                const std::vector<FFIArg>& args);

    template<typename Func>
    bool register_function(const std::string& name, Func func);

private:
    struct LibraryHandle;
    std::vector<LibraryHandle> libraries_;
};

FFIValue to_ffi_value(const FFIArg& arg);
FFIArg from_ffi_value(const FFIValue& val);

}
}

#endif

#endif
