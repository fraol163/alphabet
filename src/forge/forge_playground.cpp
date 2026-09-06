#include "forge_playground.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace alphabet {
namespace forge {

namespace {

std::string escape_html(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '<') res += "&lt;";
        else if (c == '>') res += "&gt;";
        else if (c == '&') res += "&amp;";
        else if (c == '"') res += "&quot;";
        else res += c;
    }
    return res;
}

} // namespace

bool ForgePlayground::export_playground(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error) {
    std::filesystem::path dir(output_dir.empty() ? "playground" : output_dir);
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        out_error = "Failed to create directory: " + ec.message();
        return false;
    }

    std::filesystem::path out_file = dir / "index.html";
    std::ofstream out(out_file);
    if (!out.is_open()) {
        out_error = "Could not write to: " + out_file.string();
        return false;
    }

    std::string ext = spec.extension.empty() ? ".lang" : spec.extension;

    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "  <meta charset=\"UTF-8\">\n";
    out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "  <title>" << escape_html(spec.name) << " Web Playground</title>\n";
    out << "  <style>\n";
    out << "    :root {\n";
    out << "      --bg-dark: #0d1117;\n";
    out << "      --panel-bg: #161b22;\n";
    out << "      --border: #30363d;\n";
    out << "      --text: #c9d1d9;\n";
    out << "      --accent: #58a6ff;\n";
    out << "      --green: #238636;\n";
    out << "      --green-hover: #2ea043;\n";
    out << "    }\n";
    out << "    * { box-sizing: border-box; margin: 0; padding: 0; }\n";
    out << "    body {\n";
    out << "      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\n";
    out << "      background: var(--bg-dark);\n";
    out << "      color: var(--text);\n";
    out << "      height: 100vh;\n";
    out << "      display: flex;\n";
    out << "      flex-direction: column;\n";
    out << "    }\n";
    out << "    header {\n";
    out << "      background: var(--panel-bg);\n";
    out << "      border-bottom: 1px solid var(--border);\n";
    out << "      padding: 12px 24px;\n";
    out << "      display: flex;\n";
    out << "      align-items: center;\n";
    out << "      justify-content: space-between;\n";
    out << "    }\n";
    out << "    .logo {\n";
    out << "      font-size: 1.2rem;\n";
    out << "      font-weight: bold;\n";
    out << "      color: #fff;\n";
    out << "      display: flex;\n";
    out << "      align-items: center;\n";
    out << "      gap: 10px;\n";
    out << "    }\n";
    out << "    .tagline { color: #8b949e; font-size: 0.9rem; font-weight: normal; }\n";
    out << "    .controls {\n";
    out << "      display: flex;\n";
    out << "      gap: 12px;\n";
    out << "      align-items: center;\n";
    out << "    }\n";
    out << "    button.run-btn {\n";
    out << "      background: var(--green);\n";
    out << "      color: white;\n";
    out << "      border: none;\n";
    out << "      padding: 8px 18px;\n";
    out << "      border-radius: 6px;\n";
    out << "      font-weight: 600;\n";
    out << "      cursor: pointer;\n";
    out << "      display: flex;\n";
    out << "      align-items: center;\n";
    out << "      gap: 6px;\n";
    out << "    }\n";
    out << "    button.run-btn:hover { background: var(--green-hover); }\n";
    out << "    select {\n";
    out << "      background: var(--bg-dark);\n";
    out << "      color: var(--text);\n";
    out << "      border: 1px solid var(--border);\n";
    out << "      padding: 6px 12px;\n";
    out << "      border-radius: 6px;\n";
    out << "    }\n";
    out << "    .workspace {\n";
    out << "      display: flex;\n";
    out << "      flex: 1;\n";
    out << "      overflow: hidden;\n";
    out << "    }\n";
    out << "    .editor-pane, .output-pane {\n";
    out << "      flex: 1;\n";
    out << "      display: flex;\n";
    out << "      flex-direction: column;\n";
    out << "    }\n";
    out << "    .editor-pane {\n";
    out << "      border-right: 1px solid var(--border);\n";
    out << "    }\n";
    out << "    .pane-header {\n";
    out << "      background: #1c2128;\n";
    out << "      padding: 8px 16px;\n";
    out << "      font-size: 0.85rem;\n";
    out << "      color: #8b949e;\n";
    out << "      border-bottom: 1px solid var(--border);\n";
    out << "    }\n";
    out << "    textarea.code-editor {\n";
    out << "      flex: 1;\n";
    out << "      background: var(--bg-dark);\n";
    out << "      color: #79c0ff;\n";
    out << "      border: none;\n";
    out << "      padding: 16px;\n";
    out << "      font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;\n";
    out << "      font-size: 14px;\n";
    out << "      line-height: 1.5;\n";
    out << "      resize: none;\n";
    out << "      outline: none;\n";
    out << "    }\n";
    out << "    pre.terminal {\n";
    out << "      flex: 1;\n";
    out << "      background: #040d21;\n";
    out << "      color: #56d364;\n";
    out << "      padding: 16px;\n";
    out << "      font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;\n";
    out << "      font-size: 14px;\n";
    out << "      overflow-y: auto;\n";
    out << "      white-space: pre-wrap;\n";
    out << "    }\n";
    out << "  </style>\n";
    out << "</head>\n";
    out << "<body>\n";
    out << "  <header>\n";
    out << "    <div class=\"logo\">\n";
    out << "      <span>" << escape_html(spec.name) << " Playground</span>\n";
    out << "      <span class=\"tagline\">v" << escape_html(spec.version) << " (" << escape_html(spec.paradigm) << ")</span>\n";
    out << "    </div>\n";
    out << "    <div class=\"controls\">\n";
    out << "      <select id=\"sampleSelect\" onchange=\"loadSample()\">\n";
    out << "        <option value=\"hello\">Hello World</option>\n";
    out << "        <option value=\"calc\">Variables & Math</option>\n";
    out << "        <option value=\"flow\">Control Flow</option>\n";
    out << "      </select>\n";
    out << "      <button class=\"run-btn\" onclick=\"executeCode()\">Run</button>\n";
    out << "    </div>\n";
    out << "  </header>\n\n";
    out << "  <div class=\"workspace\">\n";
    out << "    <div class=\"editor-pane\">\n";
    out << "      <div class=\"pane-header\">Source (main" << escape_html(ext) << ")</div>\n";
    out << "      <textarea id=\"editor\" class=\"code-editor\" spellcheck=\"false\"></textarea>\n";
    out << "    </div>\n";
    out << "    <div class=\"output-pane\">\n";
    out << "      <div class=\"pane-header\">Console Output</div>\n";
    out << "      <pre id=\"output\" class=\"terminal\">Click 'Run' to execute your program...</pre>\n";
    out << "    </div>\n";
    out << "  </div>\n\n";

    std::string var_kw = "let";
    std::string print_kw = "print";
    std::string if_kw = "if";
    std::string else_kw = "else";
    std::string while_kw = "while";

    for (const auto& [kw, target] : spec.tokens.keywords) {
        if (target == "VAR" || target == "LET") var_kw = kw;
        else if (target == "PRINT") print_kw = kw;
        else if (target == "IF") if_kw = kw;
        else if (target == "ELSE") else_kw = kw;
        else if (target == "WHILE") while_kw = kw;
    }

    out << "  <script>\n";

    out << "    const samples = {\n";
    out << "      hello: '#" << escape_html(spec.name) << "<1.0>\\n\\n" << var_kw << " message = \"Hello from " << escape_html(spec.name) << "!\";\\n" << print_kw << "(message);\\n',\n";
    out << "      calc: '#" << escape_html(spec.name) << "<1.0>\\n\\n" << var_kw << " a = 12;\\n" << var_kw << " b = 28;\\n" << var_kw << " sum = a + b;\\n" << print_kw << "(\"Sum:\");\\n" << print_kw << "(sum);\\n',\n";
    out << "      flow: '#" << escape_html(spec.name) << "<1.0>\\n\\n" << var_kw << " count = 0;\\n" << while_kw << " (count < 3) {\\n    count = count + 1;\\n    " << print_kw << "(count);\\n}\\n" << if_kw << " (count == 3) {\\n    " << print_kw << "(\"Done!\");\\n}\\n'\n";
    out << "    };\n\n";
    out << "    function loadSample() {\n";
    out << "      const key = document.getElementById('sampleSelect').value;\n";
    out << "      document.getElementById('editor').value = samples[key] || '';\n";
    out << "    }\n\n";
    out << "    const SPEC = {\n";
    out << "      name: " << "\"" << escape_html(spec.name) << "\",\n";
    out << "      headerPrefix: " << "\"" << escape_html(spec.header.prefix) << "\",\n";
    out << "      varKw: " << "\"" << escape_html(var_kw) << "\",\n";
    out << "      printKw: " << "\"" << escape_html(print_kw) << "\",\n";
    out << "      ifKw: " << "\"" << escape_html(if_kw) << "\",\n";
    out << "      elseKw: " << "\"" << escape_html(else_kw) << "\",\n";
    out << "      whileKw: " << "\"" << escape_html(while_kw) << "\"\n";
    out << "    };\n\n";
    out << "    function executeCode() {\n";
    out << "      const raw = document.getElementById('editor').value;\n";
    out << "      const out = document.getElementById('output');\n";
    out << "      out.textContent = '';\n";
    out << "      const append = (msg) => { out.textContent += msg + '\\n'; };\n\n";
    out << "      let lines = raw.split('\\n');\n";
    out << "      let startLine = 0;\n";
    out << "      if (SPEC.headerPrefix && SPEC.headerPrefix.length > 0) {\n";
    out << "        while (startLine < lines.length && lines[startLine].trim() === '') startLine++;\n";
    out << "        if (startLine >= lines.length || !lines[startLine].startsWith(SPEC.headerPrefix)) {\n";
    out << "          append('Header Error: Missing required language header \"' + SPEC.headerPrefix + '\"');\n";
    out << "          return;\n";
    out << "        }\n";
    out << "        startLine++;\n";
    out << "      }\n";
    out << "      const code = lines.slice(startLine).join('\\n');\n\n";
    out << "      try {\n";
    out << "        const scope = {};\n";
    out << "        function evalExpr(expr) {\n";
    out << "          expr = expr.trim();\n";
    out << "          if (/^\"(.*)\"$/.test(expr)) return expr.slice(1, -1);\n";
    out << "          if (/^-?\\d+(\\.\\d+)?$/.test(expr)) return parseFloat(expr);\n";
    out << "          if (expr === 'true') return true;\n";
    out << "          if (expr === 'false') return false;\n";
    out << "          if (expr === 'null' || expr === 'nil') return null;\n";
    out << "          if (scope.hasOwnProperty(expr)) return scope[expr];\n";
    out << "          // Binary operations\n";
    out << "          const binMatch = expr.match(/^(.+?)\\s*(==|!=|<=|>=|<|>|\\+|\\-|\\*|\\/)\\s*(.+)$/);\n";
    out << "          if (binMatch) {\n";
    out << "            const left = evalExpr(binMatch[1]);\n";
    out << "            const op = binMatch[2];\n";
    out << "            const right = evalExpr(binMatch[3]);\n";
    out << "            if (op === '+') return left + right;\n";
    out << "            if (op === '-') return left - right;\n";
    out << "            if (op === '*') return left * right;\n";
    out << "            if (op === '/') return left / right;\n";
    out << "            if (op === '==') return left == right;\n";
    out << "            if (op === '!=') return left != right;\n";
    out << "            if (op === '<') return left < right;\n";
    out << "            if (op === '<=') return left <= right;\n";
    out << "            if (op === '>') return left > right;\n";
    out << "            if (op === '>=') return left >= right;\n";
    out << "          }\n";
    out << "          return expr;\n";
    out << "        }\n\n";
    out << "        function runBlock(src) {\n";
    out << "          const stmts = src.split(';').map(s => s.trim()).filter(s => s.length > 0);\n";
    out << "          for (const stmt of stmts) {\n";
    out << "            if (stmt.startsWith(SPEC.varKw + ' ')) {\n";
    out << "              const rest = stmt.slice(SPEC.varKw.length + 1);\n";
    out << "              const eq = rest.indexOf('=');\n";
    out << "              const name = rest.slice(0, eq).trim();\n";
    out << "              const val = evalExpr(rest.slice(eq + 1));\n";
    out << "              scope[name] = val;\n";
    out << "            } else if (stmt.startsWith(SPEC.printKw + '(') && stmt.endsWith(')')) {\n";
    out << "              const inner = stmt.slice(SPEC.printKw.length + 1, -1);\n";
    out << "              const res = evalExpr(inner);\n";
    out << "              append(res === null ? 'nil' : res.toString());\n";
    out << "            } else if (stmt.indexOf('=') !== -1) {\n";
    out << "              const eq = stmt.indexOf('=');\n";
    out << "              const name = stmt.slice(0, eq).trim();\n";
    out << "              if (scope.hasOwnProperty(name)) {\n";
    out << "                scope[name] = evalExpr(stmt.slice(eq + 1));\n";
    out << "              }\n";
    out << "            }\n";
    out << "          }\n";
    out << "        }\n\n";
    out << "        // Handle loops and conditionals\n";
    out << "        let remaining = code;\n";
    out << "        let safety = 0;\n";
    out << "        while (remaining.trim().length > 0 && safety++ < 10000) {\n";
    out << "          remaining = remaining.trim();\n";
    out << "          if (remaining.startsWith(SPEC.whileKw)) {\n";
    out << "            const pOpen = remaining.indexOf('(');\n";
    out << "            const pClose = remaining.indexOf(')', pOpen);\n";
    out << "            const bOpen = remaining.indexOf('{', pClose);\n";
    out << "            let bClose = -1, depth = 0;\n";
    out << "            for (let i = bOpen; i < remaining.length; i++) {\n";
    out << "              if (remaining[i] === '{') depth++;\n";
    out << "              else if (remaining[i] === '}') { depth--; if (depth === 0) { bClose = i; break; } }\n";
    out << "            }\n";
    out << "            const cond = remaining.slice(pOpen + 1, pClose);\n";
    out << "            const body = remaining.slice(bOpen + 1, bClose);\n";
    out << "            let loopSafety = 0;\n";
    out << "            while (evalExpr(cond) && loopSafety++ < 50000) {\n";
    out << "              runBlock(body);\n";
    out << "            }\n";
    out << "            remaining = remaining.slice(bClose + 1);\n";
    out << "          } else if (remaining.startsWith(SPEC.ifKw)) {\n";
    out << "            const pOpen = remaining.indexOf('(');\n";
    out << "            const pClose = remaining.indexOf(')', pOpen);\n";
    out << "            const bOpen = remaining.indexOf('{', pClose);\n";
    out << "            let bClose = -1, depth = 0;\n";
    out << "            for (let i = bOpen; i < remaining.length; i++) {\n";
    out << "              if (remaining[i] === '{') depth++;\n";
    out << "              else if (remaining[i] === '}') { depth--; if (depth === 0) { bClose = i; break; } }\n";
    out << "            }\n";
    out << "            const cond = remaining.slice(pOpen + 1, pClose);\n";
    out << "            const body = remaining.slice(bOpen + 1, bClose);\n";
    out << "            remaining = remaining.slice(bClose + 1).trim();\n";
    out << "            let elseBody = null;\n";
    out << "            if (remaining.startsWith(SPEC.elseKw)) {\n";
    out << "              const ebOpen = remaining.indexOf('{');\n";
    out << "              let ebClose = -1, edepth = 0;\n";
    out << "              for (let i = ebOpen; i < remaining.length; i++) {\n";
    out << "                if (remaining[i] === '{') edepth++;\n";
    out << "                else if (remaining[i] === '}') { edepth--; if (edepth === 0) { ebClose = i; break; } }\n";
    out << "              }\n";
    out << "              elseBody = remaining.slice(ebOpen + 1, ebClose);\n";
    out << "              remaining = remaining.slice(ebClose + 1);\n";
    out << "            }\n";
    out << "            if (evalExpr(cond)) runBlock(body);\n";
    out << "            else if (elseBody) runBlock(elseBody);\n";
    out << "          } else {\n";
    out << "            const semi = remaining.indexOf(';');\n";
    out << "            if (semi !== -1) {\n";
    out << "              runBlock(remaining.slice(0, semi + 1));\n";
    out << "              remaining = remaining.slice(semi + 1);\n";
    out << "            } else {\n";
    out << "              runBlock(remaining);\n";
    out << "              break;\n";
    out << "            }\n";
    out << "          }\n";
    out << "        }\n";
    out << "      } catch (err) {\n";
    out << "        append('Runtime Error: ' + err.message);\n";
    out << "      }\n";
    out << "    }\n\n";
    out << "    loadSample();\n";
    out << "    document.addEventListener('keydown', (e) => { if (e.ctrlKey && e.key === 'Enter') executeCode(); });\n";
    out << "  </script>\n";
    out << "</body>\n";
    out << "</html>\n";
    out.close();

    std::cout << "\033[1;32mExported Web Playground:\033[0m " << out_file.string() << "\n";
    return true;
}

} // namespace forge
} // namespace alphabet
