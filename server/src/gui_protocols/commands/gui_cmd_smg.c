/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_smg
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_smg(UNUSED server_t *server, gui_client_t *gui, char *args)
{
    if (gui == NULL || args == NULL)
        return;
    net_dprintf(&gui->net, "smg %s\n", args);
}
