/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_mct
*/

#include <stdio.h>

#include "commands.h"
#include "server.h"

void gui_cmd_mct(server_t *server, gui_client_t *gui, UNUSED char *args)
{
    char bct_args[256];

    if (gui == NULL)
        return;
    for (size_t x = 0; x < server->ctx.width; ++x) {
        for (size_t y = 0; y < server->ctx.height; ++y) {
            snprintf(bct_args, sizeof(bct_args), "%ld %ld", x, y);
            gui_cmd_bct(server, gui, bct_args);
        }
    }
}
