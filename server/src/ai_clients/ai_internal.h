/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ia_internal
*/

#pragma once

#include "server.h"

typedef void (*ai_cmd_func_t)(server_t *, ai_client_t *, char *);

struct ai_cmd_s {
    char *cmd;
    ai_cmd_func_t func;
    time_t time;
};

struct queued_cmd_s {
    ai_cmd_func_t func;
    time_t time;
    char *args;
};

extern const struct ai_cmd_s *const AI_CLIENT_COMMANDS;
extern const size_t AI_CLIENT_COMMANDS_LEN;

void disconnect_ai_client(ai_client_t *ai);
void handle_ai_client(server_t *server, ai_client_t *client);
bool queue_add_cmd(ai_client_t *client, queued_cmd_t *qcmd);
void queue_pop_cmd(server_t *server, ai_client_t *client);
void ai_client_incantation_end(server_t *server, incantation_t *inc);
