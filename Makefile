ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SUBMODULES_DIR = $(ROOT_DIR)/submodules

CFLAGS  = -Wall -Wextra -Wno-unused -ggdb \
					-I./src \
					-I/usr/include \
					-I/usr/local/include \
					-I$(SUBMODULES_DIR) \
					-I$(SUBMODULES_DIR)/stb \
					-fsanitize=address -static-libasan

LDFLAGS = -L/usr/local/lib -lm -lraylib -lprotobuf-c

SOURCES = $(wildcard $(ROOT_DIR)/src/*.c)
PROTO_SOURCES = $(wildcard $(ROOT_DIR)/src/proto/*.pb-c.c)

BUILD := $(ROOT_DIR)/build/sandbox

$(BUILD):
	@mkdir -p $(ROOT_DIR)/build
	@$(CC) $(CFLAGS) $(SOURCES) $(PROTO_SOURCES) -o build/sandbox $(LDFLAGS)

build: $(BUILD) ## Build binary
.PHONY: build

clean: ## Clean project
	rm -rf $(ROOT_DIR)/build
.PHONY: clean

help: ## Show this help
	@echo "\nSpecify a command. The choices are:\n"
	@grep -hE '^[0-9a-zA-Z_-]+:.*?## .*$$' ${MAKEFILE_LIST} \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[0;36m%-20s\033[m %s\n", $$1, $$2}'
	@echo ""
.PHONY: help

.DEFAULT_GOAL := help
