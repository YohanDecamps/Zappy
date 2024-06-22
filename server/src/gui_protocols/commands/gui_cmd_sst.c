/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_sst
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_sst(server_t *server, gui_client_t *gui, char *args)
{
    long freq = 0;

    if (gui == NULL || args == NULL)
        return;
    freq = strtol(args, NULL, 10);
    if (freq <= 0) {
        ERR("invalid freq");
        net_dprintf(&gui->net, "ko\n");
        return;
    }
    server->ctx.freq = freq;
    net_dprintf(&gui->net, "sst %ld\n", freq);
}
