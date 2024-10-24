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

static incantation_t *get_relatives(
    const server_t *serv, const ai_client_t *client, const cell_t *cell,
    size_t needed)
{
    incantation_t *inc = calloc(1, sizeof *inc);
    ai_client_t *read = NULL;
    int *id = NULL;

    if (!inc)
        return OOM, NULL;
    needed -= 1;
    for (size_t i = 0; i < serv->ai_clients.nb_elements && needed > 0; i++) {
        read = serv->ai_clients.elements[i];
        if (read->id == client->id || read->lvl != client->lvl ||
            cell->pos.x != read->pos.x || cell->pos.y != read->pos.y)
            continue;
        needed -= 1;
        id = malloc_int(read->id);
        if (id == NULL || add_elt_to_array(&inc->players, id) == RET_ERROR)
            return free(array_destructor(&inc->players, free)), OOM, NULL;
    }
    return inc;
}

static incantation_t *count_players(
    const server_t *serv, const ai_client_t *client, const cell_t *cell)
{
    int *id = NULL;
    int needed_players = INC_NEEDS[client->lvl - 1][PLAYER];
    incantation_t *inc = get_relatives(serv, client, cell, needed_players);

    if (!inc)
        return NULL;
    inc->lvl = client->lvl;
    inc->time = serv->now;
    if (needed_players - 1 > (int)inc->players.nb_elements)
        return free(array_destructor(&inc->players, free)),
            ERRF("l %d: too few clients", inc->lvl), NULL;
    inc->leader = client->id;
    id = malloc_int(client->id);
    if (id == NULL || add_elt_to_array(&inc->players, id) == RET_ERROR)
        return free(array_destructor(&inc->players, free)), OOM, NULL;
    return inc;
}

static incantation_t *check_start_incantation(
    const server_t *server, const ai_client_t *client, const cell_t *cell)
{
    if (client->lvl >= 8 || client->lvl <= 0)
        return ERRF("invalid level %d", client->lvl), NULL;
    for (size_t i = 0; i < R_COUNT; ++i)
        if (cell->res[i].quantity < INC_NEEDS[client->lvl - 1][i])
            return ERRF(
                "l %d: missing %s: got %d need %d", client->lvl, r_name[i],
                cell->res[i].quantity, INC_NEEDS[client->lvl - 1][i]),
            NULL;
    return count_players(server, client, cell);
}

static bool check_end_incantation(
    server_t *server, cell_t *cell, const incantation_t *inc)
{
    int *id = NULL;

    if (inc->lvl >= 8 || inc->lvl <= 0)
        return ERRF("l %d: invalid level", inc->lvl), NULL;
    for (size_t i = 0; i < R_COUNT; ++i)
        if (cell->res[i].quantity < INC_NEEDS[inc->lvl - 1][i])
            return ERRF("l %d: you lost all your money", inc->lvl), false;
    for (size_t i = 0; i < inc->players.nb_elements; ++i) {
        id = inc->players.elements[i];
        if (id == NULL || get_client_by_id(server, *id) == NULL)
            return ERRF("l %d: friend gone, friendship lost", inc->lvl), false;
    }
    for (size_t i = 0; i < R_COUNT - 2; ++i) {
        cell->res[i].quantity -= INC_NEEDS[inc->lvl - 1][i];
        server->map_res[i].quantity -= INC_NEEDS[inc->lvl - 1][i];
    }
    return true;
}

static void log_promoted_players(server_t *server, incantation_t *inc)
{
    char buffer[40];
    ai_client_t *read = NULL;

    for (size_t i = 0; i < inc->players.nb_elements; ++i) {
        read = get_client_by_id(server, *(int *)inc->players.elements[i]);
        read->busy = false;
        read->lvl += 1;
        net_dprintf(&read->net, "Current level: %d\n", read->lvl);
        sprintf(buffer, "%d", read->id);
        gui_cmd_plv(server, server->gui_client, buffer);
    }
}

void ai_client_incantation_end(server_t *server, incantation_t *inc)
{
    char buffer[40];
    cell_t *cell = NULL;
    ai_client_t *leader = get_client_by_id(server, inc->leader);

    if (leader == NULL) {
        ERR("leader went missing mid-incantation");
        return;
    }
    cell = CELL(server, leader->pos.x, leader->pos.y);
    if (!check_end_incantation(server, cell, inc)) {
        gui_cmd_pie(server, server->gui_client, leader->pos, 0);
        net_dprintf(&leader->net, "ko\n");
        return;
    }
    ERRF("level passed: %d", inc->lvl + 1);
    sprintf(buffer, "%d %d", leader->pos.x, leader->pos.y);
    gui_cmd_bct(server, server->gui_client, buffer);
    gui_cmd_pie(server, server->gui_client, leader->pos, 1);
    log_promoted_players(server, inc);
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
        net_write(&client->net, "ko\n", 3);
        return;
    }
    ptr += sprintf(ptr, "%d %d %d", cell->pos.x, cell->pos.y, client->lvl);
    for (size_t i = 0; i < inc->players.nb_elements; ++i) {
        read = get_client_by_id(server, *(int *)inc->players.elements[i]);
        read->busy = true;
        ptr += sprintf(ptr, " %d", read->id);
    }
    gui_cmd_pic(server, server->gui_client, buffer);
    net_dprintf(&client->net, "Elevation underway\n");
}
