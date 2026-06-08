#!/usr/bin/env python3
"""Benchmark suite for Alphabet Language VM."""

import subprocess
import time
import sys

ALPHABET = "./build/alphabet"

BENCHMARKS = {
    "fibonacci": """#alphabet<en>
m 5 fib(5 num) {
  i (num <= 1) { r num }
  r fib(num - 1) + fib(num - 2)
}
z.o(fib(20))
""",
    "loop_sum": """#alphabet<en>
5 total = 0
l (5 i = 0 : i < 10000 : i = i + 1) {
  total = total + i
}
z.o(total)
""",
    "string_concat": """#alphabet<en>
5 sb = z.builder()
l (5 i = 0 : i < 1000 : i = i + 1) {
  z.append_str(sb, "hello")
}
z.o(z.len(z.build(sb)))
""",
    "list_ops": """#alphabet<en>
5 lst = []
l (5 i = 0 : i < 1000 : i = i + 1) {
  append(lst, i)
}
reverse(lst)
z.o(z.len(lst))
""",
    "map_ops": """#alphabet<en>
5 m = {}
l (5 i = 0 : i < 1000 : i = i + 1) {
  m[z.tostr(i)] = i * 2
}
z.o(z.len(m))
""",
}


def run_benchmark(name, code, iterations=1):
    """Run a benchmark and return average time in ms."""
    times = []
    for _ in range(iterations):
        proc = subprocess.Popen(
            [ALPHABET, "/dev/stdin"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        start = time.time()
        proc.communicate(code.encode())
        elapsed = (time.time() - start) * 1000
        times.append(elapsed)
    return min(times)


def main():
    print(f"{'Benchmark':<20} {'Time (ms)':<12} {'Status'}")
    print("-" * 50)

    for name, code in BENCHMARKS.items():
        try:
            ms = run_benchmark(name, code)
            status = "PASS" if ms < 10000 else "SLOW"
            print(f"{name:<20} {ms:<12.1f} {status}")
        except Exception as e:
            print(f"{name:<20} {'N/A':<12} FAIL: {e}")


if __name__ == "__main__":
    main()
