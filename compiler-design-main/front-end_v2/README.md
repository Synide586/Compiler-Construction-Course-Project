# Compiler Front-End Implementation
## CS F363 Compiler Design Project

### Team Information
- **Course**: CS F363 - Compiler Construction
- **Institution**: BITS Pilani
- **Compiler**: GCC / Clang (macOS / Linux compatible)

---

## Project Overview

This is a complete implementation of the **front-end** of a compiler for a custom programming language. The compiler consists of two main modules:

1. **Lexical Analyzer (Lexer)** - Tokenizes source code using DFA-based recognition
2. **Syntax Analyzer (Parser)** - Verifies syntactic correctness using LL(1) predictive parsing

### Key Features

#### Lexical Analyzer
- ✅ Twin buffer implementation for efficient I/O
- ✅ DFA-based token recognition (as per DFA design document)
- ✅ Comprehensive error detection and reporting with `errorType` and `errorMsg` fields
- ✅ Support for all token types in language specification
- ✅ Single-pass scanning with complete token information collection
- ✅ Comment removal functionality (TK_COMMENT tokens returned, not silently skipped)
- ✅ Line number tracking for error reporting

#### Syntax Analyzer
- ✅ Automated FIRST and FOLLOW set computation
- ✅ LL(1) predictive parsing table generation
- ✅ 95 grammar production rules per Modified LL(1) Grammar specification
- ✅ Parse tree construction during parsing
- ✅ Comprehensive syntax error detection with line numbers
- ✅ Panic-mode error recovery
- ✅ Parse tree output in specified format
- ✅ Single-pass parsing

---

## File Structure

```
front-end_v2/
├── lexerDef.h                  # Lexer data structures and token definitions
├── lexer.h                     # Lexer function prototypes
├── lexer.c                     # Lexer implementation (twin buffer, DFA)
├── parserDef.h                 # Parser data structures (grammar, parse tree)
├── parser.h                    # Parser function prototypes
├── parser.c                    # Parser implementation (FIRST/FOLLOW, parsing)
├── driver.c                    # Main driver with menu interface
├── test_lexer.c                # Standalone lexer test harness
├── makefile                    # GNU make build file
├── run_tests.sh                # Automated test runner script
├── Modified LL(1) Grammar.pdf  # Language grammar specification
├── lexer_test_cases/           # Lexer test cases
│   ├── t1.txt                  #   Test case with lexical errors
│   ├── t2.txt                  #   Test case: syntactically correct code
│   ├── lexemesandtokens_t1.txt #   Expected token output for t1
│   └── lexemesandtokens_t2.txt #   Expected token output for t2
└── parser_test_cases/          # Parser test cases
    ├── t3.txt                  #   Records, function calls, read/write
    ├── t4.txt                  #   Arithmetic, conditionals, global vars
    ├── t5.txt                  #   Unions, definetype, nested dot access
    ├── t6.txt                  #   Intentional syntax errors
    └── listoferrors_t6.txt     #   Expected errors for t6
```

---

## Building and Running

### Prerequisites
- GCC or Clang (any recent version with C11 support)
- macOS or Linux
- GNU Make utility

### Compilation

```bash
# Build the compiler
make

# Build the standalone lexer tester
make test_lexer

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild
```

### Execution

```bash
./stage1exe <source_file.txt> <parsetree_output.txt>
```

**Example:**
```bash
./stage1exe parser_test_cases/t3.txt parsetree.txt
```

### Running Tests

```bash
# Run all tests (lexer + parser)
make test-all

# Run only lexer tests
make test-lexer

# Run only parser tests
make test-parser

# Run standalone lexer on a file
./test_lexer lexer_test_cases/t1.txt

# Run automated test suite
bash run_tests.sh
```

---

## Menu Options

The compiler provides an interactive menu with the following options:

### Option 0: Exit
Terminates the program.

### Option 1: Remove Comments
- Removes all comments (lines starting with `%`) from source code
- Displays the clean code on console

**Use case**: Preprocessing source code before analysis

### Option 2: Print Token List
- Performs lexical analysis on source code
- Displays all tokens with:
  - Token name
  - Lexeme
  - Line number
  - Value (for numbers)
- Reports lexical errors with line numbers and descriptive messages

**Use case**: Testing and debugging the lexical analyzer

### Option 3: Parse and Verify Syntax
- Performs complete lexical and syntax analysis
- Builds parse tree if code is syntactically correct
- Reports all errors (lexical and syntactic) with line numbers
- Writes parse tree to specified output file

**Parse Tree Format:**
Each line contains:
```
LEXEME | CURRENT_NODE | LINE_NO | TOKEN_NAME | VALUE | PARENT_NODE | IS_LEAF | NODE_SYMBOL
```

**Use case**: Full compiler front-end execution

### Option 4: Performance Timing
- Measures total execution time for lexical and syntax analysis
- Reports CPU time in clock ticks and seconds

**Use case**: Performance analysis and optimization

---

## Language Specification Summary

### Keywords
`with`, `parameters`, `end`, `while`, `union`, `endunion`, `definetype`, `as`, `type`, `global`, `parameter`, `list`, `input`, `output`, `int`, `real`, `endwhile`, `if`, `then`, `endif`, `read`, `write`, `return`, `call`, `record`, `endrecord`, `else`

### Identifiers
- **Variable IDs**: `[b-d][2-7][b-d]*[2-7]*` (e.g., `b2`, `c3d7`, `d5cb34567`)
- **Function IDs**: `_[a-z|A-Z]+[0-9]*` (e.g., `_main`, `_computeValue`)
- **Record/Union IDs**: `#[a-z]+` (e.g., `#book`, `#student`)
- **Field IDs**: `[a-z]+` (e.g., `price`, `edition`, `listb`)

### Numbers
- **Integer**: `[0-9]+` (e.g., `123`, `5`)
- **Real**: `[0-9]+.[0-9][0-9]` (e.g., `12.34`, `0.00`)
- **Real with exponent**: `[0-9]+.[0-9][0-9]E[+|-][0-9][0-9]` (e.g., `34.54E-2`)

### Operators
- **Assignment**: `<---`
- **Arithmetic**: `+`, `-`, `*`, `/`
- **Relational**: `<`, `<=`, `==`, `>`, `>=`, `!=`
- **Logical**: `&&&` (AND), `@@@` (OR), `~` (NOT)

### Comments
Lines starting with `%` are treated as comments.

---

## Error Handling

### Lexical Errors Detected
- Unknown symbols (e.g., `$`, `|`)
- Incomplete operators (`<-`, `<--`, `=`, `!`, `&`, `&&`, `@`, `@@`)
- Malformed real numbers (`23.`, `23.4`, `5000.7`, `23.45E-`)
- Variable identifiers exceeding 20 characters
- Function identifiers exceeding 30 characters
- Invalid identifier patterns

### Syntax Errors Detected
- Unexpected tokens
- Missing required tokens (with expected vs. received reporting)
- Invalid grammar productions
- Mismatched parentheses, brackets
- Incomplete statements

All errors are reported with:
- Line number
- Descriptive error message
- Expected vs received tokens (for syntax errors)

---

## Design Highlights

### Twin Buffer Implementation
The lexer uses an efficient twin buffer mechanism:
- **Buffer Size**: 4096 bytes per half
- **Total Size**: 8192 bytes
- **EOF-safe retraction**: `retract()` correctly handles EOF boundary conditions
- **Advantages**:
  - Minimizes I/O operations
  - Supports unlimited lookahead
  - Handles EOF gracefully
  - Efficient retraction mechanism

### DFA-Based Tokenization
- Implements complete DFA from design document
- Maximal munch (longest match) principle
- Keyword recognition via lookup table
- Efficient state transitions
- Proper error recovery

### LL(1) Predictive Parsing
- **Grammar**: 95 production rules (per Modified LL(1) Grammar specification)
- **FIRST/FOLLOW**: Automatically computed via fixed-point iteration
- **Parse Table**: Efficiently generated from FIRST/FOLLOW sets
- **Parsing**: Single-pass, top-down
- **Error Recovery**: Panic-mode with sync token sets
- **Parse Tree**: N-ary tree with proper parent-child relationships

### Memory Management
- Proper allocation and deallocation
- Efficient data structure usage

---

## Test Cases

### Lexer Test Cases (`lexer_test_cases/`)

| File | Description |
|------|-------------|
| `t1.txt` | Mixed valid/invalid code — tests error detection (9 lexical errors) |
| `t2.txt` | Syntactically correct code — tests all basic token types (97 tokens) |
| `lexemesandtokens_t1.txt` | Expected token output for t1 |
| `lexemesandtokens_t2.txt` | Expected token output for t2 |

### Parser Test Cases (`parser_test_cases/`)

| File | Description |
|------|-------------|
| `t3.txt` | Records, function calls, read/write — syntactically correct |
| `t4.txt` | Arithmetic, conditionals, global variables — syntactically correct |
| `t5.txt` | Unions, definetype, nested dot access — syntactically correct |
| `t6.txt` | Intentional syntax errors — tests error detection (14 errors) |
| `listoferrors_t6.txt` | Expected errors for t6 |

---

## Performance Characteristics

### Time Complexity
- **Lexical Analysis**: O(n) where n = input length
- **Syntax Analysis**: O(n) where n = number of tokens
- **FIRST/FOLLOW Computation**: O(g²) where g = grammar size
- **Parse Table Creation**: O(g × t) where t = terminal count

### Space Complexity
- **Twin Buffer**: O(1) constant space (8KB)
- **Parse Table**: O(n × t) where n = non-terminals, t = terminals
- **Parse Tree**: O(n) where n = number of nodes

---

## Known Limitations

1. **Maximum Lengths**:
   - Variable identifier: 2-20 characters
   - Function identifier: ≤30 characters
   - Lexeme: 256 characters

2. **Grammar**:
   - Must be LL(1) compatible
   - While/if/else bodies use `<stmt><otherStmts>` rather than `<stmts>` (which would require a return statement in every block)

3. **Error Recovery**:
   - Panic mode recovery — may skip valid tokens after errors
   - Duplicate error suppression per line

---

## Troubleshooting

### Build Issues
```bash
# If make fails, clean and rebuild
make clean && make rebuild

# Check compiler version
gcc --version
```

### Runtime Issues
```bash
# If file not found
ls -l parser_test_cases/t3.txt

# Use standalone lexer to debug token issues first
./test_lexer your_file.txt
```

### Common Errors
- **"Cannot open file"**: Check file path and permissions
- **"Lexical error"**: Check token patterns in source code
- **"Syntax error"**: Check grammar rules and token sequence

---

## References

1. Modified LL(1) Grammar.pdf (included in repository)
2. Dragon Book (Compilers: Principles, Techniques, and Tools) - Aho, Sethi, Ullman
3. Language Specification Document (CS F363)
4. DFA Design Document (Project Deliverable)

---

## Version History

- **v1.0** (Feb 2026): Initial release with complete front-end
  - Lexical analyzer with twin buffer
  - Syntax analyzer with LL(1) parsing
  - FIRST/FOLLOW automation
  - Parse tree generation
  - Comprehensive error reporting

- **v1.1** (Feb 2026): Bug fixes and grammar compliance
  - Fixed critical infinite loop in lexer (EOF-unsafe retract)
  - Removed dead code (unused functions and declarations)
  - Updated grammar rules 35 and 49 to match PDF specification
  - Added standalone test_lexer harness
  - Added automated test runner
  - Updated makefile with test targets

---

## License

This project is submitted as part of CS F363 coursework at BITS Pilani.
For academic use only.
