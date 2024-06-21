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
    size_t new = 0;

    if (array->nb_elements + 1 >= array->size) {
        new = (array->size > 1) ? array->size * 2 : DEFAULT_SIZE;
        tmp = reallocarray(array->elements, new, sizeof(void *));
        if (NULL == tmp)
            return OOM, RET_ERROR;
        array->elements = tmp;
        array->size = new;
    }
    array->elements[array->nb_elements] = element;
    array->nb_elements++;
    return RET_VALID;
}

void *remove_elt_to_array(array_t *array, size_t i)
{
    void *tmp = array->elements[i];

    array->elements[i] = array->elements[array->nb_elements - 1];
    array->elements[array->nb_elements - 1] = NULL;
    array->nb_elements--;
    return tmp;
}

array_t *array_constructor(void)
{
    array_t *array = calloc(1, sizeof *array);

    if (!array)
        return OOM, NULL;
    array->size = DEFAULT_SIZE;
    array->elements = calloc(array->size, sizeof(void *));
    if (!array->elements)
        return OOM, free(array), NULL;
    return array;
}

void *array_destructor(array_t *array, void (*free_elt)(void *))
{
    if (array == NULL)
        return NULL;
    if (array->elements != NULL) {
        for (size_t i = 0; free_elt != NULL && i < array->nb_elements; i++)
            free_elt(array->elements[i]);
        free(array->elements);
        array->elements = NULL;
    }
    return array;
}
