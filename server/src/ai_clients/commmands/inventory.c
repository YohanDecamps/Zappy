/*
** EPITECH PROJECT, 2024
** commmands
** File description:
** inventory
*/

#include "commands.h"
#include "server.h"

void ai_cmd_inventory(
    UNUSED server_t *server, ai_client_t *client, UNUSED char *args)
{
    ai_dprintf(client, "[");
    for (int i = FOOD; i < THYSTAME; ++i) {
        ai_dprintf(
            client, "%s%s %d", i ? ", " : "", r_name[i],
            client->res[i].quantity);
    }
    ai_dprintf(client, "]\n");
}
