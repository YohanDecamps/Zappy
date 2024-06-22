/*
** EPITECH PROJECT, 2024
** gui_protocols
** File description:
** gui_internal
*/

#pragma once

#include "server.h"

typedef void (*gui_cmd_func_t)(server_t *, gui_client_t *, char *);

struct gui_cmd_s {
    char *cmd;
    gui_cmd_func_t func;
};

extern const struct gui_cmd_s *const GUI_CLIENT_COMMANDS;
extern const size_t GUI_CLIENT_COMMANDS_LEN;
