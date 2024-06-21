/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_bct
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_bct(server_t *server, gui_client_t *client, char *args)
{
    char **cmd = NULL;
    cell_t *cell = NULL;

    if (client == NULL)
        return;
    cmd = my_str_to_word_array(args, " ");
    if (cmd == NULL || !cmd[0] || !cmd[1])
        return;
    cell = CELL(server, atoi(cmd[0]), atoi(cmd[1]));
    gui_dprintf(
        client, "bct %d %d %d %d %d %d %d %d %d\n", cell->pos.x, cell->pos.y,
        cell->res[FOOD].quantity, cell->res[LINEMATE].quantity,
        cell->res[DERAUMERE].quantity, cell->res[SIBUR].quantity,
        cell->res[MENDIANE].quantity, cell->res[PHIRAS].quantity,
        cell->res[THYSTAME].quantity);
    free_array((void **)cmd);
}
