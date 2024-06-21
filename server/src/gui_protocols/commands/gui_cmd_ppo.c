/*
** EPITECH PROJECT, 2024
** commands
** File description:
** gui_cmd_ppo
*/

#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "server.h"

void gui_cmd_ppo(server_t *server, gui_client_t *client, char *args)
{
    char **cmd = my_str_to_word_array(args, " ");
    int client_id = atoi(*cmd);
    ai_client_t *current = NULL;

    current = get_client_by_id(server, client_id);
    if (client == NULL || current == NULL)
        return;
    gui_dprintf(
        client, "ppo %d %d %d %d\n", client_id, current->pos.x, current->pos.y,
        current->dir + 1);
}
