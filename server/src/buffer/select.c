/*
** EPITECH PROJECT, 2024
** src
** File description:
** server
*/

#include <bits/types/struct_timeval.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "array.h"
#include "buffer.h"
#include "buffer_internal.h"
#include "server.h"
#include "time.h"

static int set_fd(net_client_t *net, fd_set *fds)
{
    if (net == NULL || net->fd < 0)
        return -1;
    FD_SET(net->fd, fds);
    return net->fd;
}

static void read_fd(net_client_t *net, fd_set *rfd)
{
    if (net == NULL || net->fd < 0 || !FD_ISSET(net->fd, rfd))
        return;
    FD_CLR(net->fd, rfd);
    net_read(net);
}

static int add_serv_fd(server_t *server, fd_set *rfd)
{
    if (server->s_fd < 0)
        return -1;
    FD_SET(server->s_fd, rfd);
    return server->s_fd;
}

static void read_serv(server_t *server, fd_set *rfd)
{
    if (server->s_fd < 0 || !FD_ISSET(server->s_fd, rfd))
        return;
    FD_CLR(server->s_fd, rfd);
    server->incoming_connection = true;
}

static int fill_fd_sets(
    server_t *serv, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    int max_fd = -1;
    ai_client_t *ai = NULL;
    gui_client_t *gui = serv->gui_client;

    for (size_t i = 0; i < serv->waitlist_fd.nb_elements; ++i)
        max_fd = MAX(max_fd, set_fd(serv->waitlist_fd.elements[i], rfd));
    for (size_t i = 0; i < serv->ai_clients.nb_elements; ++i) {
        ai = serv->ai_clients.elements[i];
        if (ai != NULL)
            max_fd = MAX(max_fd, set_fd(&ai->net, rfd));
    }
    if (gui != NULL)
        max_fd = MAX(max_fd, set_fd(&gui->net, rfd));
    max_fd = MAX(max_fd, add_serv_fd(serv, rfd));
    return max_fd;
}

static void read_fds(
    server_t *serv, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    ai_client_t *ai = NULL;
    gui_client_t *gui = serv->gui_client;

    for (size_t i = 0; i < serv->waitlist_fd.nb_elements; ++i)
        read_fd(serv->waitlist_fd.elements[i], rfd);
    for (size_t i = 0; i < serv->ai_clients.nb_elements; ++i) {
        ai = serv->ai_clients.elements[i];
        if (ai != NULL)
            read_fd(&ai->net, rfd);
    }
    if (gui != NULL)
        read_fd(&gui->net, rfd);
    read_serv(serv, rfd);
}

void read_buffers(server_t *server)
{
    int max_fd = -1;
    fd_set rfd = {0};
    fd_set wfd = {0};
    fd_set efd = {0};

    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_ZERO(&efd);
    max_fd = fill_fd_sets(server, &rfd, &wfd, &efd);
    if (max_fd < 0 ||
        select(max_fd + 1, &rfd, &wfd, &efd, &(struct timeval){0, 1000}) <= 0)
        return;
    read_fds(server, &rfd, &wfd, &efd);
}
