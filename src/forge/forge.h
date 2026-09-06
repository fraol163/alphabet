#ifndef ALPHABET_FORGE_H
#define ALPHABET_FORGE_H

#include "forge_spec.h"
#include "header_engine.h"
#include "peg_engine.h"
#include "banner_engine.h"
#include "compiler.h"
#include "vm.h"

namespace alphabet {
namespace forge {

class ForgeRunner {
public:
    // Execute a custom language file directly
    static bool run_file(const std::string& filepath, 
                         const std::string& spec_filepath = "",
                         bool debug = false);

    // Execute source code directly using a loaded ForgeSpec
    static bool run_source(const std::string& source,
                           const ForgeSpec& spec,
                           const std::string& filename = "main",
                           bool debug = false);

    // Compile source into Alphabet bytecode Program
    static bool compile_source(const std::string& source,
                              const ForgeSpec& spec,
                              alphabet::Program& out_program,
                              std::vector<DiagnosticError>& out_errors,
                              const std::string& filename = "main");

    // Package a standalone binary for the language (Phase 2 Exporter)
    static bool export_toolchain(const ForgeSpec& spec,
                                const std::string& output_binary_path,
                                const std::string& target_mode = "bytecode");

    // Start persistent interactive REPL for custom language
    static void run_repl(const ForgeSpec& spec);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_H
