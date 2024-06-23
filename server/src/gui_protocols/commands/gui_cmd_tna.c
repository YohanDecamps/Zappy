/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_tna
*/

#include <stdio.h>

#include "commands.h"

void gui_cmd_tna(server_t *server, gui_client_t *gui, UNUSED char *args)
{
    if (gui == NULL)
        return;
    for (size_t i = 0; i < server->ctx.names.nb_elements; ++i)
        net_dprintf(
            &gui->net, "tna %s\n", (char *)server->ctx.names.elements[i]);
}
