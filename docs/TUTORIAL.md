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

## Lesson 5: Functions

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

## Lesson 6-10

See [examples/](../examples/) for more tutorials!

---

**Happy Coding! 🚀**
