/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ai_clients
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ai_internal.h"
#include "bits/types/struct_timeval.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"

void move_ai_client(server_t *server, ai_client_t *client, int dir)
{
    CELL(server, client->pos.x, client->pos.y)->res[PLAYER].quantity--;
    switch (dir) {
        case NORTH:
            client->pos.y = MOD(client->pos.y - 1, (int)server->ctx.height);
            break;
        case SOUTH:
            client->pos.y = MOD(client->pos.y + 1, (int)server->ctx.height);
            break;
        case EAST:
            client->pos.x = MOD(client->pos.x + 1, (int)server->ctx.width);
            break;
        case WEST:
            client->pos.x = MOD(client->pos.x - 1, (int)server->ctx.width);
            break;
        default:
            ERR("Shoudn't happen");
            break;
    }
    CELL(server, client->pos.x, client->pos.y)->res[PLAYER].quantity++;
}

static ai_client_t *create_client_obj(
    egg_t *egg, server_t *serv, int client_fd, char *team)
{
    ai_client_t *client = malloc(sizeof *client);

    if (client == NULL)
        return OOM, NULL;
    memset(client, 0, sizeof *client);
    strcpy(client->team, team);
    client->s_fd = client_fd;
    client->dir = rand() % 4;
    client->pos = egg->pos;
    client->lvl = 1;
    client->res[FOOD].quantity = 10;
    if (add_elt_to_array(&serv->ai_clients, client) == RET_ERROR)
        return free(client), OOM, NULL;
    client->id = serv->ai_id;
    serv->ai_id++;
    return client;
}

int init_ai_client(server_t *serv, int client_fd, char *team, size_t egg_idx)
{
    egg_t *egg = serv->eggs.elements[egg_idx];
    ai_client_t *client = create_client_obj(egg, serv, client_fd, team);

    if (client == NULL)
        return RET_ERROR;
    remove_elt_to_array(&serv->eggs, egg_idx);
    CELL(serv, client->pos.x, client->pos.y)->res[PLAYER].quantity++;
    CELL(serv, client->pos.x, client->pos.y)->res[EGG].quantity--;
    serv->map_res[EGG].quantity--;
    gui_cmd_ebo(serv, serv->gui_client, egg);
    gui_cmd_pnw(serv, serv->gui_client, client);
    ai_dprintf(client, "%zu\n", serv->ctx.client_nb - count_team(serv, team));
    ai_dprintf(client, "%ld %ld\n", serv->ctx.width, serv->ctx.height);
    return RET_VALID;
}

void disconnect_ai_client(ai_client_t *ai)
{
    if (!ai)
        return;
    if (ai->s_fd > 0)
        close(ai->s_fd);
    ai->s_fd = -1;
}

int remove_ai_client(server_t *server, size_t idx)
{
    ai_client_t *client = NULL;

    if (idx >= server->ai_clients.nb_elements)
        return RET_ERROR;
    client = server->ai_clients.elements[idx];
    if (client) {
        gui_cmd_pdi(server, server->gui_client, client->id);
        ai_write(client, UNPACK("dead\n"));
        disconnect_ai_client(client);
        CELL(server, client->pos.x, client->pos.y)->res[PLAYER].quantity--;
        free(client->buffer.str);
        free(client->q_cmds);
        free(server->ai_clients.elements[idx]);
    }
    remove_elt_to_array(&server->ai_clients, idx);
    return RET_VALID;
}

static bool starve_to_death(server_t *server, ai_client_t *ai)
{
    time_t now = time(NULL);
    char buff[256] = {0};

    if (ai->res[FOOD].quantity <= 0)
        return ERR("Starved to death"), true;
    if (ai->last_fed == 0) {
        ai->last_fed = now;
        return false;
    }
    if (server->ctx.freq >= 0 && now - ai->last_fed > 126 / server->ctx.freq) {
        ai->res[FOOD].quantity -= 1;
        ai->last_fed = now;
        sprintf(buff, "%d", ai->id);
        gui_cmd_pin(server, server->gui_client, buff);
    }
    if (ai->res[FOOD].quantity <= 0)
        return ERR("Starved to death"), true;
    return false;
}

static void summon_incantations(server_t *server)
{
    incantation_t *inc = NULL;

    for (size_t i = 0; i < server->incantations.nb_elements; ++i) {
        inc = server->incantations.elements[i];
        if (server->ctx.freq >= 0 &&
            time(NULL) - inc->time > 300 / server->ctx.freq) {
            ai_client_incantation_end(server, inc);
            array_destructor(&inc->players, free);
            free(remove_elt_to_array(&server->incantations, i));
            i -= 1;
        }
    }
}

void iterate_ai_clients(server_t *server)
{
    fd_set rfd;
    ai_client_t *client = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        client = server->ai_clients.elements[i];
        if (client->s_fd < 0 || starve_to_death(server, client)) {
            ERR("Disconnected"), remove_ai_client(server, i);
            continue;
        }
        if (!client->busy)
            queue_pop_cmd(server, client);
        FD_ZERO(&rfd);
        FD_SET(client->s_fd, &rfd);
        if (select(
                client->s_fd + 1, &rfd, NULL, NULL,
                &(struct timeval){0, 1000}) > 0 &&
            FD_ISSET(client->s_fd, &rfd))
            handle_ai_client(server, client);
    }
    summon_incantations(server);
}
