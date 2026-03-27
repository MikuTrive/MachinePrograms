PROJECT := Machine
CC := gcc
CFLAGS := -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude
RUNTIME_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image 2>/dev/null)
RUNTIME_LIBS := $(shell pkg-config --libs sdl2 SDL2_image 2>/dev/null)

SRC := src/common.c src/util.c src/lexer.c src/parser_expr.c src/parser_stmt.c src/codegen.c src/asm_backend.c
COMPILATION_SRC := Compilation/main.c Compilation/cli.c
OBJ := $(SRC:.c=.o) $(COMPILATION_SRC:.c=.o)
BIN := machine
BUILD_DIR := build
RUNTIME_OBJ := $(BUILD_DIR)/machine_runtime.o
POINTER_RUNTIME_SRC := src/pointer.c
FREESTANDING_RUNTIME_SRC := src/runtime_freestanding.c
FREESTANDING_ENTRY_SRC := src/runtime_freestanding_entry.S
BAREMETAL_RUNTIME_SRC := src/runtime_baremetal.c
BAREMETAL_ENTRY_SRC := src/runtime_baremetal_entry.S
BAREMETAL_LINKER_SRC := src/runtime_baremetal_link.ld
PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib/machine
INCDIR := $(PREFIX)/include/machine
DOCDIR := $(PREFIX)/share/doc/machine
EXAMPLESDIR := $(PREFIX)/share/machine/examples
DOC_FILES := README.md README.zh-CN.md use_order.md use_order.zh-CN.md LICENSE
EXAMPLE_FILES := $(wildcard test/*.mne)

ifeq ($(strip $(SUDO_USER)),)
VIM_HOME := $(HOME)
else
VIM_HOME := $(shell getent passwd "$(SUDO_USER)" | cut -d: -f6)
endif
USER_VIMDIR := $(VIM_HOME)/.vim

.PHONY: help Compilation clean install uninstall test freestanding-demo asm-demo baremetal-demo vim-install vim-uninstall verify

help:
	@echo "Machine project build targets"
	@echo ""
	@echo "  make Compilation        Build ./machine and precompiled runtime object"
	@echo "  make test               Build compiler and test all bundled test sources"
	@echo "  make install            Install compiler, runtimes, docs, examples, and Vim files into the invoking user's ~/.vim"
	@echo "  make uninstall          Remove installed Machine system files and Vim files from the invoking user's ~/.vim"
	@echo "  make freestanding-demo  Build test/lowlevel.mne for freestanding-x86_64"
	@echo "  make asm-demo           Build test/lowlevel.mne with x86_64-asm backend"
	@echo "  make baremetal-demo     Build test/baremetal_hello.mne as baremetal ELF"
	@echo "  make vim-install        Install Vim syntax highlighting into ~/.vim"
	@echo "  make vim-uninstall      Remove Vim syntax highlighting from ~/.vim"
	@echo "  make clean              Remove object files and built outputs"
	@echo ""
	@echo "Compiler usage:"
	@echo "  ./machine --help"
	@echo "  ./machine --version"
	@echo "  ./machine test/struct.mne -o struct"

Compilation: $(BIN) $(RUNTIME_OBJ)
	@echo "Build complete: ./$(BIN)"

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(RUNTIME_OBJ): src/runtime.c src/pointer.c include/machine_runtime.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RUNTIME_CFLAGS) -c src/runtime.c -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

Compilation/%.o: Compilation/%.c
	$(CC) $(CFLAGS) -c $< -o $@

freestanding-demo: $(BIN)
	./$(BIN) test/lowlevel.mne -o test/lowlevel.freestanding --target freestanding-x86_64 --unsafe

asm-demo: $(BIN) $(RUNTIME_OBJ)
	./$(BIN) test/lowlevel.mne -o test/lowlevel.asm --backend x86_64-asm --unsafe

baremetal-demo: $(BIN)
	./$(BIN) test/baremetal_hello.mne -o test/baremetal_hello.elf --backend x86_64-asm --target baremetal-x86_64 --unsafe

vim-install:
	install -d $(USER_VIMDIR)/ftdetect $(USER_VIMDIR)/syntax $(USER_VIMDIR)/ftplugin
	install -m 0644 vim/ftdetect/machine.vim $(USER_VIMDIR)/ftdetect/machine.vim
	install -m 0644 vim/syntax/machine.vim $(USER_VIMDIR)/syntax/machine.vim
	install -m 0644 vim/ftplugin/machine.vim $(USER_VIMDIR)/ftplugin/machine.vim
	@echo "Installed Machine Vim syntax files into $(USER_VIMDIR)"

vim-uninstall:
	rm -f $(USER_VIMDIR)/ftdetect/machine.vim
	rm -f $(USER_VIMDIR)/syntax/machine.vim
	rm -f $(USER_VIMDIR)/ftplugin/machine.vim
	@echo "Removed Machine Vim syntax files from $(USER_VIMDIR)"

install: $(BIN) $(RUNTIME_OBJ)
	install -d $(BINDIR)
	install -d $(LIBDIR)
	install -d $(INCDIR)
	install -d $(DOCDIR)
	install -d $(EXAMPLESDIR)
	install -d $(USER_VIMDIR)/ftdetect $(USER_VIMDIR)/syntax $(USER_VIMDIR)/ftplugin
	install -m 0755 $(BIN) $(BINDIR)/$(BIN)
	install -m 0644 $(RUNTIME_OBJ) $(LIBDIR)/machine_runtime.o
	install -m 0644 include/machine_runtime.h $(INCDIR)/machine_runtime.h
	install -m 0644 $(POINTER_RUNTIME_SRC) $(LIBDIR)/pointer.c
	install -m 0644 $(POINTER_RUNTIME_SRC) $(LIBDIR)/machine_pointer.c
	install -m 0644 include/machine_runtime_freestanding.h $(INCDIR)/machine_runtime_freestanding.h
	install -m 0644 $(FREESTANDING_RUNTIME_SRC) $(LIBDIR)/machine_runtime_freestanding.c
	install -m 0644 $(FREESTANDING_ENTRY_SRC) $(LIBDIR)/machine_runtime_freestanding_entry.S
	install -m 0644 include/machine_runtime_baremetal.h $(INCDIR)/machine_runtime_baremetal.h
	install -m 0644 $(BAREMETAL_RUNTIME_SRC) $(LIBDIR)/machine_runtime_baremetal.c
	install -m 0644 $(BAREMETAL_ENTRY_SRC) $(LIBDIR)/machine_runtime_baremetal_entry.S
	install -m 0644 $(BAREMETAL_LINKER_SRC) $(LIBDIR)/machine_runtime_baremetal_link.ld
	install -m 0644 vim/ftdetect/machine.vim $(USER_VIMDIR)/ftdetect/machine.vim
	install -m 0644 vim/syntax/machine.vim $(USER_VIMDIR)/syntax/machine.vim
	install -m 0644 vim/ftplugin/machine.vim $(USER_VIMDIR)/ftplugin/machine.vim
	install -m 0644 $(DOC_FILES) $(DOCDIR)
	install -m 0644 $(EXAMPLE_FILES) $(EXAMPLESDIR)
	@echo "Installed Machine compiler, runtimes, Vim syntax files, examples, and documentation"
	@echo "Vim files were installed into $(USER_VIMDIR)"

uninstall:
	rm -f $(BINDIR)/$(BIN)
	rm -f $(LIBDIR)/machine_runtime.o
	rm -f $(INCDIR)/machine_runtime.h
	rm -f $(LIBDIR)/pointer.c
	rm -f $(LIBDIR)/machine_pointer.c
	rm -f $(INCDIR)/machine_runtime_freestanding.h
	rm -f $(LIBDIR)/machine_runtime_freestanding.c
	rm -f $(LIBDIR)/machine_runtime_freestanding_entry.S
	rm -f $(INCDIR)/machine_runtime_baremetal.h
	rm -f $(LIBDIR)/machine_runtime_baremetal.c
	rm -f $(LIBDIR)/machine_runtime_baremetal_entry.S
	rm -f $(LIBDIR)/machine_runtime_baremetal_link.ld
	rm -f $(USER_VIMDIR)/ftdetect/machine.vim $(USER_VIMDIR)/syntax/machine.vim $(USER_VIMDIR)/ftplugin/machine.vim
	rm -f $(addprefix $(DOCDIR)/,$(DOC_FILES))
	rm -f $(addprefix $(EXAMPLESDIR)/,$(notdir $(EXAMPLE_FILES)))
	@echo "Removed installed Machine system files"
	@echo "Removed Machine Vim syntax files from $(USER_VIMDIR)"

clean:
	rm -f $(OBJ) $(BIN)
	rm -rf $(BUILD_DIR)

test: $(BIN) $(RUNTIME_OBJ)
	@set -e; \
	for src in test/*.mne; do \
	  case "$$src" in test/baremetal_hello.mne|test/baremetal_smoke.mne) continue ;; esac; \
	  out="$${src%.mne}"; \
	  echo "Testing $$src"; \
	  ./$(BIN) "$$src" -o "$$out"; \
	  "./$$out"; \
	  rm -f "$$out"; \
	done

verify: clean Compilation test asm-demo freestanding-demo baremetal-demo
	@echo "Verification complete: compiler build, tests, asm, freestanding, and baremetal outputs all succeeded without warnings"
