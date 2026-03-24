PROJECT := Machine
CC := gcc
CFLAGS := -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -O2 -Iinclude
RUNTIME_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image 2>/dev/null)
RUNTIME_LIBS := $(shell pkg-config --libs sdl2 SDL2_image 2>/dev/null)

SRC := src/common.c src/util.c src/lexer.c src/parser_expr.c src/parser_stmt.c src/codegen.c
COMPILATION_SRC := Compilation/main.c Compilation/cli.c
OBJ := $(SRC:.c=.o) $(COMPILATION_SRC:.c=.o)
BIN := machine
BUILD_DIR := build
RUNTIME_OBJ := $(BUILD_DIR)/machine_runtime.o

.PHONY: help Compilation clean install uninstall test

help:
	@echo "Machine project build targets"
	@echo ""
	@echo "  make Compilation   Build ./machine and precompiled runtime object"
	@echo "  make test          Build compiler and test all example sources"
	@echo "  make install       Install ./machine and runtime support files"
	@echo "  make uninstall     Remove installed Machine files"
	@echo "  make clean         Remove object files and built outputs"
	@echo ""
	@echo "Compiler usage:"
	@echo "  ./machine --help"
	@echo "  ./machine --version"
	@echo "  ./machine examples/hello.mne -o hello"

Compilation: $(BIN) $(RUNTIME_OBJ)
	@echo "Build complete: ./$(BIN)"

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(RUNTIME_OBJ): src/runtime.c include/machine_runtime.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RUNTIME_CFLAGS) -c src/runtime.c -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

Compilation/%.o: Compilation/%.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(BIN) $(RUNTIME_OBJ)
	install -d /usr/local/bin
	install -d /usr/local/lib/machine
	install -d /usr/local/include/machine
	install -m 0755 $(BIN) /usr/local/bin/$(BIN)
	install -m 0644 $(RUNTIME_OBJ) /usr/local/lib/machine/machine_runtime.o
	install -m 0644 include/machine_runtime.h /usr/local/include/machine/machine_runtime.h
	@echo "Installed Machine compiler and runtime files"

uninstall:
	rm -f /usr/local/bin/$(BIN)
	rm -f /usr/local/lib/machine/machine_runtime.o
	rm -f /usr/local/include/machine/machine_runtime.h
	@echo "Removed installed Machine files"

clean:
	rm -f $(OBJ) $(BIN)
	rm -rf $(BUILD_DIR)

test: $(BIN) $(RUNTIME_OBJ)
	./$(BIN) examples/hello.mne -o hello && ./hello
	./$(BIN) examples/strings_demo.mne -o strings_demo && ./strings_demo
	./$(BIN) examples/control_demo.mne -o control_demo && ./control_demo
	./$(BIN) examples/funcs_demo.mne -o funcs_demo && ./funcs_demo
	./$(BIN) examples/memory_demo.mne -o memory_demo && ./memory_demo
	./$(BIN) examples/list_demo.mne -o list_demo && ./list_demo
	./$(BIN) examples/math_demo.mne -o math_demo && ./math_demo
	./$(BIN) examples/module_demo.mne -o module_demo && ./module_demo
	./$(BIN) examples/nested_struct_demo.mne -o nested_struct_demo && ./nested_struct_demo
	./$(BIN) examples/grid_demo.mne -o grid_demo && ./grid_demo
	./$(BIN) examples/const_table_demo.mne -o const_table_demo && ./const_table_demo
	./$(BIN) examples/elif_blank_demo.mne -o elif_blank_demo && ./elif_blank_demo
