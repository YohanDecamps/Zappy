/*
** EPITECH PROJECT, 2024
** src
** File description:
** server
*/

#include "server.h"

#include <bits/types/struct_timeval.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "arg_parse.h"
#include "array.h"
#include "gui_protocols/commands/commands.h"
#include "time.h"

const double DENSITIES[R_COUNT] = {
    [FOOD] = 0.5,
    [LINEMATE] = 0.3,
    [DERAUMERE] = 0.15,
    [SIBUR] = 0.1,
    [MENDIANE] = 0.1,
    [PHIRAS] = 0.08,
    [THYSTAME] = 0.05,
    [EGG] = 0,
    [PLAYER] = 0,
};

const char *const r_name[R_COUNT] = {
    [FOOD] = "food",
    [LINEMATE] = "linemate",
    [DERAUMERE] = "deraumere",
    [SIBUR] = "sibur",
    [MENDIANE] = "mendiane",
    [PHIRAS] = "phiras",
    [THYSTAME] = "thystame",
    [EGG] = "egg",
    [PLAYER] = "player",
};

res_name_t get_ressource_type(char *name)
{
    for (int i = 0; i < R_COUNT; i++)
        if (strcmp(name, r_name[i]) == 0)
            return i;
    return -1;
}

static int init_server(server_t *serv, int port)
{
    serv->s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv->s_fd == -1)
        return ERR("socket failed"), RET_ERROR;
    if (setsockopt(
            serv->s_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
        -1)
        return ERR("setsockopt failed"), RET_ERROR;
    serv->s_addr.sin_family = AF_INET;
    serv->s_addr.sin_port = htons(port);
    if (bind(
            serv->s_fd, (struct sockaddr *)&serv->s_addr,
            sizeof serv->s_addr) == -1)
        return ERR("bind failed"), RET_ERROR;
    listen(serv->s_fd, 10);
    return RET_VALID;
}

static void add_client(server_t *serv, int fd)
{
    int *fd_ptr = malloc(sizeof(int));

    if (!fd_ptr) {
        OOM;
        write(fd, "ko\n", 3);
        close(fd);
    }
    *fd_ptr = fd;
    if (add_elt_to_array(&serv->waitlist_fd, fd_ptr) == RET_ERROR) {
        write(fd, "ko\n", 3);
        close(fd);
        return;
    }
    dprintf(fd, "WELCOME\n");
}

static int new_client(server_t *serv)
{
    struct timeval tv = {.tv_sec = 0, .tv_usec = 1000};
    socklen_t len = sizeof serv->s_addr;
    fd_set fdread;
    fd_set fdwrite;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_SET(serv->s_fd, &fdread);
    FD_SET(serv->s_fd, &fdwrite);
    if (select(serv->s_fd + 1, &fdread, &fdwrite, NULL, &tv) <= 0)
        return -1;
    return accept(serv->s_fd, (struct sockaddr *)&serv->s_addr, &len);
}

static bool team_valid(server_t *serv, char *team)
{
    char *newline = strchr(team, '\n');

    if (newline != NULL) {
        *newline = '\0';
        if (team != newline && *(newline - 1) == '\r')
            *(newline - 1) = '\0';
    }
    for (size_t i = 0; i < serv->ctx.names.nb_elements; i++)
        if (strcmp(team, serv->ctx.names.elements[i]) == 0)
            return true;
    return false;
}

size_t count_team(server_t *serv, char *team)
{
    size_t count = 0;
    ai_client_t *ai_client = NULL;

    for (size_t i = 0; i < serv->ai_clients.nb_elements; ++i) {
        ai_client = (ai_client_t *)serv->ai_clients.elements[i];
        if (ai_client->s_fd > 0 && strcmp(team, ai_client->team) == 0)
            count += 1;
    }
    return count;
}

static long get_egg(server_t *serv, const char *team)
{
    egg_t *egg = NULL;

    for (size_t i = 0; i < serv->eggs.nb_elements; i++) {
        egg = serv->eggs.elements[i];
        if (egg->team == NULL || strcmp(team, egg->team) == 0)
            return (long)i;
    }
    return -1;
}

static void connect_ai_client(
    server_t *serv, size_t idx, int client_fd, char *team)
{
    long egg = -1;

    if (!team_valid(serv, team)) {
        write(client_fd, "ko\n", 3);
        return;
    }
    egg = get_egg(serv, team);
    if (count_team(serv, team) >= serv->ctx.client_nb || egg == -1) {
        ERR("team is full");
        write(client_fd, "ko\n", 3);
        return;
    }
    free(remove_elt_to_array(&serv->waitlist_fd, idx));
    if (init_ai_client(serv, client_fd, team, egg) == RET_ERROR) {
        write(client_fd, "ko\n", 3);
        close(client_fd);
    }
}

static void log_new_gui(server_t *serv)
{
    gui_cmd_msz(serv, serv->gui_client, "");
    gui_cmd_sgt(serv, serv->gui_client, "");
    gui_cmd_mct(serv, serv->gui_client, "");
    gui_cmd_tna(serv, serv->gui_client, "");
    for (size_t i = 0; i < serv->eggs.nb_elements; ++i)
        gui_cmd_enw(serv, serv->gui_client, serv->eggs.elements[i], -1);
    for (size_t i = 0; i < serv->ai_clients.nb_elements; ++i)
        gui_cmd_pnw(serv, serv->gui_client, serv->ai_clients.elements[i]);
}

static void connect_gui_client(server_t *serv, int client_fd)
{
    gui_client_t *gui = NULL;

    if (serv->gui_client != NULL) {
        write(client_fd, "ko\n", 3);
        ERR("GUI client is already connected");
        close(client_fd);
        return;
    }
    gui = calloc(1, sizeof *serv->gui_client);
    if (gui == NULL) {
        write(client_fd, "ko\n", 3);
        OOM;
        close(client_fd);
        return;
    }
    gui->s_fd = client_fd;
    serv->gui_client = gui;
    log_new_gui(serv);
}

static void handle_waitlist(server_t *serv, size_t i, int client_fd)
{
    char buffer[DEFAULT_SIZE];
    ssize_t bytes_read = 0;

    bytes_read = read(client_fd, buffer, DEFAULT_SIZE);
    if (bytes_read <= 0) {
        remove_ai_client(serv, i);
        return;
    }
    buffer[bytes_read] = '\0';
    if (strncmp(buffer, "GRAPHIC\n", bytes_read) == 0) {
        connect_gui_client(serv, client_fd);
    } else {
        connect_ai_client(serv, i, client_fd, buffer);
    }
}

static int iterate_waitlist(server_t *server)
{
    fd_set rfd;
    struct timeval timeout;
    int client_fd = 0;

    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i) {
        client_fd = *((int *)server->waitlist_fd.elements[i]);
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        FD_ZERO(&rfd);
        FD_SET(client_fd, &rfd);
        if (select(client_fd + 1, &rfd, NULL, NULL, &timeout) < 0)
            continue;
        if (FD_ISSET(client_fd, &rfd))
            handle_waitlist(server, i, client_fd);
    }
    return 0;
}

static void refill_map(server_t *server, context_t *ctx)
{
    long spread = 0;
    size_t x = 0;
    size_t y = 0;
    time_t now = time(NULL);

    if (ctx->freq <= 0 || now - server->last_refill <= 20 / ctx->freq)
        return;
    server->last_refill = now;
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

static int init_map(server_t *server, context_t *ctx)
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
        spawn_egg(server, NULL);
    return RET_VALID;
}

UNUSED static void debug_payload(look_payload_t *payload)
{
    for (size_t i = 0; i < payload->size; ++i) {
        printf("%d\n", payload->cell_content[i].res[FOOD].quantity);
    }
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
    egg->team = team;
    if (add_elt_to_array(&server->eggs, egg) == RET_ERROR)
        return OOM, free(egg), NULL;
    egg->id = server->egg_id;
    server->egg_id++;
    CELL(server, egg->pos.x, egg->pos.y)->res[EGG].quantity += 1;
    server->map_res[EGG].quantity += 1;
    return egg;
}

int server(int argc, char **argv)
{
    server_t server = {0};

    signal(13, SIG_IGN);
    srand(time(NULL));
    if (arg_parse(argc, argv, &server.ctx) != RET_VALID ||
        init_server(&server, server.ctx.port) != RET_VALID ||
        init_map(&server, &server.ctx) != RET_VALID)
        return RET_ERROR;
    for (int fd = -1;; fd = -1) {
        fd = new_client(&server);
        if (fd != -1)
            add_client(&server, fd);
        refill_map(&server, &server.ctx);
        iterate_waitlist(&server);
        iterate_ai_clients(&server);
        iterate_gui(&server);
    }
    // close_server(&serv); // gui_msg_seg
    return RET_VALID;
}
