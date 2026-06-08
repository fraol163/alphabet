#include "alphabet_embed.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <iostream>
#include <sstream>

namespace alphabet {

Alphabet::Alphabet() : vm_(new VM()), sandbox_(false), language_("en") {}

Alphabet::~Alphabet() {
    delete vm_;
}

EmbedResult Alphabet::eval(const std::string& source) {
    return eval(source, language_);
}

EmbedResult Alphabet::eval(const std::string& source, const std::string& language) {
    EmbedResult result{false, "", "", 0.0};

    std::string full_source = "#alphabet<" + language + ">\n" + source;

    std::ostringstream captured_output;
    std::streambuf* old_cout = std::cout.rdbuf();

    if (output_handler_) {
        std::cout.rdbuf(captured_output.rdbuf());
    }

    try {
        Lexer lexer(full_source);
        auto tokens = lexer.scan_tokens();

        Parser parser(tokens);
        auto ast = parser.parse();

        Compiler compiler;
        Program program = compiler.compile(ast);

        vm_->init(program);
        vm_->set_sandbox_mode(sandbox_);
        vm_->run();

        result.success = true;
        result.output = captured_output.str();
    } catch (const std::exception& e) {
        result.error = e.what();
    }

    std::cout.rdbuf(old_cout);

    if (output_handler_ && !result.output.empty()) {
        output_handler_(result.output);
    }

    return result;
}

void Alphabet::set_output_handler(std::function<void(const std::string&)> handler) {
    output_handler_ = handler;
}

void Alphabet::set_sandbox(bool enabled) {
    sandbox_ = enabled;
}

void Alphabet::set_language(const std::string& lang) {
    language_ = lang;
}

double Alphabet::get_number(const std::string& var_name) {
    auto globals = vm_->get_globals();
    auto it = globals.find(var_name);
    if (it == globals.end()) {
        throw std::runtime_error("Variable '" + var_name + "' not found");
    }
    if (!it->second.is_number()) {
        throw std::runtime_error("Variable '" + var_name + "' is not a number");
    }
    return it->second.as_number();
}

std::string Alphabet::get_string(const std::string& var_name) {
    auto globals = vm_->get_globals();
    auto it = globals.find(var_name);
    if (it == globals.end()) {
        throw std::runtime_error("Variable '" + var_name + "' not found");
    }
    if (!it->second.is_string()) {
        throw std::runtime_error("Variable '" + var_name + "' is not a string");
    }
    return it->second.as_string();
}

bool Alphabet::has_variable(const std::string& var_name) {
    auto globals = vm_->get_globals();
    return globals.find(var_name) != globals.end();
}

EmbedResult alphabet_eval(const std::string& source) {
    Alphabet alpha;
    return alpha.eval(source);
}

EmbedResult alphabet_eval(const std::string& source, const std::string& language) {
    Alphabet alpha;
    return alpha.eval(source, language);
}

} // namespace alphabet
