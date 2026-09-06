#ifndef ALPHABET_FORGE_PEG_ENGINE_H
#define ALPHABET_FORGE_PEG_ENGINE_H

#include "forge_spec.h"
#include "alphabet_ast.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace alphabet {
namespace forge {

struct DiagnosticError {
    int line = 1;
    int column = 1;
    std::string message;
    std::string line_content;
    std::string hint;

    std::string format(const std::string& filename = "") const;
};

struct ParseResult {
    bool success = false;
    std::vector<alphabet::StmtPtr> statements;
    std::vector<DiagnosticError> errors;
};

class PegEngine {
public:
    explicit PegEngine(const ForgeSpec& spec);

    // Parse source code conforming to the ForgeSpec into Alphabet AST statements
    ParseResult parse(const std::string& source, const std::string& filename = "main", int line_offset = 0);

private:
    ForgeSpec spec_;
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_PEG_ENGINE_H
