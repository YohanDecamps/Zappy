/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_sst
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_sst(server_t *server, gui_client_t *client, char *args)
{
    long freq = 0;

    ERR("sst");
    if (client == NULL || args == NULL)
        return;
    freq = strtol(args, NULL, 10);
    if (freq <= 0) {
        ERR("invalid freq");
        gui_dprintf(client, "ko\n");
        return;
    }
    server->ctx.freq = freq;
    gui_dprintf(client, "sst %ld\n", freq);
}
