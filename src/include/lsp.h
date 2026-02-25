#ifndef ALPHABET_LSP_H
#define ALPHABET_LSP_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace alphabet {
namespace lsp {

struct Position {
    int line = 0;
    int character = 0;
};

struct Range {
    Position start;
    Position end;
};

struct Diagnostic {
    Range range;
    int severity = 1;
    std::string code;
    std::string source = "alphabet-compiler";
    std::string message;
};

struct CompletionItem {
    std::string label;
    std::string kind;
    std::string detail;
    std::string documentation;
};

class LanguageServer {
public:
    LanguageServer();

    std::string process_message(const std::string& json);
    std::vector<Diagnostic> get_diagnostics(const std::string& uri, const std::string& content);
    std::vector<CompletionItem> get_completions(const std::string& uri, Position position);
    void run();

private:
    std::unordered_map<std::string, std::string> documents_;
    std::vector<CompletionItem> keywords_;
    std::vector<CompletionItem> types_;

    void initialize_completions();
    std::string create_response(int id, const std::string& result);
    std::string create_error(int id, int code, const std::string& message);
    void send_response(const std::string& json);
    std::string handle_initialize(int id, const std::string& params);
    std::string handle_did_open(const std::string& params);
    std::string handle_did_change(const std::string& params);
    std::string handle_completion(int id, const std::string& params);
    std::string handle_hover(int id, const std::string& params);
};

std::string extract_json_string(const std::string& json, const std::string& key);
int extract_json_int(const std::string& json, const std::string& key, int default_val = 0);

}
}

#endif
