/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_pgt
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"

void gui_cmd_pgt(server_t *server, gui_client_t *client, char *args)
{
    char **cmd = my_str_to_word_array(args, " \n#");
    int client_idx = atoi(cmd[0]);
    int res = atoi(cmd[1]);

    dprintf(client->s_fd, "pgt #%d %d\n", client_idx, res);
}