class FileSystem {
    constructor() {
        this.storageKey = 'alphabet_ide_files';
        this.files = this.load();
        if (Object.keys(this.files).length === 0) {
            this.initDefaults();
        }
    }

    load() {
        try {
            return JSON.parse(localStorage.getItem(this.storageKey)) || {};
        } catch { return {}; }
    }

    save() {
        localStorage.setItem(this.storageKey, JSON.stringify(this.files));
    }

    initDefaults() {
        this.files = {
            'hello.abc': {
                content: `#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)

5 x = 10
5 y = 20
z.o("Sum: " + (x + y))
`,
                lang: 'alphabet'
            },
            'loop.abc': {
                content: `#alphabet<en>
5 i = 0
l (i < 5) {
  z.o("i = " + i)
  5 i = i + 1
}

z.o("---")
l (5 j = 0 : j < 5 : j = j + 1) {
  z.o("j = " + j)
}
`,
                lang: 'alphabet'
            },
            'function.abc': {
                content: `#alphabet<en>
m 5 factorial(5 n) {
  i (n <= 1) { r 1 }
  r n * factorial(n - 1)
}

m 5 fibonacci(5 n) {
  i (n <= 1) { r n }
  r fibonacci(n - 1) + fibonacci(n - 2)
}

z.o("factorial(5) = " + factorial(5))
z.o("fibonacci(10) = " + fibonacci(10))
`,
                lang: 'alphabet'
            },
            'class.abc': {
                content: `#alphabet<en>
c Calculator {
  5 value = 0

  v m 5 add(5 x) {
    value = value + x
    r value
  }

  v m 5 reset() {
    value = 0
    r value
  }
}

15 calc = n Calculator()
z.o(calc.add(10))
z.o(calc.add(5))
z.o(calc.add(25))
`,
                lang: 'alphabet'
            },
            'math.abc': {
                content: `#alphabet<en>
z.o("sqrt(144) = " + z.sqrt(144))
z.o("abs(-42) = " + z.abs(-42))
z.o("pow(2, 10) = " + z.pow(2, 10))
z.o("len(\\"hello\\") = " + z.len("hello"))
z.o("type(42) = " + z.type(42))
z.o("tostr(42) = " + z.tostr(42))
`,
                lang: 'alphabet'
            },
            'script.js': {
                content: `// JavaScript file
function greet(name) {
    return \`Hello, \${name}!\`;
}

console.log(greet("Alphabet"));
`,
                lang: 'javascript'
            },
            'index.html': {
                content: `<!DOCTYPE html>
<html>
<head>
    <title>My Page</title>
</head>
<body>
    <h1>Hello World</h1>
</body>
</html>`,
                lang: 'html'
            },
            'style.css': {
                content: `/* CSS file */
body {
    font-family: sans-serif;
    background: #1a1b26;
    color: #c0caf5;
}

h1 {
    color: #7aa2f7;
}`,
                lang: 'css'
            }
        };
        this.save();
    }

    list() {
        return Object.keys(this.files).sort();
    }

    read(name) {
        return this.files[name]?.content || '';
    }

    getLang(name) {
        return this.files[name]?.lang || this.detectLang(name);
    }

    detectLang(name) {
        const ext = name.split('.').pop();
        const map = {
            'abc': 'alphabet', 'js': 'javascript', 'ts': 'typescript',
            'py': 'python', 'html': 'html', 'css': 'css', 'json': 'json',
            'md': 'markdown', 'rs': 'rust', 'cpp': 'cpp', 'c': 'c',
            'java': 'java', 'go': 'go', 'rb': 'ruby', 'php': 'php',
            'sh': 'shell', 'sql': 'sql', 'xml': 'xml', 'yaml': 'yaml'
        };
        return map[ext] || 'plaintext';
    }

    write(name, content) {
        const lang = this.files[name]?.lang || this.detectLang(name);
        this.files[name] = { content, lang };
        this.save();
    }

    create(name) {
        if (this.files[name]) return false;
        this.write(name, '');
        return true;
    }

    delete(name) {
        delete this.files[name];
        this.save();
    }

    rename(oldName, newName) {
        if (!this.files[oldName] || this.files[newName]) return false;
        this.files[newName] = this.files[oldName];
        delete this.files[oldName];
        this.save();
        return true;
    }
}
