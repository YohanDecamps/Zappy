/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ai_clients
*/

#include <stdio.h>

#include "gui_protocols/commands/commands.h"
#include "commands.h"
#include "server.h"

void ai_cmd_eject(server_t *server, ai_client_t *client, UNUSED char *args)
{
    ai_client_t *current = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        current = server->ai_clients.elements[i];
        if (current == client)
            continue;
        move_ai_client(server, current, client->dir);
        net_dprintf(&current->net, "eject: %d\n", client->dir + 1);
    }
    CELL(server, client->pos.x, client->pos.y)->res[EGG].quantity = 0;
    net_write(&client->net, "ok\n", 3);
    gui_cmd_pex(server, server->gui_client, client->id);
}
