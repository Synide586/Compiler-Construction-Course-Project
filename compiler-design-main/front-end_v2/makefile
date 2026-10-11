# Makefile for Compiler Front-End
# CS F363 Compiler Design Project
# Compatible with GCC / Clang (macOS / Linux)

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lm

# Target executables
TARGET = stage1exe
TEST_LEXER = test_lexer

# Source files
SOURCES = driver.c lexer.c parser.c
OBJECTS = $(SOURCES:.c=.o)

# Header dependencies
HEADERS = lexerDef.h lexer.h parserDef.h parser.h

# Default target
all: $(TARGET)
	@echo "Build successful! Executable: $(TARGET)"
	@echo "Usage: ./$(TARGET) <source_file.txt> <parsetree_output.txt>"

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Test lexer executable
$(TEST_LEXER): test_lexer.o lexer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
driver.o: driver.c lexer.h parser.h
	$(CC) $(CFLAGS) -c $<

lexer.o: lexer.c lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c $<

parser.o: parser.c parser.h parserDef.h lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c $<

test_lexer.o: test_lexer.c lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c $<

# Clean build artifacts
clean:
	rm -f $(OBJECTS) test_lexer.o $(TARGET) $(TEST_LEXER) clean_code.txt
	@echo "Clean complete"

# Clean and rebuild
rebuild: clean all

# Run with a test file
test: $(TARGET)
	./$(TARGET) testcase.txt parsetree.txt

# Run lexer tests
test-lexer: $(TEST_LEXER)
	@echo ""
	@echo "========== Lexer Test: t1.txt =========="
	./$(TEST_LEXER) lexer_test_cases/t1.txt
	@echo ""
	@echo "========== Lexer Test: t2.txt =========="
	./$(TEST_LEXER) lexer_test_cases/t2.txt

# Run parser tests
test-parser: $(TARGET)
	@echo ""
	@echo "========== Parser Test: t3.txt =========="
	@echo "3" | ./$(TARGET) parser_test_cases/t3.txt parsetree_t3.txt
	@echo ""
	@echo "========== Parser Test: t4.txt =========="
	@echo "3" | ./$(TARGET) parser_test_cases/t4.txt parsetree_t4.txt
	@echo ""
	@echo "========== Parser Test: t5.txt =========="
	@echo "3" | ./$(TARGET) parser_test_cases/t5.txt parsetree_t5.txt
	@echo ""
	@echo "========== Parser Test: t6.txt (expects errors) =========="
	@echo "3" | ./$(TARGET) parser_test_cases/t6.txt parsetree_t6.txt

# Run all tests
test-all: test-lexer test-parser
	@echo ""
	@echo "========== ALL TESTS COMPLETE =========="

# Help target
help:
	@echo "Makefile for Compiler Front-End"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the compiler (default)"
	@echo "  $(TEST_LEXER)   - Build the standalone lexer test harness"
	@echo "  clean        - Remove object files and executables"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  test         - Build and run with testcase.txt"
	@echo "  test-lexer   - Run lexer on all lexer test cases"
	@echo "  test-parser  - Run parser on all parser test cases"
	@echo "  test-all     - Run all tests (lexer + parser)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make                           # Build compiler"
	@echo "  make test_lexer                # Build lexer test harness"
	@echo "  make test-all                  # Run all tests"
	@echo "  make clean                     # Clean build"
	@echo "  ./stage1exe input.txt out.txt  # Run compiler"

.PHONY: all clean rebuild test test-lexer test-parser test-all help
