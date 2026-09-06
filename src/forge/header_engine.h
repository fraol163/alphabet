#ifndef ALPHABET_FORGE_HEADER_ENGINE_H
#define ALPHABET_FORGE_HEADER_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>

namespace alphabet {
namespace forge {

enum class HeaderStyle {
    PREFIX,     // e.g. #nova<v1.0> or #nova<1.0, compiled>
    SHEBANG,    // e.g. #!/usr/bin/env nova
    PRAGMA,     // e.g. @nova(version=1.0)
    ALPHABET,   // e.g. #alphabet<nova>
    NONE        // Zero header required, pure code
};

struct HeaderConfig {
    HeaderStyle style = HeaderStyle::PREFIX;
    std::string prefix = "#";               // e.g. "#nova" or "@nova"
    bool required = false;                  // Is header mandatory?
    std::string syntax_pattern = "<{version},{mode}>";
    std::string default_version = "1.0.0";
    std::string default_mode = "compiled";
};

struct HeaderParseResult {
    bool has_header = false;
    bool valid = true;
    std::string error;
    std::string detected_language;
    std::string version;
    std::string mode;
    std::unordered_map<std::string, std::string> attributes;
    std::string stripped_source;
    int header_lines = 0;
};

class HeaderEngine {
public:
    static HeaderStyle parse_style(const std::string& style_str);
    static std::string style_to_string(HeaderStyle style);

    // Parse source code with a specific language header configuration
    static HeaderParseResult parse(const std::string& source, 
                                   const HeaderConfig& config, 
                                   const std::string& expected_lang);

    // Auto-detect language and header from raw source file without prior config
    static HeaderParseResult auto_detect(const std::string& source);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_HEADER_ENGINE_H
