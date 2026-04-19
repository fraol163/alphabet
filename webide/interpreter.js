const TYPES = {
    '0': 'void', '1': 'i8', '2': 'i16', '3': 'i32', '4': 'i64',
    '5': 'int', '6': 'f32', '7': 'f64', '8': 'float',
    '9': 'dec', '10': 'cpx', '11': 'bool', '12': 'str',
    '13': 'list', '14': 'map'
};

class AlphabetInterpreter {
    constructor() {
        this.globals = {};
        this.functions = {};
        this.output = [];
        this.inputCallback = null;
    }

    reset() {
        this.globals = {};
        this.functions = {};
        this.output = [];
    }

    run(source) {
        this.reset();
        try {
            const lines = this.preprocess(source);
            this.pass1(lines);
            this.pass2(lines);
            return { success: true, output: this.output.join('\n') };
        } catch (e) {
            return { success: false, error: e.message, output: this.output.join('\n') };
        }
    }

    preprocess(source) {
        const raw = source.split('\n');
        if (raw.length === 0) throw new Error('Empty source');
        const header = raw[0].trim();
        if (!header.startsWith('#alphabet<')) throw new Error("Missing '#alphabet<lang>' header");
        return raw.slice(1).map((l, i) => ({ text: l, num: i + 2 }));
    }

    pass1(lines) {
        // Find all top-level function declarations
        for (const line of lines) {
            const t = line.text.trim();
            const m = t.match(/^m\s+(\S+)\s+(\w+)\s*\(([^)]*)\)\s*\{/);
            if (m) {
                const retType = m[1];
                const name = m[2];
                const params = m[3].split(',').map(p => p.trim().split(/\s+/)[1]).filter(Boolean);
                this.functions[name] = { retType, params, body: [], startLine: line.num };
            }
        }

        // Collect function bodies
        let currentFunc = null;
        let braceDepth = 0;
        for (const line of lines) {
            const t = line.text.trim();
            if (!currentFunc) {
                const m = t.match(/^m\s+\S+\s+(\w+)\s*\([^)]*\)\s*\{/);
                if (m) {
                    currentFunc = m[1];
                    braceDepth = 1;
                    if (t.includes('}')) braceDepth--;
                    continue;
                }
            } else {
                for (const c of t) {
                    if (c === '{') braceDepth++;
                    if (c === '}') braceDepth--;
                }
                if (braceDepth <= 0) {
                    currentFunc = null;
                    continue;
                }
                this.functions[currentFunc].body.push(line);
            }
        }
    }

    pass2(lines) {
        let i = 0;
        while (i < lines.length) {
            const t = lines[i].text.trim();
            if (t === '' || t.startsWith('#') || t.startsWith('//')) { i++; continue; }

            // Skip function declarations (handled in pass1)
            if (t.match(/^m\s+\S+\s+\w+\s*\(/)) {
                let depth = 0;
                while (i < lines.length) {
                    for (const c of lines[i].text) {
                        if (c === '{') depth++;
                        if (c === '}') depth--;
                    }
                    i++;
                    if (depth <= 0) break;
                }
                continue;
            }

            const result = this.execStatement(t, lines, i);
            if (result && result.type === 'jump') {
                i = result.target;
            } else {
                i++;
            }
        }
    }

    execStatement(text, lines, lineIdx) {
        text = text.trim();
        if (!text || text.startsWith('//') || text.startsWith('#')) return null;

        // Variable declaration: TYPE name = expr
        let m = text.match(/^(\d+)\s+(\w+)\s*=\s*(.+)$/);
        if (m) {
            const val = this.evalExpr(m[3].trim());
            this.globals[m[2]] = val;
            return null;
        }

        // Variable assignment: name = expr
        m = text.match(/^(\w+)\s*=\s*(.+)$/);
        if (m && this.globals.hasOwnProperty(m[1])) {
            this.globals[m[1]] = this.evalExpr(m[2].trim());
            return null;
        }

        // z.o(expr) - print
        if (text.match(/^z\.o\(/)) {
            const arg = text.match(/z\.o\((.+)\)$/);
            if (arg) {
                const val = this.evalExpr(arg[1].trim());
                this.output.push(this.valToString(val));
            }
            return null;
        }

        // if: i (cond) {
        m = text.match(/^i\s*\((.+)\)\s*\{/);
        if (m) {
            const cond = this.evalExpr(m[1].trim());
            if (this.isTruthy(cond)) {
                // Execute block
                const blockEnd = this.findBlockEnd(lines, lineIdx);
                const blockLines = lines.slice(lineIdx + 1, blockEnd);
                this.execBlock(blockLines);
                // Skip else if present
                if (blockEnd + 1 < lines.length && lines[blockEnd + 1].text.trim().startsWith('e ')) {
                    const elseEnd = this.findBlockEnd(lines, blockEnd + 1);
                    return { type: 'jump', target: elseEnd + 1 };
                }
                return { type: 'jump', target: blockEnd + 1 };
            } else {
                // Skip to else or end
                const blockEnd = this.findBlockEnd(lines, lineIdx);
                if (blockEnd + 1 < lines.length && lines[blockEnd + 1].text.trim().startsWith('e ')) {
                    const elseEnd = this.findBlockEnd(lines, blockEnd + 1);
                    const blockLines = lines.slice(blockEnd + 2, elseEnd);
                    this.execBlock(blockLines);
                    return { type: 'jump', target: elseEnd + 1 };
                }
                return { type: 'jump', target: blockEnd + 1 };
            }
        }

        // return: r expr
        m = text.match(/^r\s+(.+)$/);
        if (m) {
            return { type: 'return', value: this.evalExpr(m[1].trim()) };
        }

        // Function call as statement
        if (text.match(/^\w+\(/)) {
            this.evalExpr(text);
            return null;
        }

        return null;
    }

    execBlock(lines) {
        let i = 0;
        while (i < lines.length) {
            const t = lines[i].text.trim();
            if (t === '' || t.startsWith('//')) { i++; continue; }
            const result = this.execStatement(t, lines, i);
            if (result && result.type === 'return') return result;
            if (result && result.type === 'jump') { i = result.target; }
            else { i++; }
        }
    }

    findBlockEnd(lines, startLine) {
        let depth = 0;
        for (let i = startLine; i < lines.length; i++) {
            const t = lines[i].text.trim();
            for (const c of t) {
                if (c === '{') depth++;
                if (c === '}') { depth--; if (depth === 0) return i; }
            }
        }
        return lines.length - 1;
    }

    evalExpr(expr) {
        expr = expr.trim();
        if (!expr) return null;

        // String literal
        if (expr.startsWith('"') && expr.endsWith('"')) {
            let s = expr.slice(1, -1);
            s = s.replace(/\\n/g, '\n').replace(/\\t/g, '\t').replace(/\\\\/g, '\\').replace(/\\"/g, '"');
            return s;
        }

        // Number literal
        if (/^-?\d+(\.\d+)?$/.test(expr)) return parseFloat(expr);

        // Boolean-ish
        if (expr === '1' && !expr.includes('.')) return 1;
        if (expr === '0' && !expr.includes('.')) return 0;

        // null
        if (expr === 'null') return null;

        // Function call: func(args)
        const callMatch = expr.match(/^(\w+)\(([^)]*)\)$/);
        if (callMatch && this.functions[callMatch[1]]) {
            return this.callFunction(callMatch[1], callMatch[2]);
        }

        // Method call: obj.method(args)
        const methodMatch = expr.match(/^(\w+)\.(\w+)\(([^)]*)\)$/);
        if (methodMatch) {
            const obj = methodMatch[1];
            const method = methodMatch[2];
            if (obj === 'z' && method === 'sqrt') {
                const arg = this.evalExpr(methodMatch[3].trim());
                return Math.sqrt(arg);
            }
            if (obj === 'z' && method === 'abs') {
                const arg = this.evalExpr(methodMatch[3].trim());
                return Math.abs(arg);
            }
            if (obj === 'z' && method === 'pow') {
                const args = this.parseArgs(methodMatch[3]);
                return Math.pow(args[0], args[1]);
            }
            if (obj === 'z' && method === 'len') {
                const arg = this.evalExpr(methodMatch[3].trim());
                return typeof arg === 'string' ? arg.length : 0;
            }
            if (obj === 'z' && method === 'type') {
                const arg = this.evalExpr(methodMatch[3].trim());
                if (arg === null) return 'null';
                if (typeof arg === 'number') return 'number';
                if (typeof arg === 'string') return 'string';
                return 'unknown';
            }
            if (obj === 'z' && method === 'tostr') {
                const arg = this.evalExpr(methodMatch[3].trim());
                return this.valToString(arg);
            }
            if (obj === 'z' && method === 'tonum') {
                const arg = this.evalExpr(methodMatch[3].trim());
                return parseFloat(arg) || 0;
            }
        }

        // Variable
        if (/^\w+$/.test(expr)) {
            if (this.globals.hasOwnProperty(expr)) return this.globals[expr];
            return null;
        }

        // Binary expression with string concatenation
        const concatParts = this.splitByOperator(expr, '+');
        if (concatParts.length > 1) {
            let result = this.evalExpr(concatParts[0]);
            for (let i = 1; i < concatParts.length; i++) {
                const right = this.evalExpr(concatParts[i]);
                if (typeof result === 'string' || typeof right === 'string') {
                    result = this.valToString(result) + this.valToString(right);
                } else {
                    result = result + right;
                }
            }
            return result;
        }

        // Binary expressions
        return this.evalBinary(expr);
    }

    evalBinary(expr) {
        // Comparison operators (right to left for same precedence)
        for (const op of ['==', '!=', '>=', '<=', '>', '<']) {
            const parts = this.splitByOperator(expr, op);
            if (parts.length === 2) {
                const left = this.evalExpr(parts[0]);
                const right = this.evalExpr(parts[1]);
                switch (op) {
                    case '==': return left === right ? 1 : 0;
                    case '!=': return left !== right ? 1 : 0;
                    case '>=': return left >= right ? 1 : 0;
                    case '<=': return left <= right ? 1 : 0;
                    case '>': return left > right ? 1 : 0;
                    case '<': return left < right ? 1 : 0;
                }
            }
        }

        // Arithmetic
        for (const op of ['-', '*', '/', '%']) {
            const parts = this.splitByOperator(expr, op);
            if (parts.length === 2) {
                const left = this.evalExpr(parts[0]);
                const right = this.evalExpr(parts[1]);
                switch (op) {
                    case '-': return left - right;
                    case '*': return left * right;
                    case '/': return right !== 0 ? left / right : 0;
                    case '%': return right !== 0 ? left % right : 0;
                }
            }
        }

        // Parenthesized expression
        if (expr.startsWith('(') && expr.endsWith(')')) {
            return this.evalExpr(expr.slice(1, -1));
        }

        // Variable lookup
        if (this.globals.hasOwnProperty(expr)) return this.globals[expr];

        return null;
    }

    splitByOperator(expr, op) {
        const parts = [];
        let depth = 0;
        let current = '';
        let inString = false;

        for (let i = 0; i < expr.length; i++) {
            const c = expr[i];
            if (c === '"' && (i === 0 || expr[i-1] !== '\\')) inString = !inString;
            if (!inString) {
                if (c === '(') depth++;
                if (c === ')') depth--;
                if (depth === 0 && expr.substr(i, op.length) === op) {
                    // Check it's not a comparison inside <= or >= etc
                    if (op.length === 1 || (i > 0 && expr[i-1] !== '<' && expr[i-1] !== '>' && expr[i-1] !== '!' && expr[i-1] !== '=')) {
                        parts.push(current);
                        current = '';
                        i += op.length - 1;
                        continue;
                    }
                }
            }
            current += c;
        }
        parts.push(current);
        return parts.length > 1 ? parts : [expr];
    }

    parseArgs(argStr) {
        if (!argStr.trim()) return [];
        return argStr.split(',').map(a => this.evalExpr(a.trim()));
    }

    callFunction(name, argStr) {
        const func = this.functions[name];
        if (!func) throw new Error(`Undefined function: ${name}`);

        const args = this.parseArgs(argStr);
        const savedGlobals = { ...this.globals };

        // Set parameters
        for (let i = 0; i < func.params.length; i++) {
            this.globals[func.params[i]] = args[i] !== undefined ? args[i] : null;
        }

        // Execute body
        let result = null;
        const blockResult = this.execBlock(func.body);
        if (blockResult && blockResult.type === 'return') {
            result = blockResult.value;
        }

        // Restore globals (keep only non-param changes)
        this.globals = savedGlobals;
        return result;
    }

    valToString(val) {
        if (val === null || val === undefined) return 'null';
        if (typeof val === 'number') {
            if (Number.isInteger(val)) return val.toString();
            return val.toString();
        }
        return val.toString();
    }

    isTruthy(val) {
        if (val === null || val === undefined) return false;
        if (val === 0) return false;
        if (val === '') return false;
        return true;
    }
}
