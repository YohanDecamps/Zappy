/*
** EPITECH PROJECT, 2024
** ai_clients
** File description:
** get_client_by_id
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ai_internal.h"
#include "server.h"

static const int QUEUE_SIZE = 10;

bool queue_add_cmd(server_t *server, ai_client_t *client, queued_cmd_t *qcmd)
{
    if (client->q_cmds == NULL) {
        client->q_cmds = calloc(QUEUE_SIZE, sizeof *client->q_cmds);
        client->q_size = 0;
        if (client->q_cmds == NULL)
            return OOM, false;
    }
    if (client->q_size == QUEUE_SIZE)
        return ERR("Queue is full"), false;
    if (client->q_size == 0)
        client->last_cmd = server->now;
    client->q_cmds[client->q_size] = *qcmd;
    client->q_size += 1;
    return true;
}

void queue_pop_cmd(server_t *server, ai_client_t *client)
{
    queued_cmd_t qcmd;

    if (client->q_cmds == NULL || client->q_size == 0 ||
        server->ctx.freq <= 0 ||
        server->now - client->last_cmd <
            client->q_cmds[0].time / (double)server->ctx.freq)
        return;
    qcmd = client->q_cmds[0];
    client->q_size -= 1;
    memmove(
        client->q_cmds, client->q_cmds + 1,
        client->q_size * sizeof *client->q_cmds);
    qcmd.func(server, client, qcmd.args);
    client->last_cmd = server->now;
    if (qcmd.args != NULL)
        free(qcmd.args);
}
