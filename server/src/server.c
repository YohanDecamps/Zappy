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
#include "sys/time.h"
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

static precise_time_t gettime(void)
{
    struct timeval tv = {0};

    gettimeofday(&tv, NULL);
    return GTIME(&tv);
}

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

static void add_client(server_t *serv, int fd)
{
    int *fd_ptr = malloc(sizeof(int));

    if (!fd_ptr) {
        OOM;
        write(fd, "ko\n", 3);
        close(fd);
        return;
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
        server.now = gettime();
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
