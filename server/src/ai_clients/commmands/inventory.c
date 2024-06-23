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
    net_dprintf(&client->net, "[");
    for (int i = FOOD; i < THYSTAME; ++i) {
        net_dprintf(
            &client->net, "%s%s %d", i ? ", " : "", r_name[i],
            client->res[i].quantity);
    }
    net_dprintf(&client->net, "]\n");
}
