#ifndef ALPHABET_FORGE_C_EMITTER_H
#define ALPHABET_FORGE_C_EMITTER_H

#include "forge_spec.h"
#include "alphabet_ast.h"
#include <string>
#include <vector>

namespace alphabet {
namespace forge {

class ForgeCEmitter {
public:
    // Transpile AST statements directly into clean, self-contained C code
    static std::string emit_c(const std::vector<alphabet::StmtPtr>& statements, const ForgeSpec& spec);

    // End-to-end: compile source code to native machine binary using system C compiler (gcc/clang)
    static bool compile_to_native(const std::string& source,
                                  const ForgeSpec& spec,
                                  const std::string& output_binary,
                                  std::string& out_error,
                                  bool keep_c_source = false,
                                  const std::string& target_triple = "");
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_C_EMITTER_H
