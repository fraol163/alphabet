# Alphabet Language Tutorial

**Learn Alphabet by building real programs!**

---

## Lesson 1: Hello World

```alphabet
#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)
```

---

## Lesson 2: Variables

```alphabet
#alphabet<en>
5 x = 10
6 pi = 3.14
12 name = "Alphabet"
z.o(x)
z.o(pi)
z.o(name)
```

---

## Lesson 3: If Statements

```alphabet
#alphabet<en>
5 x = 10
i (x > 5) {
  z.o("x is greater than 5")
} e {
  z.o("x is 5 or less")
}
```

---

## Lesson 4: Loops

```alphabet
#alphabet<en>
5 i = 0
l (i < 5) {
  z.o(i)
  5 i = i + 1
}
```

---

## Lesson 5: Classes & Methods

```alphabet
#alphabet<en>
c Calculator {
  v m 5 add(5 x, 5 y) {
    r x + y
  }
}

15 calc = n Calculator()
5 result = calc.add(15, 25)
z.o(result)
```

---

## Lesson 6: Error Handling

```alphabet
#alphabet<en>
t {
  5 x = 10 / 0
} h (err) {
  z.o("Caught error")
}
```

---

## Lesson 7: Lists

```alphabet
#alphabet<en>
13 nums = [1, 2, 3, 4, 5]
z.o(z.len(nums))
z.o(nums[0])
z.o(nums[-1])
```

---

## Lesson 8: Maps

```alphabet
#alphabet<en>
14 person = {"name": "Fraol", "age": 25}
z.o(person["name"])
z.o(z.keys(person))
```

---

## Lesson 9: String Operations

```alphabet
#alphabet<en>
12 msg = "Hello, World!"
z.o(z.upper(msg))
z.o(z.split(msg, ", "))
z.o(z.replace(msg, "World", "Alphabet"))
```

---

## Lesson 10: F-Strings

```alphabet
#alphabet<en>
12 name = "Alphabet"
5 version = 2
z.o(f"Welcome to {name} v{version}.0")
z.o(f"2 + 3 = {2 + 3}")
```

---

**Happy Coding! 🚀**
