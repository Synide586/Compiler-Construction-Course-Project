# Compiler Back End

The back end turns the parse tree from the front end (`../front-end_v2`) into a native executable:

```
source.txt -> lexer -> LL(1) parser -> parse tree        (front end)
           -> AST -> semantic analysis -> LLVM IR -> clang -> executable   (back end)
```

The front end is used as it is, with one addition: `parser.c` now records the number of lexical and syntax errors in `parseErrorCount`, so the back end only runs on programs that parsed cleanly.

## Building and running

Requirements: GCC (or clang) to build the compiler, and **clang** to turn the generated LLVM IR into an executable.

```bash
cd back-end
make                                   # builds ./compiler
./compiler program.txt -o program      # compile
./program                              # run
make test                              # run the test suite
```

Options:

| Option | Effect |
|---|---|
| `-o <file>` | name of the executable (default `a.out`) |
| `--emit-llvm <file.ll>` | keep the generated LLVM IR |
| `--ast` | print the abstract syntax tree and stop |
| `--check` | stop after semantic analysis |
| `-O` | build an optimized executable (`clang -O2`) |

Example, using test case t3 from the front end (average marks of 2 students):

```bash
$ ./compiler ../front-end_v2/parser_test_cases/t3.txt -o average
$ printf '2\n80.00 90.00 70.00\n60.00 70.00 80.00\n' | ./average
70.00
80.00
75.00
```

## Stages

| File | Stage | What it does |
|---|---|---|
| `ast_builder.c` | AST | Walks the parse tree and builds a compact syntax tree. The LL(1) grammar is right-recursive (`<expPrime>`, `<termPrime>`), so expressions are folded back into left-to-right operations: `a - b - c` means `(a - b) - c`. |
| `semantic.c` | Semantic analysis | Builds the type and symbol tables and type-checks every statement (rules below). Reports every error with its line number. |
| `codegen.c` | Code generation | Emits LLVM IR as text. clang compiles the IR to machine code. |
| `compiler.c` | Driver | Runs all stages in order and stops at the first stage with errors. |

## Language rules checked by the semantic analyser

**Types and names**
- Record and union types are visible in the whole program, wherever they are defined. `definetype` creates another name for an existing type.
- A record may contain other records and unions, but not itself.
- Global variables (`: global`) are visible in every function. A local variable or parameter cannot have the same name as a global.
- Every variable must be declared, and only once per function.

**Expressions and assignment**
- `int` and `real` mix freely in arithmetic and comparisons; the int is converted to real, as in C. `int / int` is integer division.
- An int value can be assigned to a real variable, but a real value cannot be assigned to an int variable.
- Two records of the same type can be added or subtracted field by field. A record can be multiplied by a number (on either side) or divided by a number (on the right).
- Comparisons need int or real operands. `read` needs an int or real variable.

**Functions**
- A function must be defined before it is called, and cannot call itself.
- A call must pass the right number and types of inputs and receive the right number and types of outputs.
- `return [ ... ]` must return one value per output parameter, and every output parameter must be assigned somewhere in the function. `_main` returns nothing.

It also warns when no variable in a `while` condition changes inside the loop, since that loop would never end.

## How the code is generated

| Language | LLVM IR |
|---|---|
| `int`, `real` | `i32`, `double` |
| `record #x` | struct `%rec.x`, one member per field |
| `union #x` | `%union.x`: storage large enough for the biggest field; every field starts at offset 0 |
| local variables and parameters | stack slots (`alloca`), initialised to 0 |
| global variables | LLVM globals, initialised to 0 |
| input parameters | passed by value |
| output parameters | the caller passes pointers to its variables; `return [...]` stores the results through them |
| record arithmetic | field by field with `extractvalue` / `insertvalue` |
| `while`, `if` | basic blocks and conditional branches |
| `read`, `write` | `scanf` and `printf`; reals are printed with 2 decimals, records print all their fields on one line |

For example, `_readMarks` from t3 becomes:

```llvm
define void @fn._readMarks(i32 %a.b7, ptr %o.b3c45) {
entry:
  %v.b7 = alloca i32, align 8
  store i32 %a.b7, ptr %v.b7
  %v.b3c45 = alloca %rec.marks, align 8
  store %rec.marks zeroinitializer, ptr %v.b3c45
  %t1 = getelementptr inbounds %rec.marks, ptr %v.b3c45, i32 0, i32 0
  %t2 = call i32 (ptr, ...) @scanf(ptr @.str.1, ptr %t1)
  ...
  %t7 = load %rec.marks, ptr %v.b3c45
  store %rec.marks %t7, ptr %o.b3c45
  ret void
}
```

## Tests

`make test` builds the compiler and runs `run_tests.sh`:

- **`tests/run/`** (5 programs): each is compiled, run with its `.in` file as input, and its output compared with the `.expected` file. They include front-end test cases t3 and t4, plus programs covering functions with several outputs, global variables, loops, compound conditions, nested records, unions inside records, `definetype`, record arithmetic, and int/real promotion.
- **`tests/errors/`** (17 programs): each contains one mistake and must be rejected with the message on its `% EXPECT:` line. They cover lexical and syntax errors, undeclared variables, type mismatches, wrong argument counts, calls before definition, recursion, unknown fields, unassigned outputs, duplicate names, and front-end test case t5.

Current result: **22 passed, 0 failed**. The compiler was also run under AddressSanitizer and UndefinedBehaviorSanitizer on every test with no findings, and all programs give the same output when built with `-O`.

## Limitations

- A whole union, or a record containing a union, cannot be written, compared or used in arithmetic; its fields can.
- `&&&` and `@@@` evaluate both sides (conditions have no side effects, so the result is the same).
- Division by zero and invalid input to `read` are not checked at run time.
- Reals are printed with 2 decimal places.
