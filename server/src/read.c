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

static int add_wt(server_t *server, net_client_t *wt)
{
    if (wt == NULL || wt->fd < 0)
        return -1;
    FD_SET(wt->fd, &server->fd_set.read);
    return wt->fd;
}

static int add_ai(server_t *server, ai_client_t *ai)
{
    if (ai == NULL || ai->net.fd < 0)
        return -1;
    FD_SET(ai->net.fd, &server->fd_set.read);
    return ai->net.fd;
}

static int add_gui(server_t *server, gui_client_t *gui)
{
    if (gui == NULL || gui->net.fd < 0)
        return -1;
    FD_SET(gui->net.fd, &server->fd_set.read);
    return gui->net.fd;
}

static int add_serv(server_t *server)
{
    if (server->s_fd < 0)
        return -1;
    FD_SET(server->s_fd, &server->fd_set.read);
    return server->s_fd;
}

void read_buffers(server_t *server)
{
    int max_fd = -1;

    FD_ZERO(&server->fd_set.read);
    FD_ZERO(&server->fd_set.write);
    FD_ZERO(&server->fd_set.except);
    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i)
        max_fd = MAX(max_fd, add_wt(server, server->waitlist_fd.elements[i]));
    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i)
        max_fd = MAX(max_fd, add_ai(server, server->ai_clients.elements[i]));
    max_fd = MAX(max_fd, add_gui(server, server->gui_client));
    max_fd = MAX(max_fd, add_serv(server));
    if (max_fd < 0)
        return;
    server->fd_set.select = select(
        max_fd + 1, &server->fd_set.read, &server->fd_set.write,
        &server->fd_set.except, &(struct timeval){0, 1000});
}
