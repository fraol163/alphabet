#include "forge_linter.h"
#include "peg_engine.h"
#include "header_engine.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

namespace alphabet {
namespace forge {

std::string LintWarning::format() const {
    std::ostringstream ss;
    if (is_error) {
        ss << "\033[1;31merror\033[0m: " << message << "\n";
    } else {
        ss << "\033[1;33mwarning\033[0m: " << message << "\n";
    }
    ss << "  \033[1;34m-->\033[0m " << filepath << ":" << line << ":" << column << "\n";
    if (!line_content.empty()) {
        ss << "   \033[1;34m|\033[0m\n";
        ss << " " << line << " \033[1;34m|\033[0m " << line_content << "\n";
        ss << "   \033[1;34m|\033[0m ";
        for (int i = 1; i < column; ++i) ss << " ";
        if (is_error) {
            ss << "\033[1;31m^\033[0m";
        } else {
            ss << "\033[1;33m^\033[0m";
        }
        if (!hint.empty()) {
            ss << " \033[1;36mhelp: " << hint << "\033[0m";
        }
        ss << "\n";
    }
    return ss.str();
}

namespace {

struct VarInfo {
    std::string name;
    int line;
    int column;
    std::string line_content;
    bool used = false;
};

class LintAstVisitor {
public:
    explicit LintAstVisitor(const std::string& src, const std::string& fname)
        : source_(src), filename_(fname) {}

    void analyze(const std::vector<alphabet::StmtPtr>& stmts, LintReport& report) {
        for (const auto& s : stmts) {
            visit_stmt(s);
        }

        // Check unused variables
        for (const auto& kv : declared_vars_) {
            if (!kv.second.used && kv.second.name != "_" && kv.second.name != "z") {
                LintWarning w;
                w.filepath = filename_;
                w.line = kv.second.line;
                w.column = kv.second.column;
                w.line_content = kv.second.line_content;
                w.message = "variable '" + kv.second.name + "' is declared but never used";
                w.hint = "if intentionally unused, prefix with '_' or remove declaration";
                w.is_error = false;
                report.warnings.push_back(w);
                report.warning_count++;
            }
        }
    }

private:
    std::string source_;
    std::string filename_;
    std::unordered_map<std::string, VarInfo> declared_vars_;

    std::string get_line_str(int line_num) const {
        std::istringstream stream(source_);
        std::string l;
        int cur = 1;
        while (std::getline(stream, l)) {
            if (cur == line_num) {
                if (!l.empty() && l.back() == '\r') l.pop_back();
                return l;
            }
            cur++;
        }
        return "";
    }

    void visit_stmt(const alphabet::StmtPtr& stmt) {
        if (!stmt) return;

        if (auto* var_stmt = dynamic_cast<alphabet::VarStmt*>(stmt.get())) {
            std::string name(var_stmt->name.lexeme);
            VarInfo vi;
            vi.name = name;
            vi.line = static_cast<int>(var_stmt->name.line);
            vi.column = static_cast<int>(var_stmt->name.column);
            vi.line_content = get_line_str(vi.line);
            vi.used = false;
            declared_vars_[name] = vi;

            if (var_stmt->initializer) {
                visit_expr(var_stmt->initializer);
            }
            return;
        }

        if (auto* expr_stmt = dynamic_cast<alphabet::ExpressionStmt*>(stmt.get())) {
            visit_expr(expr_stmt->expression);
            return;
        }

        if (auto* if_stmt = dynamic_cast<alphabet::IfStmt*>(stmt.get())) {
            visit_expr(if_stmt->condition);
            visit_stmt(if_stmt->then_branch);
            if (if_stmt->else_branch) visit_stmt(if_stmt->else_branch);
            return;
        }

        if (auto* loop = dynamic_cast<alphabet::LoopStmt*>(stmt.get())) {
            visit_expr(loop->condition);
            visit_stmt(loop->body);
            return;
        }

        if (auto* blk = dynamic_cast<alphabet::Block*>(stmt.get())) {
            for (const auto& s : blk->statements) {
                visit_stmt(s);
            }
            return;
        }

        if (auto* fn = dynamic_cast<alphabet::FunctionStmt*>(stmt.get())) {
            for (const auto& s : fn->body) {
                visit_stmt(s);
            }
            return;
        }

        if (auto* ret = dynamic_cast<alphabet::ReturnStmt*>(stmt.get())) {
            if (ret->value) visit_expr(ret->value);
            return;
        }
    }

    void visit_expr(const alphabet::ExprPtr& expr) {
        if (!expr) return;

        if (auto* var = dynamic_cast<alphabet::Variable*>(expr.get())) {
            std::string name(var->name.lexeme);
            auto it = declared_vars_.find(name);
            if (it != declared_vars_.end()) {
                it->second.used = true;
            }
            return;
        }

        if (auto* assign = dynamic_cast<alphabet::Assign*>(expr.get())) {
            visit_expr(assign->value);
            return;
        }

        if (auto* bin = dynamic_cast<alphabet::Binary*>(expr.get())) {
            visit_expr(bin->left);
            visit_expr(bin->right);
            return;
        }

        if (auto* un = dynamic_cast<alphabet::Unary*>(expr.get())) {
            visit_expr(un->right);
            return;
        }

        if (auto* grp = dynamic_cast<alphabet::Grouping*>(expr.get())) {
            visit_expr(grp->expression);
            return;
        }

        if (auto* call = dynamic_cast<alphabet::Call*>(expr.get())) {
            visit_expr(call->callee);
            for (const auto& arg : call->arguments) {
                visit_expr(arg);
            }
            return;
        }

        if (auto* get = dynamic_cast<alphabet::Get*>(expr.get())) {
            visit_expr(get->obj);
            return;
        }
    }
};

} // namespace

LintReport ForgeLinter::lint_source(const std::string& source, const ForgeSpec& spec, const std::string& filename) {
    LintReport report;

    // 1. Check header
    HeaderParseResult hdr = HeaderEngine::parse(source, spec.header, spec.name);
    if (!hdr.valid) {
        LintWarning w;
        w.filepath = filename;
        w.line = 1;
        w.column = 1;
        w.message = hdr.error;
        w.is_error = true;
        report.warnings.push_back(w);
        report.error_count++;
        return report;
    }

    // 2. Parse PEG grammar
    PegEngine peg(spec);
    ParseResult parse_res = peg.parse(hdr.stripped_source, filename);
    if (!parse_res.success) {
        for (const auto& err : parse_res.errors) {
            LintWarning w;
            w.filepath = filename;
            w.line = err.line;
            w.column = err.column;
            w.line_content = err.line_content;
            w.message = err.message;
            w.hint = err.hint;
            w.is_error = true;
            report.warnings.push_back(w);
            report.error_count++;
        }
        return report;
    }

    // 3. Analyze AST for warnings
    LintAstVisitor visitor(hdr.stripped_source, filename);
    visitor.analyze(parse_res.statements, report);

    return report;
}

LintReport ForgeLinter::lint_file(const std::string& filepath, const ForgeSpec& spec) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LintReport report;
        LintWarning w;
        w.filepath = filepath;
        w.message = "Could not open file: " + filepath;
        w.is_error = true;
        report.warnings.push_back(w);
        report.error_count++;
        return report;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return lint_source(buffer.str(), spec, filepath);
}

} // namespace forge
} // namespace alphabet
