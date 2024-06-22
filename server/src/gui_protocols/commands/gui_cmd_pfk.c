/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pfk
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_pfk(UNUSED server_t *server, gui_client_t *gui, int ai_id)
{
    if (gui == NULL)
        return;
    net_dprintf(&gui->net, "pfk %d\n", ai_id);
}
