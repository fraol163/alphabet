# Alphabet Language — Code Style Guide

## Naming Conventions

### Variables
```
snake_case for variables:     my_variable, user_name, total_count
UPPER_CASE for constants:     MAX_SIZE, DEFAULT_LANG, VERSION
camelCase for builtins:       z.toStr(), z.toJson() (future)
```

### Functions
```
snake_case for functions:     factorial(), binary_search(), merge_sort()
Verb + noun pattern:          get_name(), set_value(), is_valid()
```

### Classes
```
PascalCase for classes:       Animal, Dog, Calculator, Singleton
Abstract prefix (optional):   AbstractShape, AbstractAnimal
```

### Files
```
snake_case for files:         main.abc, math_utils.abc, test_helper.abc
Lowercase with underscores:   hello_world.abc, data_processing.abc
```

## Code Organization

### File Structure
```
#alphabet<en>
/// Module documentation

@ export_function1, export_function2

5 constant = 42

m 5 function1(5 arg) {
  r arg * 2
}

m 5 function2(5 arg) {
  r arg + 1
}
```

### Class Structure
```
c MyClass {
  5 field1 = 0
  12 field2 = ""

  m MyClass(5 a, 12 b) {
    this.field1 = a
    this.field2 = b
  }

  v m get_field1() {
    r this.field1
  }

  v m set_field1(5 val) {
    this.field1 = val
  }
}
```

## Formatting

### Indentation
- 2 spaces per indent level
- No tabs

### Braces
- Opening brace on same line
- Closing brace on own line

```
i (condition) {
  // body
} e {
  // else body
}
```

### Spaces
- Space after keywords: `i (`, `l (`, `m `
- Space around operators: `x + y`, `a == b`
- No space after function name: `my_func(arg)`

### Line Length
- Max 80 characters per line
- Break long lines at operators

## Comments

### Single-line
```
// This is a comment
5 x = 42  // inline comment
```

### Documentation
```
/// Function description
/// @param arg1 description
/// @return description
m 5 my_func(5 arg1) {
  r arg1 * 2
}
```

### Module Documentation
```
#alphabet<en>
/// Module Name
/// Description of what this module does
/// Author: Name
/// Version: 1.0.0
```

## Error Handling

### Prefer explicit errors
```
m 5 divide(5 a, 5 b) {
  i (b == 0) {
    z.o("Error: Division by zero")
    r 0
  }
  r a / b
}
```

### Use try/handle for recoverable errors
```
t {
  5 data = z.json_parse(input)
} h (0 err) {
  z.o("Invalid JSON: " + err)
  5 data = {}
}
```

## Testing

### Test naming
```
/// Test: factorial of 0 returns 1
assert_eq(factorial(0), 1, "factorial(0) == 1")

/// Test: factorial of 5 returns 120
assert_eq(factorial(5), 120, "factorial(5) == 120")
```

### Test organization
```
z.o("--- Math Tests ---")
assert_eq(2 + 2, 4, "addition")
assert_eq(10 - 5, 5, "subtraction")

z.o("--- String Tests ---")
assert_eq(z.len("hello"), 5, "string length")
```
