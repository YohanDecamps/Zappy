/*
** EPITECH PROJECT, 2024
** src
** File description:
** arg_parse
*/

#include "arg_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "server.h"

UNUSED static void debug_ctx(context_t *ctx)
{
    printf("Port: %d\n", ctx->port);
    printf("Width: %zu\n", ctx->width);
    printf("Height: %zu\n", ctx->height);
    for (size_t i = 0; i < ctx->names.nb_elements; ++i)
        printf("%s\n", (char *)ctx->names.elements[i]);
}

static void handle_error(arg_parse_t *cringe, context_t *ctx)
{
    for (int i = 0; i < 5; ++i)
        if (cringe->f_values[i] == NULL)
            ERR("Missing flag"), exit(RET_ERROR);
    ctx->port = atoi(cringe->f_values[PORT]);
    ctx->width = atoi(cringe->f_values[WIDTH]);
    ctx->height = atoi(cringe->f_values[HEIGHT]);
    ctx->freq = atoi(cringe->f_values[FREQ]);
    ctx->client_nb = atoi(cringe->f_values[CLIENTS]);
    if (ctx->port <= 0 || ctx->width <= 0 || ctx->height <= 0 ||
        ctx->width > 300 || ctx->height > 300 || ctx->freq <= 0 ||
        ctx->client_nb <= 0 || ctx->names.nb_elements == 0)
        ERR("Invalid flags"), exit(RET_ERROR);
}

static void add_teams(context_t *ctx, int argc, char *argv[])
{
    if (add_elt_to_array(&ctx->names, optarg) == RET_ERROR)
        exit(RET_ERROR);
    for (; optind < argc && argv[optind][0] != '-'; ++optind)
        if (add_elt_to_array(&ctx->names, argv[optind]) == RET_ERROR)
            exit(RET_ERROR);
}

static void add_port(arg_parse_t *cringe)
{
    cringe->f_values[PORT] = optarg;
}

static void add_width(arg_parse_t *cringe)
{
    cringe->f_values[WIDTH] = optarg;
}

static void add_height(arg_parse_t *cringe)
{
    cringe->f_values[HEIGHT] = optarg;
}

static void add_freq(arg_parse_t *cringe)
{
    cringe->f_values[FREQ] = optarg;
}

static void add_clients(arg_parse_t *cringe)
{
    cringe->f_values[CLIENTS] = optarg;
}

int arg_parse(int argc, char *argv[], context_t *ctx)
{
    arg_parse_t cringe = {0};

    for (;;) {
        cringe.opt = getopt(argc, argv, "p:x:y:n:c:f:");
        if (cringe.opt == -1)
            break;
        if (cringe.opt >= 256 || FLAGS[cringe.opt] != NULL) {
            FLAGS[cringe.opt](&cringe);
            continue;
        }
        if (cringe.opt == 'n') {
            add_teams(ctx, argc, argv);
            continue;
        }
        fprintf(stderr, "Invalid flag.\n");
        exit(RET_ERROR);
    }
    handle_error(&cringe, ctx);
    return RET_VALID;
}

static void (*const s_flags[256])(arg_parse_t *) = {
    ['c'] = add_clients,
    ['f'] = add_freq,
    ['p'] = add_port,
    ['x'] = add_width,
    ['y'] = add_height};

void (*const *FLAGS)(arg_parse_t *) = s_flags;
