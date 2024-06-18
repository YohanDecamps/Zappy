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
    look_payload_t *payload =
        malloc(sizeof *payload + (sizeof *payload->cell_content * size));

    if (payload == NULL)
        return OOM, NULL;
    payload->size = size;
    payload->cell_content = (payload_t *)(payload + 1);
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
        for (int y = client->pos.y - i; y < client->pos.y + i; ++y) {
            get_cell_payload(
                server, &(vector_t){client->pos.x - i, y},
                &payload->cell_content[(payload->idx)++]);
        }
    }
    return payload;
}

static look_payload_t *look_south(
    server_t *server, ai_client_t *client, look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int x = client->pos.x - i; x < client->pos.x + i + 1; ++x) {
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
        for (int y = client->pos.y - i; y < client->pos.y + i; ++y) {
            get_cell_payload(
                server, &(vector_t){client->pos.x + i, y},
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
    payload->idx++;
    switch (client->dir) {
        case NORTH:
            payload = look_north(server, client, payload);
            break;
        case EAST:
            payload = look_east(server, client, payload);
            break;
        case SOUTH:
            payload = look_south(server, client, payload);
            break;
        case WEST:
            payload = look_west(server, client, payload);
            break;
    }
    return payload;
}

void ai_cmd_look(server_t *server, ai_client_t *client, UNUSED char *args)
{
    look_payload_t *payload = look(server, client);
    payload_t *cell = NULL;
    bool space = false;

    if (payload == NULL) {
        write(client->s_fd, "[]\n", 3);
        return;
    }
    dprintf(client->s_fd, "[");
    for (size_t i = 0; i < payload->size; ++i) {
        cell = &payload->cell_content[i];
        for (short j = 0; j < R_COUNT; ++j) {
            for (int k = 0; k < cell->res[j].quantity; ++k) {
                dprintf(client->s_fd, "%s%s", (space) ? " " : "", r_name[j]);
                space = true;
            }
        }
        if (i != 0)
            dprintf(client->s_fd, ",");
    }
    dprintf(client->s_fd, "]\n");
}