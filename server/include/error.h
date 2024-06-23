/*
** EPITECH PROJECT, 2024
** include
** File description:
** server
*/

#pragma once

#include <unistd.h>

#include "server.h"

// Print error messages
#define __header __FILE_NAME__ ":" TOSTRING(__LINE__)
#define ERR(m) (void)!write(STDERR_FILENO, UNPACK(__header " (" m ")\n"))
#define ERRF(f, ...) dprintf(STDERR_FILENO, __header " (" f ")\n", __VA_ARGS__)
#define OOM ERR("Out of memory")
