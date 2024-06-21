/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pie
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_pie(
    UNUSED server_t *server, gui_client_t *client, vector_t pos, bool r)
{
    if (client == NULL)
        return;
    gui_dprintf(client, "pie %d %d %d\n", pos.x, pos.y, r);
}
