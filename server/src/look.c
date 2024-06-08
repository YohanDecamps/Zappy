/*
** EPITECH PROJECT, 2024
** src
** File description:
** look
*/

#include "server.h"

payload_t *get_cell_payload(server_t *server, context_t *ctx, vector_t *pos)
{
    payload_t *payload = malloc(sizeof(payload_t));

    for (size_t i = 0; i < LEN(DENSITIES); ++i)
        payload->res[i] =
            server->map[IDX(pos->x, pos->y, ctx->width, ctx->height)].res[i];
    return payload;
}

static look_payload_t *init_look_payload(ai_client_t *client)
{
    look_payload_t *payload = malloc(sizeof(look_payload_t));

    if (payload == NULL)
        return NULL;
    payload->size = (size_t)(client->lvl + 1) * (client->lvl + 1);
    payload->cell_content = malloc(sizeof(payload_t) * payload->size);
    for (size_t i = 0; i < payload->size; ++i) {
        for (size_t j = 0; j < LEN(payload->cell_content[i].res); ++j) {
            payload->cell_content[i].res[j].r_name = j;
            payload->cell_content[i].res[j].quantity = 0;
        }
    }
    return payload;
}

static look_payload_t *look_north(
    server_t *server, context_t *ctx, ai_client_t *client,
    look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int x = client->pos.x - i; x < client->pos.x + i + 1; ++x) {
            payload->cell_content[(payload->idx)++] = *get_cell_payload(
                server, ctx, &(vector_t){x, client->pos.y - i});
        }
    }
    return payload;
}

static look_payload_t *look_east(
    server_t *server, context_t *ctx, ai_client_t *client,
    look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int y = client->pos.y - i; y < client->pos.y + i; ++y) {
            payload->cell_content[(payload->idx)++] = *get_cell_payload(
                server, ctx, &(vector_t){client->pos.x - i, y});
        }
    }
    return payload;
}

static look_payload_t *look_south(
    server_t *server, context_t *ctx, ai_client_t *client,
    look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int x = client->pos.x - i; x < client->pos.x + i + 1; ++x) {
            payload->cell_content[(payload->idx)++] = *get_cell_payload(
                server, ctx, &(vector_t){x, client->pos.y + i});
        }
    }
    return payload;
}

static look_payload_t *look_west(
    server_t *server, context_t *ctx, ai_client_t *client,
    look_payload_t *payload)
{
    for (int i = 1; i <= client->lvl; ++i) {
        for (int y = client->pos.y - i; y < client->pos.y + i; ++y) {
            payload->cell_content[(payload->idx)++] = *get_cell_payload(
                server, ctx, &(vector_t){client->pos.x + i, y});
        }
    }
    return payload;
}

look_payload_t *look(server_t *server, context_t *ctx, ai_client_t *client)
{
    look_payload_t *payload = init_look_payload(client);

    if (payload == NULL)
        return NULL;
    switch (client->dir) {
        case NORTH:
            payload = look_north(server, ctx, client, payload);
            break;
        case EAST:
            payload = look_east(server, ctx, client, payload);
            break;
        case SOUTH:
            payload = look_south(server, ctx, client, payload);
            break;
        case WEST:
            payload = look_west(server, ctx, client, payload);
            break;
    }
    return payload;
}
