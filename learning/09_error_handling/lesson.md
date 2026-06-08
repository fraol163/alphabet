# Lesson 9: Error Handling

## What you'll learn
- Try/handle blocks for catching errors
- Throwing custom errors

## Concept
Errors happen. Instead of crashing, you can catch them and handle gracefully.

Real life: "Try to open the file. If it fails, show an error message."

## Syntax
```
t {
  // code that might fail
  z.t("something went wrong")
} h (12 e) {
  z.o("caught: " + e)
}
```

`t` = try, `h` = handle (catch), `z.t()` = throw

## Your turn
Throw an error and catch it.
