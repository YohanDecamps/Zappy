/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_edi
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_edi(UNUSED server_t *server, gui_client_t *gui, char *args)
{
    char **cmd = my_str_to_word_array(args, " ");
    int egg_id = 0;

    if (gui == NULL)
        return;
    if (!cmd || !cmd[0])
        return;
    egg_id = atoi(cmd[0]);
    net_dprintf(&gui->net, "edi %d\n", egg_id);
    free_array((void **)cmd);
}
