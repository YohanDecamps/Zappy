/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pdr
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_pdr(
    UNUSED server_t *server, gui_client_t *gui, int ai_id, int r)
{
    if (gui == NULL)
        return;
    net_dprintf(&gui->net, "pdr %d %d\n", ai_id, r);
}
