# Alphabet REPL Audit Report
Date: May 29, 2026
Auditor: Renan

## CRITICAL BUGS

### B1: REPL Re-runs All Code (Duplicate Output)
- Severity: HIGH
- Description: REPL accumulates all source in `all_source` and re-compiles + re-runs ALL code on every input. Causes duplicate output.
- Example: Enter `z.o("a")` then `z.o("b")` → outputs "a" twice (from re-running line 1) then "b"
- Root cause: `vm.init(program)` + `vm.run()` executes ALL bytecode each time
- Fix needed: Incremental compilation — only execute new statements

### B2: Variable Re-declaration Silent Override
- Severity: MEDIUM
- Description: `5 x = 10` then `5 x = 20` silently overrides without warning
- Expected: Warning in strict mode, or error

### B3: Stack Underflow on run_continue
- Severity: HIGH
- Description: Attempted fix with `run_continue()` caused stack underflow because bytecode layout changes on recompile
- Status: Reverted to re-run approach

## SECURITY ISSUES

### S1: Sandbox Mode Not Enforced on Builtins
- Severity: MEDIUM
- Description: `--sandbox` flag exists but some builtins may not check it
- Check: z.system(), z.exec(), z.dyn() — these should be blocked in sandbox

### S2: No Input Validation on z.i()
- Severity: LOW
- Description: `z.i()` (input) doesn't validate or sanitize user input

### S3: File Path Traversal
- Severity: MEDIUM
- Description: Import paths like `x "../../etc/passwd"` could potentially access arbitrary files
- Check: is_safe_path() function

### S4: Memory Exhaustion via Infinite Lists
- Severity: MEDIUM
- Description: `1..999999999` creates a huge list, no limit
- Mitigation: Range expressions should have a max size

## PRODUCTIVITY ISSUES

### P1: No Tab Completion
- Severity: LOW
- Description: REPL has no tab completion for keywords, variables, or functions

### P2: No History Search (Ctrl+R)
- Severity: LOW
- Description: History exists but no reverse search

### P3: No Variable Inspection Command
- Severity: LOW
- Description: `vars` command exists but only shows names, not values/types

### P4: No Multi-line Edit
- Severity: LOW
- Description: Multi-line input works but can't edit previous lines

### P5: No Auto-indent
- Severity: LOW
- Description: After `{`, next line should auto-indent

### P6: Error Messages Don't Show Source Context
- Severity: MEDIUM
- Description: Runtime errors show line number but not the source line

### P7: REPL Doesn't Handle Ctrl+C Gracefully
- Severity: LOW
- Description: Ctrl+C during execution may leave VM in bad state

## USAGE ISSUES

### U1: Language Header Required in REPL
- Severity: LOW
- Description: Must type `#alphabet<en>` first, should default to en

### U2: No Way to See Current Language
- Severity: LOW
- Description: No command to show current language setting

### U3: Reset Command Doesn't Clear Everything
- Severity: LOW
- Description: `reset` clears globals but not functions/classes

### U4: History Dedup Not Working
- Severity: LOW
- Description: Same command entered multiple times stored multiple times

## PERFORMANCE ISSUES

### PERF1: Full Recompilation Each Input
- Severity: HIGH
- Description: Every REPL input triggers full lex+parse+compile of ALL accumulated code
- Impact: Gets slower as more code is entered

### PERF2: No Bytecode Cache in REPL
- Severity: MEDIUM
- Description: Could cache compiled bytecode and only recompile changed statements

## SUMMARY

Critical: 2 (B1 duplicate output, B3 stack underflow)
Security: 4 (S1-S4)
Productivity: 7 (P1-P7)
Usage: 4 (U1-U4)
Performance: 2 (PERF1-PERF2)

Total: 19 issues found
