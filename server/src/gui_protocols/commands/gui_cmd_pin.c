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

void gui_cmd_pin(server_t *server, gui_client_t *gui, char *args)
{
    char **cmd = my_str_to_word_array(args, " \n#");
    int client_id = atoi(*cmd);
    ai_client_t *current = get_client_by_id(server, client_id);

    if (gui == NULL)
        return;
    if (!current) {
        net_dprintf(&gui->net, "ko\n");
        return;
    }
    net_dprintf(
        &gui->net, "pin %d %d %d %d %d %d %d %d %d %d\n", client_id,
        current->pos.x, current->pos.y, current->res[FOOD].quantity,
        current->res[LINEMATE].quantity, current->res[DERAUMERE].quantity,
        current->res[SIBUR].quantity, current->res[MENDIANE].quantity,
        current->res[PHIRAS].quantity, current->res[THYSTAME].quantity);
    free_array((void **)cmd);
}
