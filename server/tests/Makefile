##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

DEFAULT_GOAL := all

.PHONY: all
all: afl_wrapper

.PHONY: run
run: afl_wrapper
	afl-fuzz -i ./input -o ./output -- ./afl_wrapper

.PHONY: afl_wrapper
afl_wrapper:
	afl-gcc-fast afl_wrapper.c -o $@

.PHONY: clean
clean:
	$(RM) afl_wrapper

.PHONY: re
.NOTPARALLEL: re
re: clean all
