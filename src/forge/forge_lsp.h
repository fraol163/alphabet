#ifndef ALPHABET_FORGE_LSP_H
#define ALPHABET_FORGE_LSP_H

#include "forge_spec.h"

namespace alphabet {
namespace forge {

class ForgeLanguageServer {
public:
    explicit ForgeLanguageServer(const ForgeSpec& spec) : spec_(spec) {}

    // Run the Language Server Protocol event loop over standard input / standard output
    void run();

private:
    ForgeSpec spec_;
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_LSP_H
