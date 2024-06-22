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
#include "commmands/commands.h"
#include "gui_protocols/commands/commands.h"
#include "server.h"

// Here we define the commands in a static array
// so we can get the size at compile time.
// It needs to be sorted for the bsearch function.
static const struct ai_cmd_s commands[] = {
    {"Broadcast", ai_cmd_broadcast, 7.},
    {"Connect_nbr", ai_cmd_connect_nbr, 0.},
    {"Eject", ai_cmd_eject, 7.},
    {"Fork", ai_cmd_fork, 0.},
    {"Forward", ai_cmd_forward, 7.},
    {"Incantation", ai_cmd_incantation, 0.},
    {"Inventory", ai_cmd_inventory, 1.},
    {"Left", ai_cmd_left, 7.},
    {"Look", ai_cmd_look, 7.},
    {"Right", ai_cmd_right, 7.},
    {"Set", ai_cmd_set, 7.},
    {"Take", ai_cmd_take, 7.},
};

const struct ai_cmd_s *const AI_CLIENT_COMMANDS = commands;
const size_t AI_CLIENT_COMMANDS_LEN = LEN(commands);

static int compare(const char *buffer, const struct ai_cmd_s *cmd)
{
    if (buffer == NULL || cmd == NULL || cmd->cmd == NULL)
        return 1;
    return strcmp(buffer, cmd->cmd);
}

static struct ai_cmd_s *get_ai_cmd(const char *cmd)
{
    return bsearch(
        cmd, AI_CLIENT_COMMANDS, AI_CLIENT_COMMANDS_LEN,
        sizeof *AI_CLIENT_COMMANDS,
        (int (*)(const void *, const void *))compare);
}

static void exec_ai_cmd(server_t *server, ai_client_t *client, char *buffer)
{
    char *content = NULL;
    struct ai_cmd_s *cmd = NULL;

    content = strchr(buffer, ' ');
    if (content != NULL) {
        *content = '\0';
        content++;
    } else {
        content = "";
    }
    cmd = get_ai_cmd(buffer);
    if (cmd == NULL || cmd->func == NULL) {
        gui_cmd_suc(server, server->gui_client);
        net_write(&client->net, "ko\n", 3);
        return;
    }
    queue_add_cmd(
        server, client,
        &(queued_cmd_t){cmd->func, cmd->time, strdup(content)});
}

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
    egg_t *egg, server_t *serv, net_client_t *net, char *team)
{
    ai_client_t *client = calloc(1, sizeof *client);

    if (client == NULL)
        return OOM, NULL;
    strcpy(client->team, team);
    net_move_buffer(&client->net, net);
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

int init_ai_client(
    server_t *serv, net_client_t *net, char *team, size_t egg_idx)
{
    egg_t *egg = serv->eggs.elements[egg_idx];
    ai_client_t *client = create_client_obj(egg, serv, net, team);

    if (client == NULL)
        return RET_ERROR;
    remove_elt_to_array(&serv->eggs, egg_idx);
    CELL(serv, client->pos.x, client->pos.y)->res[PLAYER].quantity++;
    CELL(serv, client->pos.x, client->pos.y)->res[EGG].quantity--;
    serv->map_res[EGG].quantity--;
    gui_cmd_ebo(serv, serv->gui_client, egg);
    gui_cmd_pnw(serv, serv->gui_client, client);
    net_dprintf(
        &client->net, "%zu\n", serv->ctx.client_nb - count_team(serv, team));
    net_dprintf(&client->net, "%ld %ld\n", serv->ctx.width, serv->ctx.height);
    return RET_VALID;
}

int remove_ai_client(server_t *server, size_t idx)
{
    ai_client_t *client = NULL;

    if (idx >= server->ai_clients.nb_elements)
        return RET_ERROR;
    client = server->ai_clients.elements[idx];
    if (client) {
        gui_cmd_pdi(server, server->gui_client, client->id);
        net_write(&client->net, UNPACK("dead\n"));
        net_disconnect(&client->net);
        CELL(server, client->pos.x, client->pos.y)->res[PLAYER].quantity--;
        free(client->q_cmds);
    }
    free(remove_elt_to_array(&server->ai_clients, idx));
    return RET_VALID;
}

static bool starve_to_death(server_t *server, ai_client_t *ai)
{
    char buff[256] = {0};

    if (ai->res[FOOD].quantity <= 0)
        return ERR("Starved to death"), true;
    if (ai->last_fed == 0) {
        ai->last_fed = server->now;
        return false;
    }
    if (server->ctx.freq >= 0 &&
        server->now - ai->last_fed > 126. / (double)server->ctx.freq) {
        ai->res[FOOD].quantity -= 1;
        ai->last_fed = server->now;
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
            server->now - inc->time > 300. / (double)server->ctx.freq) {
            ai_client_incantation_end(server, inc);
            array_destructor(&inc->players, free);
            free(remove_elt_to_array(&server->incantations, i));
            i -= 1;
        }
    }
}

void iterate_ai_clients(server_t *server)
{
    ai_client_t *ai = NULL;
    char *ptr = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; i++) {
        ai = server->ai_clients.elements[i];
        if (!ai->busy)
            queue_pop_cmd(server, ai);
        if (ai->net.fd < 0 || starve_to_death(server, ai)) {
            remove_ai_client(server, i), i -= 1;
            continue;
        }
        if (server->fd_set.select > 0 &&
            FD_ISSET(ai->net.fd, &server->fd_set.read))
            net_read(&ai->net);
        for (; ITER_BUF(ptr, &ai->net);) {
            exec_ai_cmd(server, ai, ptr);
            free(ptr);
        }
    }
    summon_incantations(server);
}
