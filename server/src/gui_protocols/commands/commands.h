/*
** EPITECH PROJECT, 2024
** commands
** File description:
** commands
*/

#pragma once

#include "server.h"

void gui_cmd_msz(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_bct(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_mct(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_tna(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_pnw(server_t *server, gui_client_t *gui, ai_client_t *new);
void gui_cmd_ppo(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_plv(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_pin(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_pex(server_t *server, gui_client_t *gui, int ai_id);
void gui_cmd_pbc(server_t *server, gui_client_t *gui, int ai_id, char *m);
void gui_cmd_pic(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_pie(server_t *server, gui_client_t *gui, vector_t pos, bool r);
void gui_cmd_pfk(server_t *server, gui_client_t *gui, int ai_id);
void gui_cmd_pdr(server_t *server, gui_client_t *gui, int ai_id, int r);
void gui_cmd_pgt(server_t *server, gui_client_t *gui, int ai_id, int r);
void gui_cmd_pdi(server_t *server, gui_client_t *gui, int ai_id);
void gui_cmd_enw(server_t *server, gui_client_t *gui, egg_t *egg, int ai);
void gui_cmd_ebo(server_t *server, gui_client_t *gui, egg_t *egg);
void gui_cmd_edi(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_sgt(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_sst(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_seg(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_smg(server_t *server, gui_client_t *gui, char *args);
void gui_cmd_suc(server_t *server, gui_client_t *gui);
void gui_cmd_spb(server_t *server, gui_client_t *gui, char *args);
