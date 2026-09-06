#include "banner_engine.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace alphabet {
namespace forge {

namespace {

const char* ANSI_RESET = "\033[0m";
const char* ANSI_RED = "\033[31m";
const char* ANSI_GREEN = "\033[32m";
const char* ANSI_YELLOW = "\033[33m";
const char* ANSI_BLUE = "\033[34m";
const char* ANSI_MAGENTA = "\033[35m";
const char* ANSI_CYAN = "\033[36m";
const char* ANSI_BOLD = "\033[1m";

const char* RAINBOW_COLORS[] = {
    "\033[31m", // red
    "\033[33m", // yellow
    "\033[32m", // green
    "\033[36m", // cyan
    "\033[34m", // blue
    "\033[35m"  // magenta
};

// 5-line standard ASCII font glyphs for letters A-Z, 0-9, and symbols
struct Glyph5 {
    char ch;
    const char* lines[5];
};

const Glyph5 FONT_STANDARD[] = {
    {'A', {"  /\\  ", " /  \\ ", "/ /\\ \\", "| |/ /", "|_| \\\\"}},
    {'B', {" ___  ", "| _ ) ", "| _ \\ ", "|___/ ", "|___/ "}},
    {'C', {"  ___ ", " / __|", "| (__ ", " \\___|", "      "}},
    {'D', {" ___  ", "|   \\ ", "| |) |", "|___/ ", "      "}},
    {'E', {" ___ ", "| __|", "| _| ", "|___|", "     "}},
    {'F', {" ___ ", "| __|", "| _| ", "|_|  ", "     "}},
    {'G', {"  ___ ", " / __|", "| (_ |", " \\___|", "      "}},
    {'H', {" _  _ ", "| || |", "| __ |", "|_||_|", "      "}},
    {'I', {" ___ ", "|_ _|", " | | ", "|___|", "     "}},
    {'J', {"   _ ", "  | |", " _| |", "|___/ ", "     "}},
    {'K', {" _  __", "| |/ /", "| ' < ", "|_|\\_\\", "      "}},
    {'L', {" _   ", "| |  ", "| |__", "|____|", "     "}},
    {'M', {" __  __ ", "|  \\/  |", "| |\\/| |", "|_|  |_|", "        "}},
    {'N', {" _  _ ", "| \\| |", "| .` |", "|_|\\_|", "      "}},
    {'O', {"  ___ ", " / _ \\", "| (_) |", " \\___/ ", "       "}},
    {'P', {" ___  ", "| _ \\ ", "|  _/ ", "|_|   ", "      "}},
    {'Q', {"  ___  ", " / _ \\ ", "| (_) |", " \\__\\_\\", "       "}},
    {'R', {" ___  ", "| _ \\ ", "|   / ", "|_|_\\ ", "      "}},
    {'S', {" ___ ", "/ __|", "\\__ \\", "|___/", "     "}},
    {'T', {" _____ ", "|_   _|", "  | |  ", "  |_|  ", "       "}},
    {'U', {" _   _ ", "| | | |", "| |_| |", " \\___/ ", "       "}},
    {'V', {"__   __", "\\ \\ / /", " \\ V / ", "  \\_/  ", "       "}},
    {'W', {"__      __", "\\ \\    / /", " \\ \\/\\/ / ", "  \\_/\\_/  ", "          "}},
    {'X', {"__  __", "\\ \\/ /", " >  < ", "/_/\\_\\", "      "}},
    {'Y', {"__   __", "\\ \\ / /", " \\ V / ", "  |_|  ", "       "}},
    {'Z', {" ____", "|_  /", " / / ", "/___|", "     "}},
    {'0', {"  ___ ", " / _ \\", "| | | |", "| |_| |", " \\___/ "}},
    {'1', {" _ ", "/ |", "| |", "|_|", "   "}},
    {'2', {" ___ ", "|_  )", " / / ", "/___|", "     "}},
    {'3', {" ____", "|__ /", " |_ \\", "|___/", "     "}},
    {'4', {" _ _  ", "| | | ", "|_  _|", "  |_| ", "      "}},
    {'5', {" ___ ", "| __|", "|__ \\", "|___/", "     "}},
    {'6', {"  __ ", " / / ", "/ _ \\", "\\___/", "     "}},
    {'7', {" ____ ", "|__  /", "  / / ", " /_/  ", "      "}},
    {'8', {" ___ ", "( _ )", "/ _ \\", "\\___/", "     "}},
    {'9', {" ___ ", "/ _ \\", "\\_, /", " /_/ ", "     "}},
    {'-', {"      ", " ____ ", "|____|", "      ", "      "}},
    {'_', {"      ", "      ", "      ", " _____", "|_____|"}},
    {' ', {"   ", "   ", "   ", "   ", "   "}}
};

const Glyph5* find_glyph(char c) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    for (const auto& g : FONT_STANDARD) {
        if (g.ch == upper) return &g;
    }
    return nullptr;
}

} // namespace

std::string BannerEngine::colorize(const std::string& text, const std::string& color_name) {
    std::string clr = color_name;
    std::transform(clr.begin(), clr.end(), clr.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (clr == "rainbow") {
        std::ostringstream oss;
        std::istringstream stream(text);
        std::string line;
        size_t c_idx = 0;
        while (std::getline(stream, line)) {
            oss << RAINBOW_COLORS[c_idx % 6] << line << ANSI_RESET << "\n";
            c_idx++;
        }
        return oss.str();
    }

    const char* code = ANSI_CYAN;
    if (clr == "red") code = ANSI_RED;
    else if (clr == "green") code = ANSI_GREEN;
    else if (clr == "yellow") code = ANSI_YELLOW;
    else if (clr == "blue") code = ANSI_BLUE;
    else if (clr == "magenta") code = ANSI_MAGENTA;
    else if (clr == "cyan") code = ANSI_CYAN;

    return std::string(code) + text + ANSI_RESET;
}

std::string BannerEngine::render_figlet(const std::string& text, const std::string& /*font_style*/) {
    std::string lines[5];
    for (char c : text) {
        const Glyph5* g = find_glyph(c);
        if (g) {
            for (int r = 0; r < 5; ++r) {
                lines[r] += g->lines[r];
                lines[r] += " ";
            }
        } else {
            // Default space for unknown character
            for (int r = 0; r < 5; ++r) {
                lines[r] += "   ";
            }
        }
    }

    std::ostringstream oss;
    for (int r = 0; r < 5; ++r) {
        // Only print line if not entirely empty
        if (!lines[r].empty()) {
            oss << lines[r] << "\n";
        }
    }
    return oss.str();
}

std::string BannerEngine::render_banner(const ForgeSpec& spec, const std::string& /*trigger*/) {
    std::ostringstream oss;

    std::string art = spec.banner.art;
    if (art.empty()) {
        art = render_figlet(spec.name, spec.banner.font);
    }

    // Colorize art
    oss << colorize(art, spec.banner.color);

    // Metadata line
    oss << ANSI_BOLD << spec.name << " v" << spec.version << ANSI_RESET;
    if (!spec.banner.tagline.empty()) {
        oss << " (" << spec.banner.tagline << ")";
    }
    oss << "\n";

    return oss.str();
}

void BannerEngine::print_if_enabled(const ForgeSpec& spec, const std::string& trigger, std::ostream& out) {
    bool enabled = false;
    for (const auto& item : spec.banner.show_on) {
        if (item == trigger || item == "all") {
            enabled = true;
            break;
        }
    }
    if (enabled) {
        out << render_banner(spec, trigger);
    }
}

} // namespace forge
} // namespace alphabet
