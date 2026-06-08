#pragma once

#include <map>
#include <string>
#include <vector>

namespace alphabet {

class NLToCode {
  public:
    NLToCode();

    // Convert natural language to Alphabet code
    // lang: "en", "es", "fr", "de", "am"
    std::string convert(const std::string& text, const std::string& lang = "en") const;

    // Convert code-speech (keyword-based) to Alphabet code
    std::string convert_code_speech(const std::string& text, const std::string& lang = "en") const;

  private:
    // Language-specific keyword mappings
    std::map<std::string, std::map<std::string, std::string>> keyword_maps_;

    // NL patterns (language-agnostic)
    struct Pattern {
        std::string pattern;     // regex-like pattern
        std::string replacement; // code replacement
    };
    std::vector<Pattern> nl_patterns_;

    // Initialize patterns
    void init_keyword_maps();
    void init_nl_patterns();

    // Helper: apply keyword replacement
    std::string apply_keywords(const std::string& text, const std::string& lang) const;

    // Helper: apply NL patterns
    std::string apply_patterns(const std::string& text) const;

    // Helper: wrap in braces if needed
    std::string wrap_body(const std::string& body) const;

    // Helper: extract strings from quotes
    std::string extract_string(const std::string& text, size_t& pos) const;

    // Helper: Amharic post-processing
    std::string post_process_amharic(const std::string& text) const;
};

} // namespace alphabet
