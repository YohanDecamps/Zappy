/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** gui_cmd_pbc
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_pbc(
    UNUSED server_t *server, gui_client_t *client, int ai_id, char *m)
{
    if (client == NULL)
        return;
    gui_dprintf(client, "pbc %d %s\n", ai_id, m);
}
