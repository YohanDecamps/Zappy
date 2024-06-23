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

static
void handle_error(arg_parse_t *cringe, context_t *ctx)
{
    for (int i = 0; i < 5; ++i)
        if (cringe->f_values[i] == NULL)
            exit(RET_ERROR);
    ctx->port = atoi(cringe->f_values[PORT]);
    ctx->width = atoi(cringe->f_values[WIDTH]);
    ctx->height = atoi(cringe->f_values[HEIGHT]);
    ctx->freq = atoi(cringe->f_values[FREQ]);
    ctx->client_nb = atoi(cringe->f_values[CLIENTS]);
    if (ctx->port <= 0 || ctx->width <= 0 ||
            ctx->height <= 0 || ctx->freq <= 0 || ctx->client_nb <= 0
                || ctx->names.nb_elements == 0)
        exit(RET_ERROR);
}

int arg_parse(int argc, char *argv[], context_t *ctx)
{
    arg_parse_t cringe = {0};

    while ((cringe.opt = getopt(argc, argv, "p:x:y:n:c:f:")) != -1) {
        switch (cringe.opt) {
            case 'p':
                cringe.f_values[PORT] = optarg;
                break;
            case 'x':
                cringe.f_values[WIDTH] = optarg;
                break;
            case 'y':
                cringe.f_values[HEIGHT] = optarg;
                break;
            case 'n':
                add_elt_to_array(&ctx->names, optarg);
                while (optind < argc && argv[optind][0] != '-')
                    add_elt_to_array(&ctx->names, argv[optind++]);
                break;
            case 'c':
                cringe.f_values[CLIENTS] = optarg;
                break;
            case 'f':
                cringe.f_values[FREQ] = optarg;
                break;
            default:
                fprintf(stderr, "Invalid flag.\n");
                exit(RET_ERROR);
        }
    }
    handle_error(&cringe, ctx);
    return RET_VALID;
}
