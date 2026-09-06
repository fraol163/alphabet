#ifndef ALPHABET_FORGE_VALIDATOR_H
#define ALPHABET_FORGE_VALIDATOR_H

#include "forge_spec.h"
#include <string>
#include <vector>
#include <ostream>

namespace alphabet {
namespace forge {

enum class IssueSeverity {
    INFO,
    WARNING,
    ERROR
};

struct ValidationIssue {
    IssueSeverity severity;
    std::string category;     // "metadata", "header", "tokens", "grammar", "banner"
    std::string item_name;    // rule name, keyword, or attribute
    std::string message;
    std::string suggestion;
};

struct ValidationResult {
    bool valid = true;
    int error_count = 0;
    int warning_count = 0;
    int info_count = 0;
    std::vector<ValidationIssue> issues;

    void add_issue(IssueSeverity sev, const std::string& cat, const std::string& item,
                   const std::string& msg, const std::string& sug = "") {
        issues.push_back({sev, cat, item, msg, sug});
        if (sev == IssueSeverity::ERROR) {
            error_count++;
            valid = false;
        } else if (sev == IssueSeverity::WARNING) {
            warning_count++;
        } else {
            info_count++;
        }
    }
};

class ForgeValidator {
public:
    static ValidationResult validate(const ForgeSpec& spec);
    static ValidationResult validate_file(const std::string& path, ForgeSpec& out_spec);
    static void print_report(const ValidationResult& result, const ForgeSpec& spec, std::ostream& out);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_VALIDATOR_H
