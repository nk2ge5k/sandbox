ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SUBMODULES_DIR = $(ROOT_DIR)/submodules

LIBS = -I./src -I$(SUBMODULES_DIR)/stb $(shell pkg-config --cflags raylib libprotobuf-c)
CFLAGS  = -Wall -Wextra -Wno-unused -Wno-unused-parameter 
LDFLAGS = -L/usr/local/lib -lm $(shell pkg-config --libs raylib libprotobuf-c)
SOURCES = $(wildcard $(ROOT_DIR)/src/*.c) $(wildcard $(ROOT_DIR)/src/**/*.c)

BUILD := $(ROOT_DIR)/build/sandbox

OS = $(shell uname -s)
ifeq ($(OS),Linux)
	OPTS ?= -ggdb -fsanitize=address -static-libasan
endif
ifeq ($(OS),Darwin)
	OPTS ?= -fsanitize=address -static-libsan
endif

space :=
space +=
join-with = $(subst $(space),$1,$(strip $2))

$(BUILD):
	@mkdir -p $(ROOT_DIR)/build
	$(CC) -DDEBUG=1 -DSTB_DS_IMPLEMENTATION \
		$(LIBS) $(CFLAGS) $(OPTS) $(SOURCES) -o build/sandbox $(LDFLAGS)

build: $(BUILD) ## Build binary
.PHONY: build

clean: ## Clean project
	rm -rf $(ROOT_DIR)/build
.PHONY: clean

COMPILE_FLAGS := $(ROOT_DIR)/compile_flags.txt

$(COMPILE_FLAGS):
	@echo $(call join-with,"\n",$(LIBS)) > $(COMPILE_FLAGS)

# TODO(nk2ge5k): could i manage with ctags or something like that?
dump-commands: $(COMPILE_FLAGS) # Writes compile_flags.txt for clangd lsp
.PHONY: dump-commands

dump-sources: ## Print list of all source files
	@echo $(call join-with,"\n",$(SOURCES))
.PHONY: dump-sources

help: ## Show this help
	@echo "\nSpecify a command. The choices are:\n"
	@grep -hE '^[0-9a-zA-Z_-]+:.*?## .*$$' ${MAKEFILE_LIST} \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[0;36m%-20s\033[m %s\n", $$1, $$2}'
	@echo ""
.PHONY: help

.DEFAULT_GOAL := help
