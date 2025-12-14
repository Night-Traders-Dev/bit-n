# bit(N) — Vision Document

## Why bit(N) Exists

Modern software increasingly sits at the boundary between high‑level logic and low‑level reality. Embedded systems, firmware, operating systems, networking stacks, cryptography, emulators, drivers, and performance‑critical code all depend on precise control over bits, registers, memory layouts, and CPU instructions.

Today, developers face an uncomfortable trade‑off:

**High‑level languages** are expressive and safer, but abstract away bit‑level control or make it awkward, verbose, or unpredictable.

**Low‑level languages** (C, assembly) give full control, but expose developers to undefined behavior, silent bugs, unsafe memory access, and error‑prone code that is difficult to reason about and maintain.

bit(N) exists to eliminate this trade‑off.

The goal of bit(N) is to allow developers to work close to the hardware without feeling like they are working at the lowest level. It aims to make bit manipulation, memory layout, and even assembly access explicit, safe by default, and intuitive, while compiling to code that is as fast and predictable as handwritten C or assembly.

In short:

> **bit(N) exists to make low‑level programming clearer, safer, and more intentional, without sacrificing performance or control.**

---

## What bit(N) Is

bit(N) is a systems programming language designed around bit‑precision correctness, explicitness, and zero‑cost abstractions.

It is not meant to replace C, Rust, or assembly universally. Instead, it focuses on a specific problem space:

- Code where **bit layouts matter**
- Code where **performance must be deterministic**
- Code where **hardware interaction is unavoidable**

### Core Principles

#### 1. Bits are first‑class citizens

- Fixed‑width integer types (`u8`, `u16`, `u32`, etc.)
- Explicit bit slicing, insertion, and extraction
- Declarative bitfields and packed data layouts

#### 2. Safety by default, control by choice

- No implicit undefined behavior
- Bounds checking and overflow checks in safe code
- Explicit `unsafe` blocks when the programmer chooses to bypass checks

#### 3. Zero‑cost abstractions

- High‑level constructs compile down to optimal machine code
- No hidden runtime penalties
- What you write is what the CPU executes

#### 4. Intuitive low‑level expression

- Clear syntax for common bit tasks
- Pattern matching on bit layouts
- Minimal boilerplate for tasks that are verbose in C

#### 5. Predictable semantics across platforms

- No architecture‑dependent surprises
- Explicit endianness
- Well‑defined shift, mask, and overflow behavior

### What bit(N) Is Not

- It is not a scripting language
- It is not a garbage‑collected runtime
- It is not a general application framework
- It is not an experimental syntax playground

bit(N) is intentionally narrow: it targets the layer where software meets hardware.

---

## How bit(N) Achieves Its Goals

### Language Design

#### Explicit Bit Model

Every integer has a known width. There are no implicit promotions or hidden casts. Bit slicing is built into the language syntax:

```
let x: u32 = 0xDEADBEEF;
let low = x[15..0];   // u16
let mid = x[11..4];   // u8
```

These operations are:

- Bounds‑checked in safe mode
- Reduced to simple shifts and masks in optimized builds

#### Declarative Bitfields and Packed Layouts

bit(N) allows developers to describe memory layouts declaratively:

```
packet Header {
  version: 3,
  flags: 5,
  length: 10,
  crc: 14
}
```

This eliminates manual masking logic, improves readability, and guarantees correctness at compile time.

#### Pattern Matching on Bits

Bit patterns can be matched directly:

```
match opcode {
  0b111_???? => handle_special(),
  0b000_???? => handle_normal(),
  _ => error()
}
```

This makes protocol parsing, instruction decoding, and state machines expressive and concise.

### Safety Model

bit(N) follows a "safe by default" philosophy.

- All memory access, bit slicing, and arithmetic are checked in safe code
- Overflow, out‑of‑range shifts, and invalid bit access are detected
- Safety checks are elided in optimized builds when correctness can be proven

When absolute control is required, the programmer can explicitly opt out:

```c
unsafe {
  asm! { "mov r0, r1" }
}
```

This keeps dangerous operations visible, intentional, and auditable.

#### Inline Assembly Without Chaos

Inline assembly is treated as a controlled escape hatch, not a free‑for‑all.

- Explicit inputs, outputs, and clobbers
- No implicit register corruption
- Clearly marked `unsafe`

This allows expert users to write hand‑tuned code without undermining the compiler's ability to reason about the program.

### Compiler Architecture

bit(N) is designed around a traditional but powerful compiler pipeline:

1. **Lexer & Parser** — Build a clear AST
2. **Semantic Analysis** — Enforce type widths, bit safety, and correctness
3. **Mid‑level IR** — Preserve bit‑level intent while enabling optimization
4. **Backend Codegen** — Emit optimized machine code via:
   - LLVM IR (preferred)
   - or a portable C backend for early usability

By leveraging mature backend infrastructure, bit(N) focuses its innovation where it matters: language semantics and safety.

### Performance Guarantees

bit(N) follows the principle:

> **If you can express it, the compiler must not make it slower than hand‑written C.**

Bit operations map directly to CPU instructions:

- Rotations → `rol` / `ror`
- Popcount → `popcnt`
- Bit scans → `clz` / `ctz`

Abstractions exist only at compile time. At runtime, there is no cost.

---

## Who bit(N) Is For

bit(N) is designed for:

- Embedded and firmware developers
- OS, driver, and kernel developers
- Emulator and VM authors
- Networking and protocol engineers
- Cryptography and DSP programmers
- Anyone who writes C for bit manipulation and wishes it were clearer and safer

It is also well‑suited as an educational language for teaching:

- Compiler construction
- Low‑level programming concepts
- Hardware‑software interaction

---

## Long‑Term Vision

Over time, bit(N) aims to become:

- A trusted tool for writing bit‑critical code
- A reference for safe low‑level language design
- A bridge between readable code and raw hardware control

Success for bit(N) is not measured by mass adoption, but by whether it enables developers to write correct, fast, and understandable low‑level code with confidence.

---

## Summary

bit(N) exists because low‑level programming should not require low‑level thinking.

It provides:

- **Clear, expressive syntax** for bit manipulation
- **Safety without sacrificing performance**
- **Explicit control** over hardware interactions
- **Predictable, portable semantics**

bit(N) is where intent meets instruction, and where precision no longer comes at the cost of clarity.

---

## Implementation Status

bit(N) is currently in **active development**. The compiler infrastructure is being built as follows:

- **Lexer** — Complete with full token support
- **Parser** — Recursive descent parser with operator precedence
- **AST** — Complete abstract syntax tree representation
- **Semantic Analysis** — In progress
- **Code Generation** — Planning LLVM and C backend support

The language specification and core features are being refined based on practical use cases and compiler development experience.

---

## Getting Involved

bit(N) welcomes contributors interested in:

- Language design and specification
- Compiler optimization
- Backend code generation
- Safety analysis and verification
- Documentation and examples
- Embedded systems testing

For more information, see [CONTRIBUTING.md](CONTRIBUTING.md) and [IMPLEMENTATION.md](IMPLEMENTATION.md).
