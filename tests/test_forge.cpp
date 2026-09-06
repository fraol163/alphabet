#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "forge.h"
#include "header_engine.h"
#include "forge_spec.h"
#include "c_emitter.h"
#include "peg_engine.h"
#include "forge_validator.h"
#include "forge_ecosystem.h"
#include <filesystem>

using namespace alphabet;
using namespace alphabet::forge;

static std::string find_spec(const std::string& name) {
    if (std::filesystem::exists(name)) return name;
    if (std::filesystem::exists("../" + name)) return "../" + name;
    return name;
}

// ============================================================================
// Header Engine Tests
// ============================================================================

TEST_CASE("HeaderEngine validates prefix style", "[forge][header]") {
    HeaderConfig cfg;
    cfg.style = HeaderStyle::PREFIX;
    cfg.prefix = "#nova";

    std::string source = "#nova<1.0, compiled>\ncreate x = 10;";
    auto res = HeaderEngine::parse(source, cfg, "Nova");
    REQUIRE(res.valid);
    REQUIRE(res.has_header);
    REQUIRE(res.header_lines == 1);
    REQUIRE(res.stripped_source == "create x = 10;");
}

TEST_CASE("HeaderEngine validates shebang style", "[forge][header]") {
    HeaderConfig cfg;
    cfg.style = HeaderStyle::SHEBANG;
    cfg.prefix = "#!/usr/bin/env zen";

    std::string source = "#!/usr/bin/env zen\nlet a = 42;";
    auto res = HeaderEngine::parse(source, cfg, "Zen");
    REQUIRE(res.valid);
    REQUIRE(res.has_header);
    REQUIRE(res.header_lines == 1);
    REQUIRE(res.stripped_source == "let a = 42;");
}

TEST_CASE("HeaderEngine validates pragma style", "[forge][header]") {
    HeaderConfig cfg;
    cfg.style = HeaderStyle::PRAGMA;
    cfg.prefix = "@nova";

    std::string source = "@nova(version=1.0)\ncreate y = 5;";
    auto res = HeaderEngine::parse(source, cfg, "Nova");
    REQUIRE(res.valid);
    REQUIRE(res.has_header);
    REQUIRE(res.stripped_source == "create y = 5;");
}

TEST_CASE("HeaderEngine handles NONE style without stripping", "[forge][header]") {
    HeaderConfig cfg;
    cfg.style = HeaderStyle::NONE;

    std::string source = "create msg = \"Hello\";";
    auto res = HeaderEngine::parse(source, cfg, "Any");
    REQUIRE(res.valid);
    REQUIRE(res.header_lines == 0);
    REQUIRE(res.stripped_source == source);
}

// ============================================================================
// Forge Specification Parser Tests
// ============================================================================

TEST_CASE("ForgeSpec loads nova.forge correctly", "[forge][spec]") {
    ForgeSpec spec;
    std::string err;
    bool ok = ForgeSpec::load_from_file(find_spec("nova.forge"), spec, err);
    REQUIRE(ok);
    REQUIRE(spec.name == "Nova");
    REQUIRE(spec.version == "1.0.0");
    REQUIRE(spec.extension == ".nv");
    REQUIRE(spec.tokens.keywords.find("create") != spec.tokens.keywords.end());
    REQUIRE(spec.tokens.keywords.at("create") == "VAR");
    REQUIRE(spec.tokens.keywords.at("display") == "PRINT");
    REQUIRE(spec.tokens.keywords.at("condition") == "IF");
}

TEST_CASE("ForgeSpec loads zen.forge correctly", "[forge][spec]") {
    ForgeSpec spec;
    std::string err;
    bool ok = ForgeSpec::load_from_file(find_spec("zen.forge"), spec, err);
    REQUIRE(ok);
    REQUIRE(spec.name == "Zen");
    REQUIRE(spec.extension == ".zen");
    REQUIRE(spec.header.style == HeaderStyle::SHEBANG);
    REQUIRE(spec.tokens.keywords.at("let") == "VAR");
    REQUIRE(spec.tokens.keywords.at("echo") == "PRINT");
}

// ============================================================================
// PEG Parser Engine Tests
// ============================================================================

TEST_CASE("PegEngine parses variable declarations and expressions", "[forge][peg]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("nova.forge"), spec, err));

    PegEngine peg(spec);
    std::string code = "create x = 15;\ncreate y = x + 25;\ndisplay(y);";
    auto pres = peg.parse(code, "test.nv", 0);
    REQUIRE(pres.success);
    REQUIRE(pres.statements.size() == 3);
}

TEST_CASE("PegEngine reports syntax errors with line numbers", "[forge][peg]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("nova.forge"), spec, err));

    PegEngine peg(spec);
    std::string invalid_code = "create = 100;"; // Missing identifier
    auto pres = peg.parse(invalid_code, "test.nv", 0);
    REQUIRE_FALSE(pres.success);
    REQUIRE_FALSE(pres.errors.empty());
    REQUIRE(pres.errors[0].line >= 1);
}

// ============================================================================
// C Emitter Tests
// ============================================================================

TEST_CASE("ForgeCEmitter generates valid C source code", "[forge][c_emitter]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("nova.forge"), spec, err));

    PegEngine peg(spec);
    std::string code = "create val = 50;\ndisplay(val);";
    auto pres = peg.parse(code, "test.nv", 0);
    REQUIRE(pres.success);

    std::string c_out = ForgeCEmitter::emit_c(pres.statements, spec);
    REQUIRE_FALSE(c_out.empty());
    REQUIRE(c_out.find("#include <stdio.h>") != std::string::npos);
    REQUIRE(c_out.find("int main(") != std::string::npos);
    REQUIRE(c_out.find("val_add") != std::string::npos);
}

// ============================================================================
// End-to-End ForgeRunner Compilation & Execution Tests
// ============================================================================

TEST_CASE("ForgeRunner compiles and executes Nova program to bytecode", "[forge][runner]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("nova.forge"), spec, err));

    std::string source = "#nova<1.0, compiled>\ncreate a = 12;\ncreate b = 28;\ncreate sum = a + b;";
    alphabet::Program prog;
    std::vector<DiagnosticError> errors;
    bool ok = ForgeRunner::compile_source(source, spec, prog, errors, "test.nv");
    REQUIRE(ok);
    REQUIRE(errors.empty());
    REQUIRE_FALSE(prog.main.empty());

    // Execute in VM
    alphabet::VM vm(prog);
    vm.run();
    auto globals = vm.get_globals();
    REQUIRE(globals.find("a") != globals.end());
    REQUIRE(globals["a"].as_number() == 12);
    REQUIRE(globals.find("b") != globals.end());
    REQUIRE(globals["b"].as_number() == 28);
    REQUIRE(globals.find("sum") != globals.end());
    REQUIRE(globals["sum"].as_number() == 40);
}

TEST_CASE("ForgeRunner compiles and executes Lisp with pragma header", "[forge][lisp]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("lisp.forge"), spec, err));

    std::string source = "@lisp(version=1.0)\ndef x = 50;\ndef y = 2;\ndef result = x * y;\nwrite(\"Output: \" + result);";
    alphabet::Program prog;
    std::vector<DiagnosticError> errors;
    bool ok = ForgeRunner::compile_source(source, spec, prog, errors, "test.lsp");
    REQUIRE(ok);
    REQUIRE(errors.empty());

    alphabet::VM vm(prog);
    vm.run();
    auto globals = vm.get_globals();
    REQUIRE(globals.find("result") != globals.end());
    REQUIRE(globals["result"].as_number() == 100);
}

// ============================================================================
// Forge Specification & Grammar Validator Tests
// ============================================================================

TEST_CASE("ForgeValidator validates valid specs", "[forge][validator]") {
    ForgeSpec nova_spec, zen_spec, lisp_spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("nova.forge"), nova_spec, err));
    REQUIRE(ForgeSpec::load_from_file(find_spec("zen.forge"), zen_spec, err));
    REQUIRE(ForgeSpec::load_from_file(find_spec("lisp.forge"), lisp_spec, err));

    auto nova_res = ForgeValidator::validate(nova_spec);
    CHECK(nova_res.valid);
    CHECK(nova_res.error_count == 0);

    auto zen_res = ForgeValidator::validate(zen_spec);
    CHECK(zen_res.valid);
    CHECK(zen_res.error_count == 0);

    auto lisp_res = ForgeValidator::validate(lisp_spec);
    CHECK(lisp_res.valid);
    CHECK(lisp_res.error_count == 0);
}

TEST_CASE("ForgeValidator flags undefined non-terminals and left-recursion", "[forge][validator]") {
    // 1. Spec with undefined non-terminal
    std::string bad_spec_str = R"(
language Broken {
    version: "1.0.0"
    extension: ".bad"
    tokens {
        keyword "let" => VAR
    }
    grammar {
        rule Program = UndefinedRule*
    }
}
)";
    ForgeSpec bad_spec;
    std::string err;
    REQUIRE(ForgeSpec::parse_string(bad_spec_str, bad_spec, err));
    auto bad_res = ForgeValidator::validate(bad_spec);
    CHECK_FALSE(bad_res.valid);
    CHECK(bad_res.error_count > 0);

    // 2. Spec with direct left-recursion
    std::string rec_spec_str = R"(
language Recur {
    version: "1.0.0"
    extension: ".rec"
    tokens {
        keyword "let" => VAR
    }
    grammar {
        rule Program = Program "+" Identifier | Identifier
    }
}
)";
    ForgeSpec rec_spec;
    REQUIRE(ForgeSpec::parse_string(rec_spec_str, rec_spec, err));
    auto rec_res = ForgeValidator::validate(rec_spec);
    CHECK(rec_res.warning_count > 0);
}

// ============================================================================
// Scaffolding & Distribution Bundle Tests
// ============================================================================

TEST_CASE("ForgeEcosystem initializes new language project", "[forge][ecosystem]") {
    std::string temp_dir = "/tmp/test_init_lang";
    std::string err;
    bool ok = ForgeEcosystem::init_project("TestLang", temp_dir, "shebang", err);
    REQUIRE(ok);
    REQUIRE(std::filesystem::exists(temp_dir + "/TestLang.forge"));
    REQUIRE(std::filesystem::exists(temp_dir + "/examples/hello.tes"));
    REQUIRE(std::filesystem::exists(temp_dir + "/tests/test_basic.tes"));
    REQUIRE(std::filesystem::exists(temp_dir + "/README.md"));

    // Verify generated spec is valid
    ForgeSpec spec;
    REQUIRE(ForgeSpec::load_from_file(temp_dir + "/TestLang.forge", spec, err));
    auto val_res = ForgeValidator::validate(spec);
    CHECK(val_res.valid);
    CHECK(val_res.error_count == 0);

    // Clean up
    std::filesystem::remove_all(temp_dir);
}

TEST_CASE("ForgeEcosystem creates complete distribution bundle", "[forge][ecosystem]") {
    ForgeSpec spec;
    std::string err;
    REQUIRE(ForgeSpec::load_from_file(find_spec("zen.forge"), spec, err));

    std::string temp_bundle = "/tmp/test_zen_bundle";
    bool ok = ForgeEcosystem::bundle_distribution(spec, temp_bundle, err);
    REQUIRE(ok);
    REQUIRE(std::filesystem::exists(temp_bundle + "/bin/zen"));
    REQUIRE(std::filesystem::exists(temp_bundle + "/Zen.forge"));
    REQUIRE(std::filesystem::exists(temp_bundle + "/vscode/package.json"));
    REQUIRE(std::filesystem::exists(temp_bundle + "/docs/index.html"));
    REQUIRE(std::filesystem::exists(temp_bundle + "/playground/index.html"));
    REQUIRE(std::filesystem::exists(temp_bundle + "/README.md"));

    // Clean up
    std::filesystem::remove_all(temp_bundle);
    std::filesystem::remove(temp_bundle + ".tar.gz");
}
