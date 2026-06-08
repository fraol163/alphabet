#include "alphabet_embed.h"
#include <iostream>

int main() {
    alphabet::Alphabet alpha;

    auto result = alpha.eval("5 x = 42\nz.o(\"Answer: \" + z.tostr(x))\nx");
    if (result.success) {
        std::cout << "Output: " << result.output;
        std::cout << "Value: " << result.numeric_value << "\n";
    } else {
        std::cerr << "Error: " << result.error << "\n";
    }

    alpha.set_language("en");
    auto r2 = alpha.eval("5 sum = 0\nl (5 i = 1 : i <= 100 : i = i + 1) {\n  sum = sum + i\n}\nsum");
    if (r2.success) {
        std::cout << "Sum 1..100 = " << r2.numeric_value << "\n";
    }

    alpha.set_sandbox(true);
    auto r3 = alpha.eval("z.exec(\"rm -rf /\")");
    if (!r3.success) {
        std::cout << "Sandbox blocked exec: " << r3.error << "\n";
    }

    std::string output;
    alpha.set_output_handler([&output](const std::string &s) {
        output += s;
    });
    alpha.eval("z.o(\"captured!\")");
    std::cout << "Captured: " << output;

    return 0;
}
