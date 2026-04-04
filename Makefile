SHELL := /bin/bash
ENV_VAR := PANDORA
DEST := $(HOME)/.$(ENV_VAR)
BIN_DIR := bin
SRC := src/main.c
TARGET := $(BIN_DIR)/pandora

WRAPPER_SRC := src/core
WRAPPER_BIN := $(BIN_DIR)

CC := $(shell command -v gcc 2>/dev/null || command -v clang 2>/dev/null)
ifeq ($(CC),)
$(error "No compiler found. ('gcc' or 'clang' required.)")
endif
CFLAGS := -Wall -Wextra -Iinclude

ifeq ($(shell basename $$SHELL),zsh)
    SHELL_RC := $(HOME)/.zshrc
else
    SHELL_RC := $(HOME)/.bashrc
endif

.PHONY: all setup build wrappers env dirs clean

all: setup build wrappers env dirs

setup:
	@echo "Ensuring project is at '$(DEST)'..."
	@if [ ! -d "$(DEST)" ]; then \
		mkdir -p "$(DEST)"; \
	fi
	@if [ "$$PWD" != "$(DEST)" ]; then \
		PROJECT_NAME=$$(basename "$$PWD"); \
		mv "$$PWD" "$(DEST)/"; \
	fi
	@mkdir -p $(DEST)/$(BIN_DIR)

build:
	@echo "Compiling 'main.c' and 'abstract.c'..."
	@$(CC) $(CFLAGS) src/main.c ext/abstract.c -o $(DEST)/$(TARGET)
	@echo "Build complete: $(DEST)/$(TARGET)"

wrappers:
	@echo "Building compiler wrappers..."
	@mkdir -p $(DEST)/$(BIN_DIR)
    @$(CC) -Iinclude $(WRAPPER_SRC)/pcc.c ext/abstract.c -o $(DEST)/$(BIN_DIR)/pcc
    @$(CC) -Iinclude $(WRAPPER_SRC)/pc++.c ext/abstract.c -o $(DEST)/$(BIN_DIR)/pc++
	@chmod +x $(DEST)/$(BIN_DIR)/pcc
	@chmod +x $(DEST)/$(BIN_DIR)/pc++
	@echo "Wrappers built as 'pcc', 'pc++'."

dirs:
	@echo "Creating $PANDORA subdirectories..."
	@mkdir -p $(DEST)/include
	@mkdir -p $(DEST)/lib
	@mkdir -p $(DEST)/etc
	@echo "Directories ready as 'include', 'lib', and 'etc'."

env:
	@echo "Configuring shell environment..."
	@if ! grep -q '^export $(ENV_VAR)=' $(SHELL_RC); then \
		echo 'export $(ENV_VAR)="$(DEST)"' >> $(SHELL_RC); \
	fi
	@if ! grep -q 'export PATH=.*$(BIN_DIR)' $(SHELL_RC); then \
		echo 'export PATH="$(DEST)/$(BIN_DIR):$$PATH"' >> $(SHELL_RC); \
	fi
	@echo "Setup complete. Restart shell or run 'source $(SHELL_RC)'."

clean:
	@echo "Cleaning build..."
	@rm -rf $(DEST)/$(BIN_DIR)/pandora
	@rm -rf $(DEST)/$(BIN_DIR)/pcc
	@rm -rf $(DEST)/$(BIN_DIR)/pc++
