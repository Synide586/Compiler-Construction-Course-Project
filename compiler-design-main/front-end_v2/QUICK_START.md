# QUICK START GUIDE
## Compiler Front-End - CS F363

### 🚀 Get Started in 3 Steps

#### Step 1: Build Everything
```bash
make            # Build the compiler
make test_lexer # Build the standalone lexer tester
```

#### Step 2: Run a Test
```bash
./stage1exe parser_test_cases/t3.txt parsetree.txt
# Select option 3 to parse, then 0 to exit
```

#### Step 3: Test the Lexer Standalone
```bash
./test_lexer lexer_test_cases/t2.txt
```

---

## Quick Command Reference

### Build Commands
```bash
make              # Build compiler (stage1exe)
make test_lexer   # Build standalone lexer tester
make clean        # Remove build artifacts
make rebuild      # Clean and rebuild
```

### Run Commands
```bash
# Interactive menu mode
./stage1exe parser_test_cases/t3.txt output.txt

# Standalone lexer test
./test_lexer lexer_test_cases/t1.txt

# Automated test suite
bash run_tests.sh
```

### Test Targets
```bash
make test-lexer   # Run lexer on all test cases (t1, t2)
make test-parser  # Run parser on all test cases (t3–t6)
make test-all     # Run everything
```

---

## Menu Options Quick Reference

When you run `./stage1exe`, you'll see:

```
0 : Exit
1 : Remove comments and print clean code
2 : Print token list (lexical analysis)
3 : Parse and verify syntax (with parse tree)
4 : Print total time taken
```

**Most Common Usage**: Option 3 (full parsing with parse tree generation)

---

## Test Files

### Lexer Tests (`lexer_test_cases/`)
| File | Description |
|------|-------------|
| `t1.txt` | Mixed valid/invalid code — 9 lexical errors |
| `t2.txt` | Syntactically correct code — 97 tokens, 0 errors |
| `lexemesandtokens_t1.txt` | Expected token output for t1 |
| `lexemesandtokens_t2.txt` | Expected token output for t2 |

### Parser Tests (`parser_test_cases/`)
| File | Description |
|------|-------------|
| `t3.txt` | Records, function calls — syntactically correct ✅ |
| `t4.txt` | Arithmetic, conditionals — syntactically correct ✅ |
| `t5.txt` | Unions, definetype — syntactically correct ✅ |
| `t6.txt` | Intentional errors — 14 errors detected ✅ |
| `listoferrors_t6.txt` | Expected errors for t6 |

---

## Common Tasks

### Task: Check if source code has lexical errors
```bash
./test_lexer your_file.txt
```

### Task: Get complete parse tree
```bash
./stage1exe your_file.txt output_parsetree.txt
# Then select option 3
```

### Task: See all tokens
```bash
./test_lexer your_file.txt 2>/dev/null   # tokens only (errors hidden)
./test_lexer your_file.txt               # tokens + errors on stderr
```

### Task: Remove comments from source code
```bash
./stage1exe your_file.txt dummy.txt
# Then select option 1
```

### Task: Run all tests automatically
```bash
bash run_tests.sh
```

---

## Language Quick Reference

### Valid Identifiers
```
Variables:  b2, c3d7, d5cb34567  (pattern: [b-d][2-7][b-d]*[2-7]*)
Functions:  _main, _computeValue  (pattern: _[a-zA-Z]+[0-9]*)
Records:    #book, #student       (pattern: #[a-z]+)
Fields:     price, edition        (pattern: [a-z]+)
```

### Basic Program Structure
```
_functionName
input parameter list [int b2, int c3cd]
output parameter list [int d5];
    type int : b4;
    b4 <--- 10;
    return [d5];
end

_main
    type int : b3;
    b3 <--- 7;
    write(b3);
    return;
end
```

### Operators
```
Assignment:    <---
Arithmetic:    +  -  *  /
Relational:    <  <=  ==  >  >=  !=
Logical:       &&&  @@@  ~
```

### Comments
```
% This is a comment (everything after % until end of line)
```

---

## Troubleshooting

### "Cannot open file"
- Check file exists: `ls -l yourfile.txt`
- Check file path is correct (use relative or absolute path)

### "Lexical error"
- Check token patterns match spec
- Common errors: `<-` (should be `<---`), `23.4` (should be `23.40`)

### "Syntax error"
- Check semicolons after statements
- Check `end` keyword after functions
- Check parentheses/brackets match
- Use `./test_lexer` first to verify tokens are correct

### Build fails
```bash
make clean && make rebuild
```

---

## File Organization

```
front-end_v2/
├── stage1exe                   ← Main compiler executable (after make)
├── test_lexer                  ← Standalone lexer tester (after make test_lexer)
├── driver.c                    ← Main driver with interactive menu
├── lexer.c / lexer.h / lexerDef.h  ← Lexical analyzer module
├── parser.c / parser.h / parserDef.h  ← Syntax analyzer module
├── test_lexer.c                ← Standalone lexer test harness
├── makefile                    ← Build system
├── run_tests.sh                ← Automated test runner
├── Modified LL(1) Grammar.pdf  ← Grammar specification
├── lexer_test_cases/           ← Lexer test inputs + expected outputs
└── parser_test_cases/          ← Parser test inputs + expected errors
```

---

## Pro Tips

✅ **Always test lexer first** with `./test_lexer` before full parsing
✅ **Check line numbers** in error messages to locate issues
✅ **Compare with test cases** when writing your own code
✅ **Use `make test-all`** to verify everything works after changes
✅ **Build clean** before submission: `make clean && make`

---

**🎉 You're ready to go! Start with `make && make test_lexer` then `bash run_tests.sh`**

For detailed documentation, see **README.md**
