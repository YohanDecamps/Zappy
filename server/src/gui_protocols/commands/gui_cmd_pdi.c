/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pdi
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_pdi(UNUSED server_t *server, gui_client_t *gui, int ai_id)
{
    if (gui == NULL)
        return;
    net_dprintf(&gui->net, "pdi %d\n", ai_id);
}
