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

#include "buffer_internal.h"
#include "error.h"
#include "stdio.h"

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
