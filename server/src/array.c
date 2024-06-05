/*
** EPITECH PROJECT, 2024
** src
** File description:
** array
*/

#include "array.h"

#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int add_elt_to_array(array_t *array, void *element)
{
    void *tmp = NULL;

    if (array->nb_elements + 1 >= array->size) {
        tmp = reallocarray(
            array->elements, ((array->size) ? array->size * 2 : DEFAULT_SIZE),
            sizeof(void *));
        if (NULL == tmp)
            return RET_ERROR;
        array->elements = tmp;
        array->size *= 2;
    }
    array->elements[array->nb_elements] = element;
    array->nb_elements++;
    return RET_VALID;
}

int remove_elt_to_array(array_t *array, size_t i)
{
    void *tmp = array->elements[i];

    array->elements[i] = array->elements[array->nb_elements - 1];
    array->elements[array->nb_elements - 1] = tmp;
    array->nb_elements--;
    return SUCCESS;
}

array_t *array_constructor(void)
{
    array_t *array = malloc(sizeof(array_t));

    if (!array)
        return NULL;
    array->nb_elements = 0;
    array->size = DEFAULT_SIZE;
    array->elements = calloc(DEFAULT_SIZE, sizeof(void *));
    if (!array->elements)
        return NULL;
    return array;
}