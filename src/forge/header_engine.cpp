#include "header_engine.h"
#include <algorithm>
#include <sstream>
#include <regex>

namespace alphabet {
namespace forge {

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

HeaderStyle HeaderEngine::parse_style(const std::string& style_str) {
    std::string s = to_lower(trim(style_str));
    if (s == "shebang" || s == "hashbang") return HeaderStyle::SHEBANG;
    if (s == "pragma" || s == "decorator") return HeaderStyle::PRAGMA;
    if (s == "alphabet") return HeaderStyle::ALPHABET;
    if (s == "none" || s == "clean" || s == "zero") return HeaderStyle::NONE;
    return HeaderStyle::PREFIX;
}

std::string HeaderEngine::style_to_string(HeaderStyle style) {
    switch (style) {
        case HeaderStyle::PREFIX: return "prefix";
        case HeaderStyle::SHEBANG: return "shebang";
        case HeaderStyle::PRAGMA: return "pragma";
        case HeaderStyle::ALPHABET: return "alphabet";
        case HeaderStyle::NONE: return "none";
    }
    return "prefix";
}

HeaderParseResult HeaderEngine::parse(const std::string& source,
                                     const HeaderConfig& config,
                                     const std::string& expected_lang) {
    HeaderParseResult result;
    result.detected_language = expected_lang;
    result.version = config.default_version;
    result.mode = config.default_mode;
    result.stripped_source = source;

    if (source.empty()) {
        if (config.required) {
            result.valid = false;
            result.error = "File is empty; expected header " + config.prefix;
        }
        return result;
    }

    std::istringstream stream(source);
    std::string first_line;
    int line_count = 0;
    size_t header_end_pos = 0;

    // Read the first non-empty line
    while (std::getline(stream, first_line)) {
        line_count++;
        header_end_pos = stream.tellg();
        std::string trimmed = trim(first_line);
        if (!trimmed.empty()) {
            break;
        }
    }

    std::string line = trim(first_line);
    if (line.empty()) {
        if (config.required) {
            result.valid = false;
            result.error = "Expected header with prefix " + config.prefix + " but found empty file";
        }
        return result;
    }

    bool matched = false;

    if (config.style == HeaderStyle::NONE) {
        result.has_header = false;
        result.stripped_source = source;
        result.header_lines = 0;
        return result;
    }

    if (config.style == HeaderStyle::PREFIX) {
        // e.g. #nova or #nova<1.0, compiled> or #nova(1.0)
        std::string pfx = config.prefix.empty() ? ("#" + to_lower(expected_lang)) : config.prefix;
        if (line.rfind(pfx, 0) == 0) {
            matched = true;
            result.has_header = true;
            std::string rest = trim(line.substr(pfx.length()));
            if (!rest.empty()) {
                // Parse delimiters <...>, (...), or [...]
                if ((rest.front() == '<' && rest.back() == '>') ||
                    (rest.front() == '(' && rest.back() == ')') ||
                    (rest.front() == '[' && rest.back() == ']')) {
                    std::string params = rest.substr(1, rest.length() - 2);
                    std::stringstream ss(params);
                    std::string item;
                    int idx = 0;
                    while (std::getline(ss, item, ',')) {
                        std::string p = trim(item);
                        if (idx == 0 && !p.empty()) result.version = p;
                        else if (idx == 1 && !p.empty()) result.mode = p;
                        idx++;
                    }
                }
            }
        }
    } else if (config.style == HeaderStyle::SHEBANG) {
        // e.g. #!/usr/bin/env nova or #!/usr/bin/nova
        if (line.rfind("#!", 0) == 0) {
            matched = true;
            result.has_header = true;
            size_t slash = line.find_last_of("/ \t");
            if (slash != std::string::npos && slash + 1 < line.length()) {
                result.detected_language = line.substr(slash + 1);
            }
        }
    } else if (config.style == HeaderStyle::PRAGMA) {
        // e.g. @nova(version=1.0) or @lang(nova)
        std::string pfx = config.prefix.empty() ? ("@" + to_lower(expected_lang)) : config.prefix;
        if (line.rfind(pfx, 0) == 0) {
            matched = true;
            result.has_header = true;
            size_t open_p = line.find('(');
            size_t close_p = line.rfind(')');
            if (open_p != std::string::npos && close_p != std::string::npos && close_p > open_p) {
                std::string inside = line.substr(open_p + 1, close_p - open_p - 1);
                std::stringstream ss(inside);
                std::string pair;
                while (std::getline(ss, pair, ',')) {
                    size_t eq = pair.find('=');
                    if (eq != std::string::npos) {
                        std::string k = trim(pair.substr(0, eq));
                        std::string v = trim(pair.substr(eq + 1));
                        if (k == "version") result.version = v;
                        else if (k == "mode") result.mode = v;
                        result.attributes[k] = v;
                    }
                }
            }
        }
    } else if (config.style == HeaderStyle::ALPHABET) {
        // e.g. #alphabet<nova> or #alphabet<nova, 1.0>
        if (line.rfind("#alphabet", 0) == 0) {
            matched = true;
            result.has_header = true;
            size_t open_b = line.find('<');
            size_t close_b = line.rfind('>');
            if (open_b != std::string::npos && close_b != std::string::npos && close_b > open_b) {
                std::string content = line.substr(open_b + 1, close_b - open_b - 1);
                std::stringstream ss(content);
                std::string item;
                int idx = 0;
                while (std::getline(ss, item, ',')) {
                    std::string p = trim(item);
                    if (idx == 0 && !p.empty()) result.detected_language = p;
                    else if (idx == 1 && !p.empty()) result.version = p;
                    else if (idx == 2 && !p.empty()) result.mode = p;
                    idx++;
                }
            }
        }
    }

    if (!matched && config.required) {
        result.valid = false;
        result.error = "Missing required " + style_to_string(config.style) +
                       " header (expected prefix: '" + config.prefix + "')";
        return result;
    }

    if (matched) {
        if (header_end_pos != std::string::npos && header_end_pos <= source.length()) {
            result.stripped_source = source.substr(header_end_pos);
        } else {
            result.stripped_source = "";
        }
        result.header_lines = line_count;
    }

    return result;
}

HeaderParseResult HeaderEngine::auto_detect(const std::string& source) {
    HeaderParseResult result;
    result.stripped_source = source;

    if (source.empty()) return result;

    std::istringstream stream(source);
    std::string first_line;
    size_t header_end_pos = 0;

    while (std::getline(stream, first_line)) {
        header_end_pos = stream.tellg();
        std::string trimmed = trim(first_line);
        if (!trimmed.empty()) {
            break;
        }
    }

    std::string line = trim(first_line);
    if (line.empty()) return result;

    // Check shebang: #!/usr/bin/env nova
    if (line.rfind("#!", 0) == 0) {
        result.has_header = true;
        size_t last_word = line.find_last_of("/ \t");
        if (last_word != std::string::npos && last_word + 1 < line.length()) {
            result.detected_language = line.substr(last_word + 1);
        }
    }
    // Check #alphabet<lang...>
    else if (line.rfind("#alphabet", 0) == 0) {
        result.has_header = true;
        size_t open_b = line.find('<');
        size_t close_b = line.rfind('>');
        if (open_b != std::string::npos && close_b != std::string::npos && close_b > open_b) {
            std::string content = line.substr(open_b + 1, close_b - open_b - 1);
            std::stringstream ss(content);
            std::string item;
            int idx = 0;
            while (std::getline(ss, item, ',')) {
                std::string p = trim(item);
                if (idx == 0) result.detected_language = p;
                else if (idx == 1) result.version = p;
                else if (idx == 2) result.mode = p;
                idx++;
            }
        }
    }
    // Check custom #prefix<...> or #prefix
    else if (line.front() == '#' && line.length() > 1 && std::isalpha(static_cast<unsigned char>(line[1]))) {
        result.has_header = true;
        size_t delim = line.find_first_of("<([ \t");
        if (delim != std::string::npos) {
            result.detected_language = line.substr(1, delim - 1);
            std::string rest = trim(line.substr(delim));
            if (!rest.empty() && ((rest.front() == '<' && rest.back() == '>') ||
                                  (rest.front() == '(' && rest.back() == ')'))) {
                std::string inner = rest.substr(1, rest.length() - 2);
                std::stringstream ss(inner);
                std::string p;
                int idx = 0;
                while (std::getline(ss, p, ',')) {
                    std::string t = trim(p);
                    if (idx == 0) result.version = t;
                    else if (idx == 1) result.mode = t;
                    idx++;
                }
            }
        } else {
            result.detected_language = line.substr(1);
        }
    }
    // Check @pragma
    else if (line.front() == '@' && line.length() > 1 && std::isalpha(static_cast<unsigned char>(line[1]))) {
        result.has_header = true;
        size_t delim = line.find_first_of("(<[ \t");
        if (delim != std::string::npos) {
            result.detected_language = line.substr(1, delim - 1);
        } else {
            result.detected_language = line.substr(1);
        }
    }

    if (result.has_header) {
        if (header_end_pos != std::string::npos && header_end_pos <= source.length()) {
            result.stripped_source = source.substr(header_end_pos);
        } else {
            result.stripped_source = "";
        }
    }

    return result;
}

} // namespace forge
} // namespace alphabet
