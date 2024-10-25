/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_plv
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_plv(server_t *server, gui_client_t *gui, char *args)
{
    char **cmd = my_str_to_word_array(args, " ");
    int id = (cmd && *cmd) ? atoi(*cmd) : -1;
    ai_client_t *current = get_client_by_id(server, id);

    if (gui == NULL || current == NULL)
        return;
    net_dprintf(&gui->net, "plv %d %d\n", id, current->lvl);
}
