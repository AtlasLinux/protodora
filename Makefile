SHELL := /bin/bash

ENV_VAR := PANDORA
STORE := $(HOME)/pandora/store
VIR := $(HOME)/pandora/vir
PROFILES := $(HOME)/pandora/profiles
MANIFESTS := $(HOME)/pandora/manifests
TMP := $(HOME)/pandora/tmp
CACHE := $(HOME)/pandora/cache

BIN_DIR := $(VIR)/bin
LIB_DIR := $(VIR)/lib
ETC_DIR := $(VIR)/etcetera

SRC := src/main.c
WRAPPER_SRC := src/core

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
	@echo "Setting up Pandora directories..."
	@mkdir -p $(STORE) $(VIR) $(PROFILES) $(MANIFESTS) $(TMP) $(CACHE)
	@mkdir -p $(BIN_DIR) $(LIB_DIR) $(ETC_DIR)
	@echo "Pandora directories ready."

build:
	@echo "Compiling main..."
	@$(CC) $(CFLAGS) src/main.c ext/abstract.c -o $(BIN_DIR)/pandora
	@chmod +x $(BIN_DIR)/pandora
	@echo "Build complete: $(BIN_DIR)/pandora"

wrappers:
	@echo "Building compiler wrappers..."
	@$(CC) -Iinclude $(WRAPPER_SRC)/pcc.c ext/abstract.c -o $(BIN_DIR)/pcc
	@$(CC) -Iinclude $(WRAPPER_SRC)/pc++.c ext/abstract.c -o $(BIN_DIR)/pc++
	@chmod +x $(BIN_DIR)/pcc $(BIN_DIR)/pc++
	@echo "Wrappers built: pcc, pc++"

dirs:
	@echo "Ensuring vir subdirectories exist..."
	@mkdir -p $(BIN_DIR) $(LIB_DIR) $(ETC_DIR)
	@echo "Directories ready: bin, lib, etcetera"

env:
	@echo "Configuring shell environment..."
	@if ! grep -q '^export $(ENV_VAR)=' $(SHELL_RC); then \
		echo 'export $(ENV_VAR)="$(VIR)"' >> $(SHELL_RC); \
	fi
	@if ! grep -q 'export PATH=.*pandora/vir/bin' $(SHELL_RC); then \
		echo 'export PATH="$(BIN_DIR):$$PATH"' >> $(SHELL_RC); \
	fi
	@echo "Setup complete. Restart shell or run 'source $(SHELL_RC)'."

clean:
	@echo "Cleaning build..."
	@rm -f $(BIN_DIR)/pandora $(BIN_DIR)/pcc $(BIN_DIR)/pc++
