# Alphabet Language — Threading Design

## Overview

Add multi-threading support to Alphabet for concurrent execution.

## Proposed Syntax

### Thread Creation
```alphabet
#alphabet<en>
5 result = 0

t {
  // This runs in a new thread
  5 total = 0
  l (5 i = 0 : i < 1000 : i = i + 1) {
    total = total + i
  }
  result = total
}

// Main thread continues
z.o("Computing in background...")
```

### Thread with Function
```alphabet
m 5 compute(5 start, 5 end) {
  5 total = 0
  l (5 i = start : i < end : i = i + 1) {
    total = total + i
  }
  r total
}

5 t1 = thread(compute, 0, 500)
5 t2 = thread(compute, 500, 1000)

5 r1 = join(t1)
5 r2 = join(t2)
z.o("Total: " + z.tostr(r1 + r2))
```

### Mutex/Lock
```alphabet
5 counter = 0
5 mutex = lock()

l (5 i = 0 : i < 100 : i = i + 1) {
  t {
    acquire(mutex)
    counter = counter + 1
    release(mutex)
  }
}
```

## Implementation Plan

### Step 1: Thread Primitive
- Add `thread` keyword (or `t` for thread)
- Create std::thread in C++ VM
- Share globals between threads (with mutex)
- Add `join()` builtin to wait for thread completion

### Step 2: Synchronization
- Add `lock()` builtin — create mutex
- Add `acquire()` builtin — lock mutex
- Add `release()` builtin — unlock mutex
- Add `atomic()` builtin — atomic operations

### Step 3: Thread-Safe Builtins
- Make z.o() thread-safe (mutex on stdout)
- Make list operations thread-safe
- Make map operations thread-safe

### Step 4: Thread Pool
- Add `pool(size)` builtin — create thread pool
- Add `submit(pool, fn)` — submit task to pool
- Add `wait(pool)` — wait for all tasks

## Challenges

1. **Shared State:** Globals shared between threads — need mutex
2. **GIL Alternative:** No GIL in Alphabet — true parallelism
3. **Deadlocks:** User can create deadlocks with mutex
4. **Race Conditions:** User can create race conditions
5. **Performance:** Thread creation overhead

## Design Decisions

1. **Shared Globals:** Yes — threads share global variables
2. **Mutex Required:** Yes — user must protect shared state
3. **Thread Pool:** Yes — for efficient task distribution
4. **Atomic Operations:** Yes — for simple counters

## Timeline

- **Week 1:** Thread primitive, join()
- **Week 2:** Mutex, lock/unlock
- **Week 3:** Thread-safe builtins
- **Week 4:** Thread pool, testing

## Status

- [x] Design document
- [ ] Thread primitive
- [ ] Mutex/Lock
- [ ] Thread-safe builtins
- [ ] Thread pool
- [ ] Testing
- [ ] Documentation
