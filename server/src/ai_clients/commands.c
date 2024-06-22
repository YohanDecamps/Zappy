/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ai_clients
*/

#include "commmands/commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai_internal.h"
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

static void exec_ai_cmd(server_t *server, ai_client_t *client)
{
    char *content = NULL;
    struct ai_cmd_s *cmd = NULL;

    content = strchr(client->buffer.str, ' ');
    if (content != NULL) {
        *content = '\0';
        content++;
    } else {
        content = "";
    }
    cmd = get_ai_cmd(client->buffer.str);
    if (cmd == NULL || cmd->func == NULL) {
        gui_cmd_suc(server, server->gui_client);
        ai_write(client, "ko\n", 3);
        return;
    }
    queue_add_cmd(
        server, client,
        &(queued_cmd_t){cmd->func, cmd->time, strdup(content)});
}

static bool process_ai_cmd(server_t *server, ai_client_t *client)
{
    char *newline = strchr(client->buffer.str, '\n');
    size_t offset = 0;

    if (newline == NULL) {
        return false;
    }
    *newline = '\0';
    if (client->buffer.str != newline && *(newline - 1) == '\r')
        *(newline - 1) = '\0';
    exec_ai_cmd(server, client);
    offset = newline - client->buffer.str + 1;
    client->buffer.size -= offset;
    memmove(client->buffer.str, newline + 1, client->buffer.size);
    client->buffer.str[client->buffer.size] = '\0';
    return true;
}

static int resize_buffer(ai_client_t *client, size_t size)
{
    char *tmp = NULL;

    if (client->buffer.str == NULL) {
        client->buffer.str = calloc(size, sizeof(char));
        if (client->buffer.str == NULL)
            return OOM, RET_ERROR;
        client->buffer.alloc = size;
        return RET_VALID;
    }
    if (client->buffer.size + size >= client->buffer.alloc) {
        tmp = realloc(client->buffer.str, client->buffer.alloc + size);
        if (tmp == NULL)
            return OOM, RET_ERROR;
        client->buffer.str = tmp;
        client->buffer.alloc += size;
    }
    return RET_VALID;
}

void handle_ai_client(server_t *server, ai_client_t *client)
{
    const size_t bufsiz = 512;
    ssize_t bytes_read = 0;
    char *ptr = NULL;

    if (resize_buffer(client, bufsiz) == RET_ERROR) {
        ai_write(client, "ko\n", 3);
        return;
    }
    ptr = client->buffer.str + client->buffer.size;
    bytes_read = read(client->s_fd, ptr, bufsiz);
    if (bytes_read <= 0) {
        disconnect_ai_client(client);
        return;
    }
    client->buffer.size += bytes_read;
    ptr[bytes_read] = '\0';
    for (; process_ai_cmd(server, client);)
        ;
}
