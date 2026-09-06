@lisp(version=1.0)

// Reference Lisp/Iris program in Alphabet Forge
def greeting = "Hello from Lisp under Alphabet Forge!";
write(greeting);

def factor = 6;
def multiplier = 7;
def result = factor * multiplier;
write("Calculated answer: " + result);

when (result == 42) {
    write("Computation verified successfully!");
}
