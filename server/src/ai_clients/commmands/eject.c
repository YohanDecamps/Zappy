/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ai_clients
*/

#include <stdio.h>

#include "../../gui_protocols/commands/commands.h"
#include "commands.h"
#include "server.h"

void ai_cmd_eject(server_t *server, ai_client_t *client, UNUSED char *args)
{
    ai_client_t *current = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        current = server->ai_clients.elements[i];
        if (current->s_fd < 0 || current == client)
            continue;
        move_ai_client(server, current, client->dir);
        ai_dprintf(current, "eject: %d\n", client->dir + 1);
    }
    CELL(server, client->pos.x, client->pos.y)->res[EGG].quantity = 0;
    ai_write(client, "ok\n", 3);
    gui_cmd_pex(server, server->gui_client, client->id);
}
