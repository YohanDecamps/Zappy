/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_tna
*/

#include <stdio.h>

#include "commands.h"

void gui_cmd_tna(server_t *server, gui_client_t *client, UNUSED char *args)
{
    if (client == NULL)
        return;
    for (size_t i = 0; i < server->ctx.names.nb_elements; ++i)
        gui_dprintf(client, "tna %s\n", (char *)server->ctx.names.elements[i]);
}
