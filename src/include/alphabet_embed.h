#pragma once

#include <functional>
#include <stdexcept>
#include <string>

namespace alphabet {

class VM;
struct Program;

struct EmbedResult {
    bool success;
    std::string output;
    std::string error;
    double numeric_value;
};

class Alphabet {
  public:
    Alphabet();
    ~Alphabet();

    Alphabet(const Alphabet&) = delete;
    Alphabet& operator=(const Alphabet&) = delete;

    EmbedResult eval(const std::string& source);
    EmbedResult eval(const std::string& source, const std::string& language);

    void set_output_handler(std::function<void(const std::string&)> handler);
    void set_sandbox(bool enabled);
    void set_language(const std::string& lang);

    double get_number(const std::string& var_name);
    std::string get_string(const std::string& var_name);
    bool has_variable(const std::string& var_name);

  private:
    VM* vm_;
    bool sandbox_;
    std::string language_;
    std::function<void(const std::string&)> output_handler_;
};

EmbedResult alphabet_eval(const std::string& source);
EmbedResult alphabet_eval(const std::string& source, const std::string& language);

} // namespace alphabet
