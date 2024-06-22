/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pic
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_pic(UNUSED server_t *server, gui_client_t *gui, char *args)
{
    if (gui == NULL)
        return;
    net_dprintf(&gui->net, "pic %s\n", args);
}
