/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** incantation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../ai_internal.h"
#include "array.h"
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

static int *malloc_int(int v)
{
    int *res = malloc(sizeof *res);

    if (res == NULL)
        return NULL;
    *res = v;
    return res;
}

static incantation_t *count_players(
    const server_t *serv, const ai_client_t *client, const cell_t *cell)
{
    incantation_t *inc = calloc(1, sizeof *inc);
    ai_client_t *read = NULL;
    int *id = NULL;

    if (!inc)
        return OOM, NULL;
    for (size_t i = 0; i < serv->ai_clients.nb_elements; i++) {
        read = serv->ai_clients.elements[i];
        if (read->id == client->id || read->lvl != client->lvl ||
            cell->pos.x != read->pos.x || cell->pos.y != read->pos.y)
            continue;
        id = malloc_int(read->id);
        if (id == NULL || add_elt_to_array(&inc->players, id) == RET_ERROR)
            return free(array_destructor(&inc->players, free)), OOM, NULL;
    }
    if ((size_t)INC_NEEDS[client->lvl - 1][PLAYER] - 1 >
        inc->players.nb_elements)
        return free(array_destructor(&inc->players, free)),
            ERR("Too few clients"), NULL;
    inc->leader = client->id;
    id = malloc_int(client->id);
    if (id == NULL || add_elt_to_array(&inc->players, id) == RET_ERROR)
        return free(array_destructor(&inc->players, free)), OOM, NULL;
    return inc;
}

static incantation_t *check_start_incantation(
    const server_t *server, const ai_client_t *client, const cell_t *cell)
{
    incantation_t *inc = NULL;

    if (client->lvl >= 7 || client->lvl == 0)
        return NULL;
    for (size_t i = 0; i < R_COUNT; ++i)
        if (cell->res[i].quantity < INC_NEEDS[client->lvl - 1][i])
            return NULL;
    inc = count_players(server, client, cell);
    if (inc == NULL)
        return NULL;
    inc->lvl = client->lvl;
    inc->time = time(NULL);
    return inc;
}

static bool check_end_incantation(
    const server_t *server, const cell_t *cell, const incantation_t *inc)
{
    for (size_t i = 0; i < R_COUNT; ++i)
        if (cell->res[i].quantity < INC_NEEDS[inc->lvl - 1][i])
            return false;
    for (size_t i = 0; i < inc->players.nb_elements; ++i)
        if (get_client_by_id(server, *(int *)inc->players.elements[i]) == NULL)
            return false;
    return true;
}

static void consume_ressources(
    server_t *server, ai_client_t *client, cell_t *cell)
{
    int qty = 0;

    if (client->lvl >= 7 || client->lvl == 0)
        return;
    for (size_t i = 0; i < R_COUNT - 2; ++i) {
        qty = INC_NEEDS[client->lvl - 1][i];
        cell->res[i].quantity -= qty;
        server->map_res[i].quantity -= qty;
    }
}

void ai_client_incantation_end(
    server_t *server, ai_client_t *leader, incantation_t *inc)
{
    char buffer[40];
    cell_t *cell = CELL(server, leader->pos.x, leader->pos.y);
    ai_client_t *read = NULL;

    if (!check_end_incantation(server, cell, inc)) {
        gui_cmd_pie(server, server->gui_client, leader->pos, 0);
        ai_dprintf(leader, "ko\n");
        ERR("Uwu you lost all your money");
        return;
    }
    sprintf(buffer, "%d %d", leader->pos.x, leader->pos.y);
    gui_cmd_bct(server, server->gui_client, buffer);
    consume_ressources(server, leader, cell);
    for (size_t i = 0; i < inc->players.nb_elements; ++i) {
        read = get_client_by_id(server, *(int *)inc->players.elements[i]);
        read->busy = false;
        read->lvl += 1;
        ai_dprintf(read, "Current level: %d\n", read->lvl);
        gui_cmd_pie(server, server->gui_client, read->pos, 1);
        sprintf(buffer, "%d", read->id);
        gui_cmd_plv(server, server->gui_client, buffer);
    }
}

void ai_cmd_incantation(
    server_t *server, ai_client_t *client, UNUSED char *args)
{
    cell_t *cell = CELL(server, client->pos.x, client->pos.y);
    ai_client_t *read = NULL;
    char buffer[4096];
    char *ptr = buffer;
    incantation_t *inc = check_start_incantation(server, client, cell);

    if (inc == NULL ||
        add_elt_to_array(&server->incantations, inc) == RET_ERROR) {
        ai_write(client, "ko\n", 3);
        ERR("Skill issue");
        return;
    }
    ptr += sprintf(ptr, "%d %d %d", cell->pos.x, cell->pos.y, client->lvl);
    for (size_t i = 0; i < inc->players.nb_elements; ++i) {
        read = get_client_by_id(server, *(int *)inc->players.elements[i]);
        read->busy = true;
        ptr += sprintf(ptr, " %d", read->id);
    }
    gui_cmd_pic(server, server->gui_client, buffer);
    ai_dprintf(client, "Elevation underway\n");
}
