/*
** EPITECH PROJECT, 2024
** include
** File description:
** utils
*/

#pragma once

#include <stddef.h>

#define DEFAULT_SIZE 1024
#define MAX_PATH 4096
#define SUCCESS 0
#define DISCONNECT 83
#define FAILURE 84

typedef struct array_s {
    size_t size;
    size_t nb_elements;
    void **elements;
} array_t;

int add_elt_to_array(array_t *array, void *element);
void *remove_elt_to_array(array_t *array, size_t i);
array_t *array_constructor(void);
void *array_destructor(array_t *array, void (*free_elt)(void *));
