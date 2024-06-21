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

void gui_cmd_pin(server_t *server, gui_client_t *client, char *args)
{
    char **cmd = my_str_to_word_array(args, " \n#");
    int client_id = atoi(*cmd);
    ai_client_t *current = get_client_by_id(server, client_id);

    if (client == NULL)
        return;
    if (!current) {
        gui_dprintf(client, "ko\n");
        return;
    }
    gui_dprintf(
        client, "pin %d %d %d %d %d %d %d %d %d %d\n", client_id, current->pos.x,
        current->pos.y, current->res[FOOD].quantity,
        current->res[LINEMATE].quantity, current->res[DERAUMERE].quantity,
        current->res[SIBUR].quantity, current->res[MENDIANE].quantity,
        current->res[PHIRAS].quantity, current->res[THYSTAME].quantity);
    free_array((void **)cmd);
}
