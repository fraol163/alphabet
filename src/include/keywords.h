#ifndef ALPHABET_KEYWORDS_H
#define ALPHABET_KEYWORDS_H

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace alphabet {

static const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> KEYWORD_MAPPINGS = {

    {"en",
     {{"class", "c"},  {"abstract", "a"}, {"interface", "j"}, {"method", "m"}, {"if", "i"},       {"else", "e"},
      {"loop", "l"},   {"while", "l"},    {"return", "r"},    {"break", "b"},  {"continue", "k"}, {"new", "n"},
      {"public", "v"}, {"private", "p"},  {"static", "s"},    {"try", "t"},    {"catch", "h"},    {"handle", "h"},
      {"print", "z"},  {"output", "z"},   {"input", "z.i"},   {"import", "x"}, {"match", "q"},    {"extends", "^"},
      {"export", "@"}, {"const", "\x80"}}},

    {"am", {{"ክፍል", "c"},    {"ሥር", "a"},    {"በይነገጽ", "j"}, {"ዘዴ", "m"},  {"ከሆነ", "i"}, {"ያለበለዚያ", "e"},
            {"ሉፕ", "l"},     {"እስከሆነ", "l"}, {"ተመለስ", "r"},  {"ስበር", "b"}, {"ቀጥል", "k"}, {"አዲስ", "n"},
            {"ግልጽ", "v"},    {"ግል", "p"},    {"ቋሚ", "s"},    {"ሞክር", "t"}, {"ያዟ", "h"},  {"ውጤት", "z"},
            {"ግብአት", "z.i"}, {"አስገባ", "x"},  {"ምረጥ", "q"},   {"ወራሽ", "^"}, {"ላክ", "@"},  {"ቋሚ-እሴት", "\x80"}}},

    {"es", {{"clase", "c"},     {"abstracto", "a"}, {"interfaz", "j"}, {"método", "m"},   {"si", "i"},
            {"sino", "e"},      {"bucle", "l"},     {"mientras", "l"}, {"retornar", "r"}, {"romper", "b"},
            {"continuar", "k"}, {"nuevo", "n"},     {"público", "v"},  {"privado", "p"},  {"estático", "s"},
            {"intentar", "t"},  {"capturar", "h"},  {"imprimir", "z"}, {"salida", "z"},   {"entrada", "z.i"},
            {"importar", "x"},  {"coincidir", "q"}, {"extiende", "^"}, {"exportar", "@"}, {"constante", "\x80"}}},

    {"fr", {{"classe", "c"},       {"abstrait", "a"}, {"interface", "j"}, {"méthode", "m"},     {"si", "i"},
            {"sinon", "e"},        {"boucle", "l"},   {"tantque", "l"},   {"retour", "r"},      {"rompre", "b"},
            {"continuer", "k"},    {"nouveau", "n"},  {"public", "v"},    {"privé", "p"},       {"statique", "s"},
            {"essayer", "t"},      {"attraper", "h"}, {"afficher", "z"},  {"entrer", "z.i"},    {"importer", "x"},
            {"correspondre", "q"}, {"étend", "^"},    {"exporter", "@"},  {"constante", "\x80"}}},

    {"de",
     {{"klasse", "c"},         {"abstrakt", "a"},  {"schnittstelle", "j"}, {"methode", "m"},     {"wenn", "i"},
      {"sonst", "e"},          {"schleife", "l"},  {"solange", "l"},       {"zurück", "r"},      {"brechen", "b"},
      {"fortsetzen", "k"},     {"neu", "n"},       {"öffentlich", "v"},    {"privat", "p"},      {"statisch", "s"},
      {"versuchen", "t"},      {"fangen", "h"},    {"ausgeben", "z.o"},      {"eingabe", "z.i"},   {"importieren", "x"},
      {"übereinstimmen", "q"}, {"erweitert", "^"}, {"exportieren", "@"},   {"konstante", "\x80"}}}};

inline std::string translate_keyword(const std::string& keyword, const std::string& lang) {
    auto lang_it = KEYWORD_MAPPINGS.find(lang);
    if (lang_it == KEYWORD_MAPPINGS.end()) {
        return keyword;
    }

    auto keyword_it = lang_it->second.find(keyword);
    if (keyword_it == lang_it->second.end()) {
        return keyword;
    }

    return keyword_it->second;
}

inline bool is_utf8_keyword(const std::string& str) {
    for (unsigned char c : str) {
        if (c > 127)
            return true;
    }
    return false;
}

} // namespace alphabet

#endif
