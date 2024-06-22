/*
** EPITECH PROJECT, 2024
** src
** File description:
** array
*/

#pragma once

#include <stdlib.h>
#include <stdbool.h>

struct buffer_s {
    char *str;
    size_t size;
    size_t alloc;
};

typedef struct {
    int fd;
    struct buffer_s buffer;
} net_client_t;

bool net_disconnect(net_client_t *net);
void net_read(net_client_t *net);
char *net_getline(net_client_t *net);
void net_move_buffer(net_client_t *target, net_client_t *source);

__attribute__((format(printf, 2, 3)))
ssize_t net_dprintf(net_client_t *net, const char *fmt, ...);
ssize_t net_write(net_client_t *net, const char *str, size_t n);
