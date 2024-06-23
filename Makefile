##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

ifeq ($(DEBUG),1)
CMAKEFLAGS ?= -DCMAKE_BUILD_TYPE=Debug
else
CMAKEFLAGS ?= -DCMAKE_BUILD_TYPE=Release
endif
CMAKEFLAGS += -DCMAKE_INSTALL_PREFIX=./ -DCMAKE_INSTALL_BINDIR=./

DEFAULT_GOAL := all

.PHONY: all
.NOTPARALLEL: re # Because the pc is gonna die if we dont
all: zappy_server zappy_gui zappy_ai

.PHONY: zappy_server
zappy_server:
	cmake -S server -B .build/server ${CMAKEFLAGS}
	cmake --build .build/server --target install

.PHONY: zappy_gui
zappy_gui:
	cmake -S GUI -B .build/gui ${CMAKEFLAGS}
	cmake --build .build/gui --target install

# TODO
.PHONY: zappy_ai
zappy_ai:
	ln -fs AI/zappy_ai zappy_ai

.PHONY: clean
clean:
	rm -rf .build lib64 include

.PHONY: re
.NOTPARALLEL: re
re: clean all
