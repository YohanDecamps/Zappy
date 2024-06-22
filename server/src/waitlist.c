/*
** EPITECH PROJECT, 2024
** src
** File description:
** server
*/

#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"
#include "stdio.h"

static bool team_valid(server_t *serv, char *team)
{
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
    server_t *serv, net_client_t *client, char *team)
{
    long egg = -1;

    if (!team_valid(serv, team)) {
        ERR("team is invalid");
        net_write(client, "ko\n", 3);
        return;
    }
    egg = get_egg(serv, team);
    if (count_team(serv, team) >= serv->ctx.client_nb || egg == -1) {
        ERR("team is full");
        net_write(client, "ko\n", 3);
        return;
    }
    if (init_ai_client(serv, client->fd, team, egg) == RET_VALID) {
        client->fd = -1;
    } else {
        net_write(client, "ko\n", 3);
        net_disconnect(client);
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

static void connect_gui_client(server_t *serv, net_client_t *client)
{
    gui_client_t *gui = NULL;

    if (serv->gui_client != NULL) {
        net_write(client, "ko\n", 3);
        ERR("GUI client is already connected");
        net_disconnect(client);
        return;
    }
    gui = calloc(1, sizeof *gui);
    if (gui == NULL) {
        net_write(client, "ko\n", 3);
        OOM;
        net_disconnect(client);
        return;
    }
    gui->net.fd = client->fd;
    client->fd = -1;
    serv->gui_client = gui;
    log_new_gui(serv);
}

static void exec_wait_cmd(server_t *server, net_client_t *client, char *buffer)
{
    if (strcmp(buffer, "GRAPHIC") == 0) {
        connect_gui_client(server, client);
    } else {
        connect_ai_client(server, client, buffer);
    }
}

int iterate_waitlist(server_t *server)
{
    char *ptr = NULL;
    net_client_t *client = NULL;

    for (size_t i = 0; i < server->waitlist_fd.nb_elements; ++i) {
        client = server->waitlist_fd.elements[i];
        if (client->fd < 0) {
            net_disconnect(client);
            free(remove_elt_to_array(&server->waitlist_fd, i));
            i -= 1;
            continue;;
        }
        for (;;) {
            ptr = net_getline(client);
            if (ptr == NULL)
                break;
            exec_wait_cmd(server, client, ptr);
            free(ptr);
        }
    }
    return 0;
}
