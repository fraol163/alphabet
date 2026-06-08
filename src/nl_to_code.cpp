#include "nl_to_code.h"
#include <algorithm>
#include <regex>
#include <sstream>

namespace alphabet {

NLToCode::NLToCode() {
    init_keyword_maps();
    init_nl_patterns();
}

void NLToCode::init_keyword_maps() {
    keyword_maps_["en"] = {
        {"if", "i"},     {"else", "e"},    {"loop", "l"},   {"while", "l"},   {"break", "b"},   {"continue", "k"},
        {"return", "r"}, {"class", "c"},   {"method", "m"}, {"new", "n"},     {"print", "z.o"}, {"output", "z.o"},
        {"show", "z.o"}, {"input", "z.i"}, {"import", "x"}, {"match", "q"},   {"and", "&&"},    {"or", "||"},
        {"not", "!"},    {"true", "1"},    {"false", "0"},  {"null", "null"},
    };

    keyword_maps_["am"] = {
        {"ከሆነ", "i"},  {"ያለበለዚያ", "e"}, {"ሉፕ", "l"},   {"እስከሆነ", "l"}, {"ስበር", "b"},   {"ቀጥል", "k"},
        {"ተመለስ", "r"}, {"ክፍል", "c"},    {"ዘዴ", "m"},   {"አዲስ", "n"},   {"ውጤት", "z.o"}, {"ግብአት", "z.i"},
        {"አስገባ", "x"}, {"እና", "&&"},    {"ወይም", "||"}, {"እውነት", "1"},  {"ሐሰት", "0"},
    };

    keyword_maps_["es"] = {
        {"si", "i"},       {"sino", "e"},      {"bucle", "l"},    {"mientras", "l"},
        {"romper", "b"},   {"continuar", "k"}, {"retornar", "r"}, {"imprimir", "z.o"},
        {"importar", "x"}, {"y", "&&"},        {"o", "||"},       {"no", "!"},
    };

    keyword_maps_["fr"] = {
        {"si", "i"},     {"sinon", "e"},      {"boucle", "l"},   {"tantque", "l"}, {"rompre", "b"}, {"continuer", "k"},
        {"retour", "r"}, {"afficher", "z.o"}, {"importer", "x"}, {"et", "&&"},     {"ou", "||"},    {"non", "!"},
    };

    keyword_maps_["de"] = {
        {"wenn", "i"},        {"sonst", "e"},      {"schleife", "l"}, {"solange", "l"},
        {"brechen", "b"},     {"fortsetzen", "k"}, {"zurück", "r"},   {"ausgeben", "z.o"},
        {"importieren", "x"}, {"und", "&&"},       {"oder", "||"},    {"nicht", "!"},
    };
}

void NLToCode::init_nl_patterns() {
    // Variable declarations
    nl_patterns_.push_back({"variable (\\w+) equals (\\d+)", "5 $1 = $2"});
    nl_patterns_.push_back({"set (\\w+) to (\\d+)", "5 $1 = $2"});

    // If/else
    nl_patterns_.push_back({"if (\\w+) greater than (\\w+) then (.+)", "i ($1 > $2) { $3 }"});
    nl_patterns_.push_back({"if (\\w+) less than (\\w+) then (.+)", "i ($1 < $2) { $3 }"});
    nl_patterns_.push_back({"if (\\w+) equals (\\w+) then (.+)", "i ($1 == $2) { $3 }"});

    // Print
    nl_patterns_.push_back({"print (\\w+)", "z.o($1)"});

    // Loops
    nl_patterns_.push_back({"loop from (\\d+) to (\\d+)", "l (5 i = $1 : i < $2 : i = i + 1)"});

    // Math
    nl_patterns_.push_back({"(\\w+) plus (\\w+)", "($1 + $2)"});
    nl_patterns_.push_back({"(\\w+) minus (\\w+)", "($1 - $2)"});
    nl_patterns_.push_back({"(\\w+) times (\\w+)", "($1 * $2)"});

    // Return
    nl_patterns_.push_back({"return (\\w+)", "r $1"});

    // Function
    nl_patterns_.push_back({"function (\\w+) takes (\\w+) and (\\w+)", "m 5 $1(5 $2, 5 $3)"});
    nl_patterns_.push_back({"function (\\w+) takes (\\w+)", "m 5 $1(5 $2)"});
    nl_patterns_.push_back({"function (\\w+)", "m 5 $1()"});
}

std::string NLToCode::convert(const std::string& text, const std::string& lang) const {
    std::string result = apply_keywords(text, lang);
    if (result == text) {
        result = apply_patterns(text);
    }
    if (lang == "am") {
        result = post_process_amharic(result);
    }
    return result;
}

std::string NLToCode::convert_code_speech(const std::string& text, const std::string& lang) const {
    return apply_keywords(text, lang);
}

std::string NLToCode::apply_keywords(const std::string& text, const std::string& lang) const {
    auto lang_it = keyword_maps_.find(lang);
    if (lang_it == keyword_maps_.end())
        return text;

    std::string result = text;
    const auto& keywords = lang_it->second;

    std::vector<std::pair<std::string, std::string>> sorted_kw(keywords.begin(), keywords.end());
    std::sort(sorted_kw.begin(), sorted_kw.end(),
              [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });

    for (const auto& [native, code] : sorted_kw) {
        size_t pos = 0;
        while ((pos = result.find(native, pos)) != std::string::npos) {
            bool left_ok = (pos == 0 || !std::isalnum(result[pos - 1]));
            bool right_ok = (pos + native.size() >= result.size() || !std::isalnum(result[pos + native.size()]));
            if (left_ok && right_ok) {
                result.replace(pos, native.size(), code);
                pos += code.size();
            } else {
                pos += native.size();
            }
        }
    }
    return result;
}

std::string NLToCode::apply_patterns(const std::string& text) const {
    std::string result = text;
    for (const auto& p : nl_patterns_) {
        try {
            std::regex re(p.pattern, std::regex::icase);
            result = std::regex_replace(result, re, p.replacement);
        } catch (...) {
        }
    }
    return result;
}

std::string NLToCode::post_process_amharic(const std::string& text) const {
    std::string result = text;
    std::vector<std::pair<std::string, std::string>> fixes = {
        {"ከሆነ", "i"},   {"ያለበለዚያ", "e"}, {"ሉፕ", "l"},   {"ስበር", "b"},  {"ቀጥል", "k"}, {"ተመለስ", "r"},  {"ክፍል", "c"},
        {"ዘዴ", "m"},    {"አዲስ", "n"},    {"ግልጽ", "v"},  {"ግል", "p"},   {"ቋሚ", "s"},  {"ሞክር", "t"},   {"ያዟ", "h"},
        {"ውጤት", "z.o"}, {"ግብአት", "z.i"}, {"አስገባ", "x"}, {"ላክ", "@"},   {"ምረጥ", "q"}, {"ሥር", "a"},    {"በይነገጽ", "j"},
        {"ወራሽ", "^"},   {"እና", "&&"},    {"ወይም", "||"}, {"እውነት", "1"}, {"ሐሰት", "0"}, {"ባዶ", "null"},
    };
    for (const auto& [am, code] : fixes) {
        size_t pos = 0;
        while ((pos = result.find(am, pos)) != std::string::npos) {
            result.replace(pos, am.size(), code);
            pos += code.size();
        }
    }
    return result;
}

std::string NLToCode::wrap_body(const std::string& body) const {
    if (body.find('{') == std::string::npos)
        return "{ " + body + " }";
    return body;
}

std::string NLToCode::extract_string(const std::string& text, size_t& pos) const {
    return "";
}

} // namespace alphabet
