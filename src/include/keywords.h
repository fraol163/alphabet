#ifndef ALPHABET_KEYWORDS_H
#define ALPHABET_KEYWORDS_H

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace alphabet {

// Keyword mappings for different languages
// Each language maps native keywords to ASCII equivalents

static const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> KEYWORD_MAPPINGS = {
    // English aliases (multi-letter keywords)
    {"en", {
        {"class", "c"},
        {"method", "m"},
        {"if", "i"},
        {"else", "e"},
        {"loop", "l"},
        {"while", "l"},
        {"return", "r"},
        {"break", "b"},
        {"continue", "k"},
        {"new", "n"},
        {"public", "v"},
        {"private", "p"},
        {"static", "s"},
        {"try", "t"},
        {"catch", "h"},
        {"handle", "h"},
        {"print", "z"},
        {"output", "z"},
        {"input", "z.i"},
        {"import", "x"},
        {"match", "q"}
    }},
    
    // Amharic keywords
    {"am", {
        {"ክፍል", "c"},        // class
        {"ዘዴ", "m"},          // method
        {"ከሆነ", "i"},        // if
        {"አለበለዚህ", "e"},    // else
        {"ሉፕ", "l"},         // loop
        {"ተመለስ", "r"},       // return
        {"ስበር", "b"},        // break
        {"ቀጥል", "k"},        // continue
        {"አዲስ", "n"},        // new
        {"ግልጽ", "v"},        // public
        {"ግል", "p"},         // private
        {"ቋሚ", "s"},        // static
        {"ሞክር", "t"},        // try
        {"ይዝ", "h"},         // catch/handle
        {"ውጤት", "z"},       // print/output
        {"ግብአት", "z.i"},    // input
        {"አስገባ", "x"},       // import
        {"ምረጥ", "q"}         // match
    }},

    // Spanish keywords
    {"es", {
        {"clase", "c"},
        {"metodo", "m"},
        {"si", "i"},
        {"sino", "e"},
        {"bucle", "l"},
        {"mientras", "l"},
        {"retornar", "r"},
        {"romper", "b"},
        {"continuar", "k"},
        {"nuevo", "n"},
        {"publico", "v"},
        {"privado", "p"},
        {"estatico", "s"},
        {"intentar", "t"},
        {"capturar", "h"},
        {"imprimir", "z"},
        {"salida", "z"},
        {"entrada", "z.i"},
        {"importar", "x"},
        {"coincidir", "q"}
    }},

    // French keywords
    {"fr", {
        {"classe", "c"},
        {"methode", "m"},
        {"si", "i"},
        {"sinon", "e"},
        {"boucle", "l"},
        {"retour", "r"},
        {"rompre", "b"},
        {"continuer", "k"},
        {"nouveau", "n"},
        {"public", "v"},
        {"prive", "p"},
        {"statique", "s"},
        {"essayer", "t"},
        {"attraper", "h"},
        {"afficher", "z"},
        {"entrer", "z.i"},
        {"importer", "x"},
        {"correspondre", "q"}
    }},

    // German keywords
    {"de", {
        {"klasse", "c"},
        {"methode", "m"},
        {"wenn", "i"},
        {"sonst", "e"},
        {"schleife", "l"},
        {"zuruck", "r"},
        {"brechen", "b"},
        {"fortsetzen", "k"},
        {"neu", "n"},
        {"offentlich", "v"},
        {"privat", "p"},
        {"statisch", "s"},
        {"versuchen", "t"},
        {"fangen", "h"},
        {"ausgeben", "z"},
        {"eingabe", "z.i"},
        {"importieren", "x"},
        {"ubereinstimmen", "q"}
    }}
};

// Helper function to get keyword mapping for a language
inline std::string translate_keyword(const std::string& keyword, const std::string& lang) {
    auto lang_it = KEYWORD_MAPPINGS.find(lang);
    if (lang_it == KEYWORD_MAPPINGS.end()) {
        return keyword;  // No mapping for this language
    }
    
    auto keyword_it = lang_it->second.find(keyword);
    if (keyword_it == lang_it->second.end()) {
        return keyword;  // No mapping for this keyword
    }
    
    return keyword_it->second;
}

// Check if a string contains UTF-8 characters
inline bool is_utf8_keyword(const std::string& str) {
    for (unsigned char c : str) {
        if (c > 127) return true;
    }
    return false;
}

}

#endif
