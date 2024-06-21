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
#include <errno.h>

#include "server.h"

static char const *const flags[] = {
    "-p", "-x", "-y", "-n", "-c", "-f",
};

UNUSED static void debug_ctx(context_t *ctx)
{
    printf("Port: %d\n", ctx->port);
    printf("Width: %ld\n", ctx->width);
    printf("Height: %ld\n", ctx->height);
    for (size_t i = 0; i < ctx->names.nb_elements; ++i)
        printf("%s\n", (char *)ctx->names.elements[i]);
}

static int check_flags(const int *array, char *argv[])
{
    for (int i = 0; i < 6; ++i)
        if (strcmp(argv[array[i]], flags[i]) != 0)
            return RET_ERROR;
    return RET_VALID;
}

int arg_parse(int argc, char *argv[], context_t *ctx)
{
    int adjust = strcmp(argv[argc - 1], "-v") == 0
                || strcmp(argv[argc - 1], "--verbose") == 0;
    int array[6] = {1, 3, 5, 7, argc - 4 - adjust, argc - 2 - adjust};

    if (argc < 13 || check_flags(array, argv) != RET_VALID)
        return RET_ERROR;
    errno = 0;
    ctx->port = (int)strtol(argv[2], NULL, 10);
    ctx->width = strtol(argv[4], NULL, 10);
    ctx->height = strtol(argv[6], NULL, 10);
    ctx->client_nb = strtol(argv[argc - 3 - adjust], NULL, 10);
    ctx->freq = strtol(argv[argc - 1 - adjust], NULL, 10);
    if (errno != 0)
        return RET_ERROR;
    for (int i = 8; i < array[4]; ++i)
        if (add_elt_to_array(&ctx->names, argv[i]) == RET_ERROR)
            return RET_ERROR;
    debug_ctx(ctx);
    return RET_VALID;
}
