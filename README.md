# Compiler for a Custom Language

A complete compiler, written in C, that turns programs in a custom language (from the CS F363 Compiler Construction course at BITS Pilani) into native executables.

```
source code -> lexer -> LL(1) parser -> AST -> semantic analysis -> LLVM IR -> executable
```

## Structure

| Folder | What it contains |
|---|---|
| `front-end_v2/` | Lexer (twin buffer, DFA) and table-driven LL(1) parser with automatic FIRST/FOLLOW sets and panic-mode error recovery. **Built as a course team project**. |
| `back-end/` | AST construction, semantic analysis (symbol tables, type checking) and LLVM IR code generation. Added later as an individual effort.|

## Quick start

Requires GCC and clang.

```bash
cd back-end
make                                # build the compiler
./compiler program.txt -o program   # compile a program
./program                           # run it
make test                           # run the test suite
```

## Example

```bash
$ ./compiler ../front-end_v2/parser_test_cases/t3.txt -o average
$ printf '2\n80.00 90.00 70.00\n60.00 70.00 80.00\n' | ./average
70.00
80.00
75.00
```

## Language features

- `int` and `real` variables, records, unions and `definetype` aliases
- Functions with several input and output parameters, and global variables
- `while` loops, `if`/`else`, arithmetic and boolean expressions
- `read` and `write`

## Tests

22 tests: 5 programs that are compiled, run and checked against expected output, and 17 programs with deliberate errors that must be rejected with the right message.
