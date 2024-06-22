/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_sgt
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_sgt(server_t *server, gui_client_t *gui, UNUSED char *args)
{
    if (gui == NULL)
        return;
    net_dprintf(&gui->net, "sgt %ld\n", server->ctx.freq);
}
