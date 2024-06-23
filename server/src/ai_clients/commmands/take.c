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

void ai_cmd_take(server_t *server, ai_client_t *client, char *args)
{
    res_name_t res = get_ressource_type(args);
    cell_t *cell = CELL(server, client->pos.x, client->pos.y);
    char buff[256] = {0};

    if ((int)res == -1 || cell->res[res].quantity == 0) {
        net_write(&client->net, "ko\n", 3);
        return;
    }
    cell->res[res].quantity--;
    server->map_res[res].quantity--;
    client->res[res].quantity++;
    net_write(&client->net, "ok\n", 3);
    sprintf(buff, "%d", client->id);
    gui_cmd_pgt(server, server->gui_client, client->id, res);
    gui_cmd_mct(server, server->gui_client, NULL);
    gui_cmd_pin(server, server->gui_client, buff);
}
