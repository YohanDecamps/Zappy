/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** incantation
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../ai_internal.h"
#include "commands.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"

static const int INC_NEEDS[7][R_COUNT] = {
    {
        // level 1 -> 2
        [PLAYER] = 1,
        [LINEMATE] = 1,
        [DERAUMERE] = 0,
        [SIBUR] = 0,
        [MENDIANE] = 0,
        [PHIRAS] = 0,
        [THYSTAME] = 0,
    },
    {
        // level 2 -> 3
        [PLAYER] = 2,
        [LINEMATE] = 1,
        [DERAUMERE] = 1,
        [SIBUR] = 1,
        [MENDIANE] = 0,
        [PHIRAS] = 0,
        [THYSTAME] = 0,
    },
    {
        // level 3 -> 4
        [PLAYER] = 2,
        [LINEMATE] = 2,
        [DERAUMERE] = 0,
        [SIBUR] = 1,
        [MENDIANE] = 0,
        [PHIRAS] = 2,
        [THYSTAME] = 0,
    },
    {
        // level 4 -> 5
        [PLAYER] = 4,
        [LINEMATE] = 1,
        [DERAUMERE] = 1,
        [SIBUR] = 2,
        [MENDIANE] = 0,
        [PHIRAS] = 1,
        [THYSTAME] = 0,
    },
    {
        // level 5 -> 6
        [PLAYER] = 4,
        [LINEMATE] = 1,
        [DERAUMERE] = 2,
        [SIBUR] = 1,
        [MENDIANE] = 3,
        [PHIRAS] = 0,
        [THYSTAME] = 0,
    },
    {
        // level 6 -> 7
        [PLAYER] = 6,
        [LINEMATE] = 1,
        [DERAUMERE] = 2,
        [SIBUR] = 3,
        [MENDIANE] = 0,
        [PHIRAS] = 1,
        [THYSTAME] = 0,
    },
    {
        // level 7 -> 8
        [PLAYER] = 6,
        [LINEMATE] = 2,
        [DERAUMERE] = 2,
        [SIBUR] = 2,
        [MENDIANE] = 2,
        [PHIRAS] = 2,
        [THYSTAME] = 1,
    },
};

static bool can_incantation(ai_client_t *client, const cell_t *cell)
{
    if (client->lvl >= 7 || client->lvl == 0)
        return false;
    for (size_t i = 0; i < R_COUNT; ++i)
        if (cell->res[i].quantity < INC_NEEDS[client->lvl - 1][i])
            return false;
    return true;
}

static void consume_ressources(
    server_t *server, ai_client_t *client, cell_t *cell, bool real_life)
{
    int qty = 0;

    if (client->lvl >= 7 || client->lvl == 0)
        return;
    for (size_t i = 0; i < R_COUNT - 2; ++i) {
        qty = INC_NEEDS[client->lvl - 1][i];
        cell->res[i].quantity -= qty;
        if (real_life)
            server->map_res[i].quantity -= qty;
    }
}

void ai_client_incantation_end(server_t *server, ai_client_t *client)
{
    char buffer[40];
    cell_t *cell = CELL(server, client->pos.x, client->pos.y);

    client->last_inc = 0;
    if (!can_incantation(client, cell)) {
        gui_cmd_pie(server, server->gui_client, client->pos, 0);
        ai_dprintf(client, "ko\n");
        ERR("Uwu you lost all your money");
        return;
    }
    consume_ressources(server, client, cell, true);
    client->lvl += 1;
    sprintf(buffer, "%d %d", client->pos.x, client->pos.y);
    gui_cmd_bct(server, server->gui_client, buffer);
    ai_dprintf(client, "Current level: %d\n", client->lvl);
    gui_cmd_pie(server, server->gui_client, client->pos, 1);
    sprintf(buffer, "%d", client->id);
    gui_cmd_plv(server, server->gui_client, buffer);
}

void ai_cmd_incantation(
    server_t *server, ai_client_t *client, UNUSED char *args)
{
    cell_t cell_cpy = *CELL(server, client->pos.x, client->pos.y);
    ai_client_t *read = NULL;
    char buffer[4096];
    char *cringe = buffer;
    time_t now = time(NULL);

    if (!can_incantation(client, &cell_cpy)) {
        ai_write(client, "ko\n", 3);
        ERR("Skill issue");
        return;
    }
    cringe += sprintf(
        buffer, "%d %d %d", cell_cpy.pos.x, cell_cpy.pos.y, client->lvl);
    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        read = server->ai_clients.elements[i];
        if (read->pos.x == client->pos.x && read->pos.y == client->pos.y &&
            client->lvl == read->lvl && can_incantation(read, &cell_cpy)) {
            consume_ressources(server, client, &cell_cpy, false);
            client->last_inc = now;
            cringe += sprintf(cringe, " %d", read->id);
        }
    }
    gui_cmd_pic(server, server->gui_client, buffer);
    ai_dprintf(client, "Elevation underway\n");
}
