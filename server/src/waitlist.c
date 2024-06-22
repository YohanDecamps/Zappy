/*
** EPITECH PROJECT, 2024
** src
** File description:
** server
*/

#include <stdlib.h>
#include <string.h>

#include "gui_protocols/commands/commands.h"
#include "server.h"
#include "stdio.h"

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

static long get_egg(server_t *serv, const char *team)
{
    egg_t *egg = NULL;

    for (size_t i = 0; i < serv->eggs.nb_elements; i++) {
        egg = serv->eggs.elements[i];
        if (egg->team[0] == '\0' || strcmp(team, egg->team) == 0)
            return (long)i;
    }
    return -1;
}

static void connect_ai_client(
    server_t *serv, size_t idx, int client_fd, char *team)
{
    long egg = -1;

    if (!team_valid(serv, team)) {
        ERR("team is invalid");
        (void)!write(client_fd, "ko\n", 3);
        return;
    }
    egg = get_egg(serv, team);
    if (count_team(serv, team) >= serv->ctx.client_nb || egg == -1) {
        ERR("team is full");
        (void)!write(client_fd, "ko\n", 3);
        return;
    }
    free(remove_elt_to_array(&serv->waitlist_fd, idx));
    if (init_ai_client(serv, client_fd, team, egg) == RET_ERROR) {
        OOM;
        (void)!write(client_fd, "ko\n", 3);
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

static void connect_gui_client(server_t *serv, int idx, int client_fd)
{
    gui_client_t *gui = NULL;

    free(remove_elt_to_array(&serv->waitlist_fd, idx));
    if (serv->gui_client != NULL) {
        (void)!write(client_fd, "ko\n", 3);
        ERR("GUI client is already connected");
        close(client_fd);
        return;
    }
    gui = calloc(1, sizeof *serv->gui_client);
    if (gui == NULL) {
        (void)!write(client_fd, "ko\n", 3);
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
    if (bytes_read < 0) {
        free(remove_elt_to_array(&serv->waitlist_fd, i));
        ERR("waitlist client disconnected");
        return;
    }
    buffer[bytes_read] = '\0';
    if (strncmp(buffer, "GRAPHIC\n", bytes_read) == 0) {
        connect_gui_client(serv, i, client_fd);
    } else {
        connect_ai_client(serv, i, client_fd, buffer);
    }
}

int iterate_waitlist(server_t *server)
{
    fd_set rfd;
    int client_fd = 0;

    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i) {
        client_fd = *((int *)server->waitlist_fd.elements[i]);
        FD_ZERO(&rfd);
        FD_SET(client_fd, &rfd);
        if (select(client_fd + 1, &rfd, NULL, NULL, &(struct timeval){0}) < 0)
            continue;
        if (FD_ISSET(client_fd, &rfd))
            handle_waitlist(server, i, client_fd);
    }
    return 0;
}
