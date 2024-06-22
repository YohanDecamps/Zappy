/*
** EPITECH PROJECT, 2024
** src
** File description:
** server
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gui_protocols/commands/commands.h"
#include "server.h"

void refill_map(server_t *server, context_t *ctx)
{
    long spread = 0;
    size_t x = 0;
    size_t y = 0;

    if (ctx->freq <= 0 ||
        server->now - server->last_refill <= 20. / (double)ctx->freq)
        return;
    server->last_refill = server->now;
    for (size_t i = 0; i < R_COUNT - 2; ++i) {
        spread = (long)((double)ctx->map_size * DENSITIES[i]);
        spread -= server->map_res[i].quantity;
        for (long cell = 0; cell < spread; ++cell) {
            x = rand() % ctx->width;
            y = rand() % ctx->height;
            server->map[IDX(x, y, ctx->width, ctx->height)].res[i].quantity++;
            server->map_res[i].quantity++;
        }
    }
    gui_cmd_mct(server, server->gui_client, NULL);
}

int init_map(server_t *server, context_t *ctx)
{
    ctx->map_size = ctx->width * ctx->height;
    if (ctx->map_size == 0)
        return RET_ERROR;
    server->map = calloc(ctx->map_size, sizeof(cell_t));
    if (server->map == NULL)
        return OOM, RET_ERROR;
    refill_map(server, ctx);
    for (size_t i = 0; i < server->ctx.map_size; ++i)
        server->map[i].pos = (vector_t){i % ctx->width, i / ctx->width};
    for (size_t i = 0;
        i < server->ctx.names.nb_elements * server->ctx.client_nb; ++i)
        spawn_egg(server, "");
    return RET_VALID;
}

egg_t *spawn_egg(server_t *server, char *team)
{
    egg_t *egg = malloc(sizeof *egg);

    if (!egg)
        return OOM, NULL;
    egg->pos = (vector_t){
        .x = rand() % (int)server->ctx.width,
        .y = rand() % (int)server->ctx.height,
    };
    strcpy(egg->team, team);
    if (add_elt_to_array(&server->eggs, egg) == RET_ERROR)
        return OOM, free(egg), NULL;
    egg->id = server->egg_id;
    server->egg_id++;
    CELL(server, egg->pos.x, egg->pos.y)->res[EGG].quantity += 1;
    server->map_res[EGG].quantity += 1;
    return egg;
}
