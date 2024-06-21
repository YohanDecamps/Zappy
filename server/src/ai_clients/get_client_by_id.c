/*
** EPITECH PROJECT, 2024
** ai_clients
** File description:
** get_client_by_id
*/

#include "server.h"

egg_t *get_egg_by_id(const server_t *server, int egg_id)
{
    egg_t *current = NULL;

    for (size_t i = 0; i < server->eggs.nb_elements; ++i) {
        current = server->eggs.elements[i];
        if (current->id == egg_id)
            return current;
    }
    return NULL;
}

ai_client_t *get_client_by_id(const server_t *server, int client_id)
{
    ai_client_t *current = NULL;

    for (size_t i = 0; i < server->ai_clients.nb_elements; ++i) {
        current = server->ai_clients.elements[i];
        if (current->id == client_id)
            return current;
    }
    return NULL;
}
