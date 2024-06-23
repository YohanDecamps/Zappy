##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

PY ?= python3

ifeq ($(DEBUG),1)
CMAKEFLAGS ?= -DCMAKE_BUILD_TYPE=Debug
PYFLAGS += -e
else
CMAKEFLAGS ?= -DCMAKE_BUILD_TYPE=Release
endif
CMAKEFLAGS += -DCMAKE_INSTALL_PREFIX=./ -DCMAKE_INSTALL_BINDIR=./

DEFAULT_GOAL := all

.PHONY: all
.NOTPARALLEL: all # Because the pc is gonna die if we dont
all: zappy_server zappy_gui zappy_ai

.PHONY: zappy_server
zappy_server:
	cmake -S server -B .build/server ${CMAKEFLAGS}
	cmake --build .build/server --target install

.PHONY: zappy_gui
zappy_gui:
	cmake -S GUI -B .build/gui ${CMAKEFLAGS}
	cmake --build .build/gui --target install

.PHONY: zappy_ai
zappy_ai:
	$(PY) -m venv .venv
	.venv/bin/pip install $(PYFLAGS) ./AI
	ln -fs .venv/bin/zappy_ai zappy_ai

.PHONY: clean
clean:
	$(RM) zappy_ai zappy_gui zap_server
	$(RM) -r .build lib64 include .venv

.PHONY: re
.NOTPARALLEL: re
re: clean all
