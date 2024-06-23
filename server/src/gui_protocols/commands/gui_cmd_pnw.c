/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pnw
*/

#include <stdio.h>

#include "commands.h"
#include "server.h"

void gui_cmd_pnw(UNUSED server_t *server, gui_client_t *gui, ai_client_t *new)
{
    if (gui == NULL)
        return;
    net_dprintf(
        &gui->net, "pnw %d %d %d %d %d %s\n", new->id, new->pos.x, new->pos.y,
        new->dir + 1, new->lvl, new->team);
}
