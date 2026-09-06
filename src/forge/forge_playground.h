#ifndef ALPHABET_FORGE_PLAYGROUND_H
#define ALPHABET_FORGE_PLAYGROUND_H

#include "forge_spec.h"
#include <string>

namespace alphabet {
namespace forge {

class ForgePlayground {
public:
    // Generate interactive Web Playground / IDE in output_dir (index.html)
    static bool export_playground(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_PLAYGROUND_H
