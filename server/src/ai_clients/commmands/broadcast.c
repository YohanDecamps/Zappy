/*
** EPITECH PROJECT, 2024
** src
** File description:
** broadcast
*/

#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "commands.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"

static int compute(const int a, const int b, const int grid_size)
{
    int result = 0;
    int direct_diff = b - a;
    int wrap_diff = grid_size - abs(direct_diff);

    if (direct_diff > 0) {
        result = (direct_diff <= grid_size / 2) ? direct_diff : -wrap_diff;
        return result;
    }
    result = (abs(direct_diff) <= grid_size / 2) ? direct_diff : wrap_diff;
    return result;
}

static int compute_dir(
    const vector_t p1, const vector_t p2, const int grid_size[2])
{
    int dx = compute(p1.x, p2.x, grid_size[0]);
    int dy = compute(p1.y, p2.y, grid_size[1]);

    if (abs(dx) == abs(dy)) {
        if (dy < 0 && dx > 0)
            return S_SOUTH_WEST;
        if (dy < 0 && dx < 0)
            return S_SOUTH_EAST;
        if (dy > 0 && dx > 0)
            return S_NORTH_WEST;
        if (dy > 0 && dx < 0)
            return S_NORTH_EAST;
    }
    if (abs(dy) >= abs(dx))
        return (dy < 0) ? S_SOUTH : S_NORTH;
    if (abs(dx) >= abs(dy))
        return (dx > 0) ? S_WEST : S_EAST;
    return -1;
}

static int get_dir(server_t *server, ai_client_t *client, ai_client_t *current)
{
    int dir = 0;
    int current_dir = 0;
    int computed_dir = 0;

    if (client->pos.x == current->pos.x && client->pos.y == current->pos.y)
        return 0;
    current_dir = conv_table[current->dir];
    computed_dir = compute_dir(
        client->pos, current->pos,
        (int[2]){(int)server->ctx.width, (int)server->ctx.height});
    dir = computed_dir - current_dir;
    if (dir < 0)
        dir += 7;
    dir += 1;
    return dir;
}

void ai_cmd_broadcast(server_t *server, ai_client_t *client, char *args)
{
    ai_client_t *current = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        current = server->ai_clients.elements[i];
        if (current != client)
            ai_dprintf(
                current, "message %d, %s\n", get_dir(server, client, current),
                args);
    }
    net_write(&client->net, "ok\n", 3);
    gui_cmd_pbc(server, server->gui_client, client->id, args);
}
