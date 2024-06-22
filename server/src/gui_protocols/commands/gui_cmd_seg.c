/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_seg
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_seg(UNUSED server_t *server, gui_client_t *gui, char *args)
{
    if (args == NULL || gui == NULL)
        return;
    net_dprintf(&gui->net, "seg %s\n", args);
}
