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

#include "buffer.h"
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

static void exec_gui_cmd(server_t *server, gui_client_t *client, char *buffer)
{
    char *content = NULL;
    struct gui_cmd_s *cmd = NULL;

    content = strchr(buffer, ' ');
    if (content != NULL) {
        *content = '\0';
        content++;
    } else {
        content = "";
    }
    cmd = get_gui_cmd(buffer);
    if (cmd == NULL || cmd->func == NULL) {
        ERR("cmd not found"), net_write(&client->net, "ko\n", 3);
        return;
    }
    cmd->func(server, client, content);
}

int remove_gui(server_t *server)
{
    gui_client_t *gui = server->gui_client;

    if (!gui)
        return RET_ERROR;
    net_disconnect(&gui->net);
    free(gui);
    server->gui_client = NULL;
    return RET_VALID;
}

int iterate_gui(server_t *server)
{
    gui_client_t *gui = server->gui_client;
    char *ptr = NULL;

    if (!gui)
        return RET_ERROR;
    if (gui->net.fd < 0)
        return remove_gui(server);
    for (; ITER_BUF(ptr, &gui->net);) {
        exec_gui_cmd(server, gui, ptr);
        free(ptr);
    }
    return RET_VALID;
}
