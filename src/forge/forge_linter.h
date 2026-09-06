#ifndef ALPHABET_FORGE_LINTER_H
#define ALPHABET_FORGE_LINTER_H

#include "forge_spec.h"
#include <string>
#include <vector>

namespace alphabet {
namespace forge {

struct LintWarning {
    std::string message;
    std::string filepath;
    int line = 1;
    int column = 1;
    std::string line_content;
    std::string hint;
    bool is_error = false;

    std::string format() const;
};

struct LintReport {
    std::vector<LintWarning> warnings;
    int error_count = 0;
    int warning_count = 0;
};

class ForgeLinter {
public:
    static LintReport lint_source(const std::string& source, const ForgeSpec& spec, const std::string& filename = "<source>");
    static LintReport lint_file(const std::string& filepath, const ForgeSpec& spec);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_LINTER_H
