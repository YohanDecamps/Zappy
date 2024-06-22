/*
** EPITECH PROJECT, 2024
** include
** File description:
** server
*/

#pragma once

#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>

#include "array.h"
#include "buffer.h"
#include "error.h"

#define ATTR(x) __attribute__((x))
#define UNUSED ATTR(unused)
#define MOD(x, y) ((((x) % (y)) + (y)) % (y))
#define LEN(x) (sizeof(x) / sizeof *(x))

#define UNPACK(x) (x), sizeof(x)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define IDX(x, y, w, h) (MOD(y, h) * (w) + MOD(x, w))
#define R_COUNT 9
#define CELL(s, x, y) (&s->map[IDX(x, y, s->ctx.width, s->ctx.height)])

// This should be fine as this is long and it is 2024
#define GTIME(t) ((double)(t)->tv_sec + (double)(t)->tv_usec / 1000000)

typedef double precise_time_t;

enum {
    RET_VALID = 0,
    RET_ERROR = 84,
};

typedef enum {
    FOOD,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME,
    EGG,
    PLAYER,
} res_name_t;

typedef enum {
    S_EAST,
    S_NORTH_EAST,
    S_NORTH,
    S_NORTH_WEST,
    S_WEST,
    S_SOUTH_WEST,
    S_SOUTH,
    S_SOUTH_EAST
} sound_direction_t;

typedef struct {
    res_name_t r_name;
    int quantity;
} ressource_t;

typedef struct {
    int x;
    int y;
} vector_t;

typedef struct {
    vector_t pos;
    char team[512];
    int id;
} egg_t;

extern const double DENSITIES[R_COUNT];
extern const char *const r_name[R_COUNT];

struct cell_s {
    vector_t pos;
    ressource_t res[R_COUNT];
};

typedef struct cell_s cell_t;

typedef struct payload_s {
    ressource_t res[R_COUNT];
} payload_t;

typedef struct look_payload_s {
    size_t size;
    size_t idx;
    payload_t *cell_content;
} look_payload_t;

typedef struct context_s {
    int port;
    size_t width;
    size_t height;
    size_t map_size;
    array_t names;
    size_t client_nb;
    long freq;
} context_t;

typedef struct queued_cmd_s queued_cmd_t;

typedef struct ai_client_s {
    net_client_t net;
    char team[512];
    ressource_t res[R_COUNT];
    vector_t pos;
    enum {
        NORTH,
        EAST,
        SOUTH,
        WEST
    } dir;
    int lvl;
    int id;
    queued_cmd_t *q_cmds;
    size_t q_size;
    precise_time_t last_cmd;
    precise_time_t last_fed;
    bool busy;
} ai_client_t;

typedef struct {
    array_t players;
    precise_time_t time;
    int lvl;
    int leader;
} incantation_t;

static const sound_direction_t conv_table[] = {
    S_NORTH, S_EAST, S_SOUTH, S_WEST};

typedef struct gui_client_s {
    net_client_t net;
} gui_client_t;

typedef struct server_s {
    int s_fd;
    struct sockaddr_in s_addr;
    gui_client_t *gui_client;
    array_t ai_clients;
    array_t waitlist_fd;
    array_t eggs;
    cell_t *map;
    context_t ctx;
    int ai_id;
    int egg_id;
    ressource_t map_res[R_COUNT];
    precise_time_t last_refill;
    array_t incantations;
    precise_time_t now;
} server_t;

int server(UNUSED int argc, UNUSED char **argv);
payload_t *get_cell_payload(server_t *serv, vector_t *pos, payload_t *payload);
res_name_t get_ressource_type(char *name);
egg_t *spawn_egg(server_t *server, char *team);
size_t count_team(server_t *serv, char *team);
void refill_map(server_t *server, context_t *ctx);
int init_map(server_t *server, context_t *ctx);

int iterate_waitlist(server_t *server);

void iterate_ai_clients(server_t *server);
int init_ai_client(server_t *serv, int client_fd, char *team, size_t egg_idx);
int remove_ai_client(server_t *server, size_t idx);
void move_ai_client(server_t *server, ai_client_t *client, int dir);

int iterate_gui(server_t *server);
int remove_gui(server_t *server);

char **my_str_to_word_array(const char *str, char const *separator);
ai_client_t *get_client_by_id(const server_t *server, int client_id);
egg_t *get_egg_by_id(const server_t *server, int egg_id);
void free_array(void **array);
