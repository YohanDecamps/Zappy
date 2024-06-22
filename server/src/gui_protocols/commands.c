/*
** EPITECH PROJECT, 2024
** gui_protocols
** File description:
** commands
*/

#include "commands/commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui_internal.h"

static const struct gui_cmd_s commands[] = {
    {"bct", gui_cmd_bct},
    {"mct", gui_cmd_mct},
    {"msz", gui_cmd_msz},
    {"pin", gui_cmd_pin},
    {"plv", gui_cmd_plv},
    {"ppo", gui_cmd_ppo},
    {"sgt", gui_cmd_sgt},
    {"sst", gui_cmd_sst},
    {"tna", gui_cmd_tna},
};

const struct gui_cmd_s *const GUI_CLIENT_COMMANDS = commands;
const size_t GUI_CLIENT_COMMANDS_LEN = LEN(commands);

static int compare(const char *buffer, const struct gui_cmd_s *cmd)
{
    if (buffer == NULL || cmd == NULL || cmd->cmd == NULL)
        return 1;
    return strcmp(buffer, cmd->cmd);
}

static struct gui_cmd_s *get_gui_cmd(const char *cmd)
{
    return bsearch(
        cmd, GUI_CLIENT_COMMANDS, GUI_CLIENT_COMMANDS_LEN,
        sizeof *GUI_CLIENT_COMMANDS,
        (int (*)(const void *, const void *))compare);
}

static void exec_gui_cmd(server_t *server, gui_client_t *client)
{
    char *content = NULL;
    struct gui_cmd_s *cmd = NULL;

    content = strchr(client->buffer.str, ' ');
    if (content != NULL) {
        *content = '\0';
        content++;
    } else {
        content = "";
    }
    cmd = get_gui_cmd(client->buffer.str);
    if (cmd == NULL || cmd->func == NULL) {
        ERR("cmd not found"), gui_write(client, "ko\n", 3);
        return;
    }
    cmd->func(server, client, content);
}

static bool process_gui_cmd(server_t *server, gui_client_t *client)
{
    char *newline = strchr(client->buffer.str, '\n');
    size_t offset = 0;

    if (newline == NULL) {
        return false;
    }
    *newline = '\0';
    if (client->buffer.str != newline && *(newline - 1) == '\r')
        *(newline - 1) = '\0';
    exec_gui_cmd(server, client);
    offset = newline - client->buffer.str + 1;
    client->buffer.size -= offset;
    memmove(client->buffer.str, newline + 1, client->buffer.size);
    client->buffer.str[client->buffer.size] = '\0';
    return true;
}

static int resize_buffer(gui_client_t *client, size_t size)
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

static void handle_gui(server_t *server)
{
    const size_t bufsiz = 512;
    ssize_t bytes_read = 0;
    char *ptr = NULL;
    gui_client_t *gui = server->gui_client;

    if (resize_buffer(gui, bufsiz) == RET_ERROR) {
        ERR("resize_buffer"), gui_write(gui, "ko\n", 3);
        return;
    }
    ptr = gui->buffer.str + gui->buffer.size;
    bytes_read = read(gui->s_fd, ptr, bufsiz);
    if (bytes_read < 0) {
        ERR("read"), remove_gui(server);
        return;
    }
    gui->buffer.size += bytes_read;
    ptr[bytes_read] = '\0';
    for (; process_gui_cmd(server, gui);)
        ;
}

int remove_gui(server_t *server)
{
    gui_client_t *gui = server->gui_client;

    if (!gui)
        return RET_ERROR;
    if (gui->s_fd > 0)
        close(gui->s_fd);
    free(gui->buffer.str);
    free(gui);
    server->gui_client = NULL;
    return RET_VALID;
}

int iterate_gui(server_t *server)
{
    fd_set rfd;
    gui_client_t *gui = server->gui_client;

    if (!gui)
        return RET_ERROR;
    if (gui->s_fd < 0)
        return remove_gui(server);
    FD_ZERO(&rfd);
    FD_SET(gui->s_fd, &rfd);
    if (select(
            gui->s_fd + 1, &rfd, NULL, NULL,
            &(struct timeval){0, 0}) > 0 &&
        FD_ISSET(gui->s_fd, &rfd))
        handle_gui(server);
    return RET_VALID;
}
