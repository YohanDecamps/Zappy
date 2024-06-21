/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_enw
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_enw(
    UNUSED server_t *server, gui_client_t *client, egg_t *egg, int ai_id)
{
    if (client == NULL)
        return;
    gui_dprintf(
        client, "enw %d %d %d %d\n", egg->id, ai_id, egg->pos.x, egg->pos.y);
}
