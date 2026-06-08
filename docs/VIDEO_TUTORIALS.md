# Alphabet Language — Video Tutorial Scripts

## Series 1: Beginner Tutorial (10 videos)

### Video 1: "Hello World" (3 min)
```
[INTRO - 0:00]
"Welcome to Alphabet! Today we'll write our first program."

[SETUP - 0:30]
"First, install Alphabet:"
  git clone https://github.com/alphabet-lang/alphabet
  cd alphabet
  cmake -S . -B build
  cmake --build build

[HELLO WORLD - 1:00]
"Create hello.abc:"
  #alphabet<en>
  z.o("Hello, World!")

[RUN - 1:30]
"Run it:"
  ./build/alphabet run hello.abc

[OTHER LANGUAGES - 2:00]
"Try Amharic:"
  #alphabet<am>
  ውጤት("ሰላም ዓለም!")

[END - 2:30]
"Next: Variables and Types"
```

### Video 2: "Variables and Types" (4 min)
```
[INTRO - 0:00]
"Today: variables and types in Alphabet."

[TYPE IDS - 0:30]
"Type IDs — numbers for types:"
  5 x = 42          # number
  12 name = "Fraol"  # string
  11 flag = 1        # bool

[TYPE NAMES - 1:30]
"Or use readable names:"
  integer x = 42
  string name = "Fraol"
  bool flag = 1

[TYPE INFERENCE - 2:30]
"Or let the compiler figure it out:"
  let x = 42
  let name = "Fraol"

[TYPE CHECKING - 3:00]
"Check types at runtime:"
  z.type(42)      → "number"
  z.type("hello") → "string"

[END - 3:30]
"Next: Arithmetic and Operators"
```

### Video 3: "Control Flow" (5 min)
```
[INTRO - 0:00]
"if/else and loops in Alphabet."

[IF/ELSE - 0:30]
  5 x = 10
  i (x > 5) {
    z.o("big")
  } e {
    z.o("small")
  }

[ELSE IF - 1:30]
  i (x > 10) { z.o("huge") }
  e i (x > 5) { z.o("big") }
  e { z.o("small") }

[TERNARY - 2:30]
  12 result = x > 5 ? "big" : "small"

[LOOPS - 3:00]
  l (5 i = 0 : i < 10 : i = i + 1) {
    z.o(i)
  }

[FOR EACH - 4:00]
  5 lst = [1, 2, 3]
  l (item : lst) {
    z.o(item)
  }

[END - 4:30]
"Next: Functions"
```

## Series 2: Advanced Topics (5 videos)

### Video 1: "Functional Programming" (5 min)
```
[INTRO - 0:00]
"map, filter, reduce in Alphabet."

[LAMBDAS - 0:30]
  5 double = m(5 x) { r x * 2 }

[MAP - 1:30]
  5 lst = [1, 2, 3, 4, 5]
  5 doubled = z.map(lst, m(5 x) { r x * 2 })

[FILTER - 2:30]
  5 evens = z.filter(lst, m(5 x) { r x % 2 == 0 })

[REDUCE - 3:30]
  5 sum = z.reduce(lst, 0, m(5 acc, 5 x) { r acc + x })

[CLOSURES - 4:00]
  5 factor = 10
  5 multiply = m(5 x) { r x * factor }

[END - 4:30]
"Closures capture global variables. Next: OOP"
```

## Production Notes

- Screen recording: OBS Studio
- Editor: VS Code with Alphabet extension
- Terminal: Dark theme, large font
- Resolution: 1920x1080
- Length: 2-5 minutes per video
- Style: Friendly, educational, hands-on
