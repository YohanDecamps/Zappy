/*
** EPITECH PROJECT, 2024
** src
** File description:
** look
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "server.h"

payload_t *get_cell_payload(server_t *serv, vector_t *pos, payload_t *payload)
{
    memcpy(
        payload->res,
        serv->map[IDX(pos->x, pos->y, serv->ctx.width, serv->ctx.height)].res,
        sizeof(payload->res));
    return payload;
}

static look_payload_t *init_look_payload(ai_client_t *client)
{
    size_t size = (size_t)(client->lvl + 1) * (client->lvl + 1);
    look_payload_t *payload = calloc(1, sizeof(look_payload_t));

    if (payload == NULL)
        return OOM, NULL;
    payload->cell_content = calloc(size, sizeof(payload_t));
    if (payload->cell_content == NULL)
        return OOM, free(payload), NULL;
    payload->size = size;
    for (size_t i = 0; i < payload->size; ++i) {
        for (size_t j = 0; j < R_COUNT; ++j) {
            payload->cell_content[i].res[j].r_name = j;
            payload->cell_content[i].res[j].quantity = 0;
        }
    }
    return payload;
}

static look_payload_t *look_north(
    server_t *server, ai_client_t *client, look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int x = client->pos.x - i; x < client->pos.x + i + 1; ++x) {
            get_cell_payload(
                server, &(vector_t){x, client->pos.y - i},
                &payload->cell_content[(payload->idx)++]);
        }
    }
    return payload;
}

static look_payload_t *look_east(
    server_t *server, ai_client_t *client, look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int y = client->pos.y - i; y < client->pos.y + i + 1; ++y) {
            get_cell_payload(
                server, &(vector_t){client->pos.x + i, y},
                &payload->cell_content[(payload->idx)++]);
        }
    }
    return payload;
}

static look_payload_t *look_south(
    server_t *server, ai_client_t *client, look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int x = client->pos.x + i; x > client->pos.x - i - 1; --x) {
            get_cell_payload(
                server, &(vector_t){x, client->pos.y + i},
                &payload->cell_content[(payload->idx)++]);
        }
    }
    return payload;
}

static look_payload_t *look_west(
    server_t *server, ai_client_t *client, look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int y = client->pos.y + i; y > client->pos.y - i - 1; --y) {
            get_cell_payload(
                server, &(vector_t){client->pos.x - i, y},
                &payload->cell_content[(payload->idx)++]);
        }
    }
    return payload;
}

static look_payload_t *look(server_t *server, ai_client_t *client)
{
    look_payload_t *payload = init_look_payload(client);

    if (payload == NULL)
        return NULL;
    get_cell_payload(server, &client->pos, &payload->cell_content[0]);
    payload->idx += 1;
    switch (client->dir) {
        case NORTH:
            return look_north(server, client, payload);
        case EAST:
            return look_east(server, client, payload);
        case SOUTH:
            return look_south(server, client, payload);
        case WEST:
            return look_west(server, client, payload);
        default:
            return payload;
    }
}

static void print_cell_content(
    ai_client_t *client, payload_t *cell, bool *space)
{
    for (short j = 0; j < R_COUNT; ++j) {
        for (int k = 0; k < cell->res[j].quantity; ++k) {
            net_dprintf(&client->net, "%s%s", (space) ? " " : "", r_name[j]);
            *space = true;
        }
    }
}

void ai_cmd_look(server_t *server, ai_client_t *client, UNUSED char *args)
{
    look_payload_t *payload = look(server, client);
    bool space = false;

    if (payload == NULL) {
        net_write(&client->net, "[]\n", 3);
        return;
    }
    net_dprintf(&client->net, "[");
    for (size_t i = 0; i < payload->size; ++i) {
        if (i != 0)
            net_dprintf(&client->net, ",");
        print_cell_content(client, &payload->cell_content[i], &space);
    }
    net_dprintf(&client->net, "]\n");
}
