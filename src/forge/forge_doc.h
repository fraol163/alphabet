#ifndef ALPHABET_FORGE_DOC_H
#define ALPHABET_FORGE_DOC_H

#include "forge_spec.h"
#include <string>

namespace alphabet {
namespace forge {

class ForgeDocGenerator {
public:
    // Generate complete interactive HTML documentation site
    static bool generate_html_docs(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_DOC_H
