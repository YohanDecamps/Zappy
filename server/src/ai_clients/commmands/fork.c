/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ai_clients
*/

#include "../ai_internal.h"
#include "commands.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"

static void after_fork(
    server_t *server, ai_client_t *client, UNUSED char *args)
{
    egg_t *egg = spawn_egg(server, client->team);

    ai_write(client, "ok\n", 3);
    gui_cmd_enw(server, server->gui_client, egg, client->id);
}

void ai_cmd_fork(server_t *server, ai_client_t *client, UNUSED char *args)
{
    queue_add_cmd(server, client, &(queued_cmd_t){after_fork, 42, NULL});
    gui_cmd_pfk(server, server->gui_client, client->id);
}
