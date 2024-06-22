/*
** EPITECH PROJECT, 2024
** src
** File description:
** array
*/

#include "buffer.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "stdio.h"

const size_t BUFFER_SIZE = 512;

bool net_disconnect(net_client_t *net)
{
    if (!net)
        return false;
    if (net->fd > 0)
        ERRF("Disconnecting client %d", net->fd), close(net->fd);
    free(net->buffer.str);
    memset(net, 0, sizeof *net);
    net->fd = -1;
    return true;
}

char *net_getline(net_client_t *net)
{
    char *ptr = NULL;
    char *newline = NULL;
    size_t len = 0;

    if (net->buffer.str == NULL)
        return NULL;
    newline = strchr(net->buffer.str, '\n');
    if (newline == NULL)
        return NULL;
    *newline = '\0';
    if (net->buffer.str != newline && *(newline - 1) == '\r')
        *(newline - 1) = '\0';
    len = newline - net->buffer.str + 1;
    ptr = strndup(net->buffer.str, len);
    net->buffer.size -= len;
    memmove(net->buffer.str, newline + 1, net->buffer.size);
    net->buffer.str[net->buffer.size] = '\0';
    return ptr;
}

static bool resize_buffer(net_client_t *net)
{
    char *tmp = NULL;

    if (net->buffer.str == NULL) {
        net->buffer.str = calloc(BUFFER_SIZE, sizeof(char));
        if (net->buffer.str == NULL)
            return OOM, false;
        net->buffer.alloc = BUFFER_SIZE;
        return true;
    }
    if (net->buffer.size + BUFFER_SIZE >= net->buffer.alloc) {
        tmp = realloc(net->buffer.str, net->buffer.alloc + BUFFER_SIZE);
        if (tmp == NULL)
            return OOM, false;
        net->buffer.str = tmp;
        net->buffer.alloc += BUFFER_SIZE;
    }
    return true;
}

void net_read(net_client_t *net)
{
    ssize_t bytes_read = 0;
    char *ptr = NULL;

    if (!resize_buffer(net)) {
        net_disconnect(net);
        return;
    }
    ptr = net->buffer.str + net->buffer.size;
    bytes_read = read(net->fd, ptr, BUFFER_SIZE);
    if (bytes_read <= 0) {
        ERRF("read: %s", (bytes_read) ? strerror(errno) : "disconnected");
        net_disconnect(net);
    } else {
        net->buffer.size += bytes_read;
        ptr[bytes_read] = '\0';
    }
}

ssize_t net_write(net_client_t *net, const char *str, size_t n)
{
    ssize_t ret = 0;

    if (net->fd < 0)
        return -1;
    ret = write(net->fd, str, n);
    if (ret > 0)
        return ret;
    ERRF("Err writing to fd: %s", strerror(errno));
    net_disconnect(net);
    return ret;
}

ssize_t net_dprintf(net_client_t *net, const char *fmt, ...)
{
    va_list args;
    ssize_t ret = 0;

    if (net->fd < 0)
        return -1;
    va_start(args, fmt);
    ret = vdprintf(net->fd, fmt, args);
    va_end(args);
    if (ret > 0)
        return ret;
    ERR("Err writing to fd");
    net_disconnect(net);
    return ret;
}

void net_move_buffer(net_client_t *target, net_client_t *source)
{
    memcpy(target, source, sizeof *target);
    memset(source, 0, sizeof *source);
    source->fd = -1;
}
