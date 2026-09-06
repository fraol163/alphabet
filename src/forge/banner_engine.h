#ifndef ALPHABET_FORGE_BANNER_ENGINE_H
#define ALPHABET_FORGE_BANNER_ENGINE_H

#include "forge_spec.h"
#include <string>
#include <ostream>

namespace alphabet {
namespace forge {

class BannerEngine {
public:
    // Colorize a string with the specified color name (cyan, red, green, yellow, blue, magenta, rainbow)
    static std::string colorize(const std::string& text, const std::string& color_name);

    // Generate ASCII art for a text string using embedded FIGlet font
    static std::string render_figlet(const std::string& text, const std::string& font_style = "standard");

    // Render full banner (art + metadata + tagline) to a string or stream
    static std::string render_banner(const ForgeSpec& spec, const std::string& trigger = "repl");

    // Print banner if the trigger is enabled in banner.show_on
    static void print_if_enabled(const ForgeSpec& spec, const std::string& trigger, std::ostream& out);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_BANNER_ENGINE_H
