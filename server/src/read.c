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
#include "server.h"
#include "time.h"

static int add_wt(
    net_client_t *wt, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (wt == NULL || wt->fd < 0)
        return -1;
    FD_SET(wt->fd, rfd);
    return wt->fd;
}

static void read_wt(
    net_client_t *wt, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (wt == NULL || wt->fd < 0 || !FD_ISSET(wt->fd, rfd))
        return;
    FD_CLR(wt->fd, rfd);
    net_read(wt);
}

static int add_ai(
    ai_client_t *ai, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (ai == NULL || ai->net.fd < 0)
        return -1;
    FD_SET(ai->net.fd, rfd);
    return ai->net.fd;
}

static void read_ai(
    ai_client_t *ai, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (ai == NULL || ai->net.fd < 0 || !FD_ISSET(ai->net.fd, rfd))
        return;
    FD_CLR(ai->net.fd, rfd);
    net_read(&ai->net);
}

static int add_gui(
    gui_client_t *gui, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (gui == NULL || gui->net.fd < 0)
        return -1;
    FD_SET(gui->net.fd, rfd);
    return gui->net.fd;
}

static void read_gui(
    gui_client_t *gui, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (gui == NULL || gui->net.fd < 0 || !FD_ISSET(gui->net.fd, rfd))
        return;
    FD_CLR(gui->net.fd, rfd);
    net_read(&gui->net);
}

static int add_serv(
    server_t *server, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (server->s_fd < 0)
        return -1;
    FD_SET(server->s_fd, rfd);
    return server->s_fd;
}

static void read_serv(
    server_t *server, fd_set *rfd, UNUSED fd_set *wfd, UNUSED fd_set *efd)
{
    if (server->s_fd < 0 || !FD_ISSET(server->s_fd, rfd))
        return;
    FD_CLR(server->s_fd, rfd);
    server->incoming_connection = true;
}

static int fill_fd_sets(
    server_t *server, fd_set *rfd, fd_set *wfd, fd_set *efd)
{
    int max_fd = -1;

    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i)
        max_fd = MAX(
            max_fd, add_wt(server->waitlist_fd.elements[i], rfd, wfd, efd));
    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i)
        max_fd =
            MAX(max_fd, add_ai(server->ai_clients.elements[i], rfd, wfd, efd));
    max_fd = MAX(max_fd, add_gui(server->gui_client, rfd, wfd, efd));
    max_fd = MAX(max_fd, add_serv(server, rfd, wfd, efd));
    return max_fd;
}

static void read_fds(server_t *server, fd_set *rfd, fd_set *wfd, fd_set *efd)
{
    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i)
        read_wt(server->waitlist_fd.elements[i], rfd, wfd, efd);
    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i)
        read_ai(server->ai_clients.elements[i], rfd, wfd, efd);
    read_gui(server->gui_client, rfd, wfd, efd);
    read_serv(server, rfd, wfd, efd);
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
