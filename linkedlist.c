#include "common.h"
#include "linkedlist.h"

bool ll_append(struct list_header *list, struct list_node *node)
{
    if (list == NULL)
    {
        fprintf(stderr, "[ERR] ll_append: list argument is null\n");
        return false;
    }

    if (list->head == NULL || list->tail == NULL)
    {
        list->head = node;
        list->tail = node;
        printf("[LOG] ll_append: head and tail is NULL\n");
        return true;
    } 
    else 
    {
        printf("[LOG] ll_append: appended to list\n");
        list->tail->next = node;
        list->tail = node;
        return true;
    }
}

int ll_delete_node(struct list_header *list, struct list_node *node)
{
    struct list_node **head = &list->head;
    struct list_node **tail = &list->tail;
    struct list_node *tmp = list->head, *prev_node = NULL;

    if (*head == NULL)
        return 1;

    if (*head == node)
    {
        struct list_node *node = *head;
        *head = (*head)->next;
        free(node);
        return 0;
    }

    while (tmp)
    {
        if (tmp == node)
        {
            if (tmp == *tail)
            {
                *tail = prev_node;
                prev_node->next = NULL;
                free(tmp);
                return 0;
            }
            prev_node->next = tmp->next; 
            free(tmp);
            return 0;
        }
        prev_node = tmp;
        tmp = tmp->next;
    }
    return 2;
}

void ll_free(struct list_header *list)
{
    struct list_node *prev_node = NULL;
    while (list->head != NULL)
    {
        prev_node = list->head;
        list->head = list->head->next;
        free(prev_node);
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

bool ll_is_node_in_list(struct list_header *list, struct list_node *node)
{
    struct list_node *track = list->head;
    if (list->head == node)
        return true;
    else if (list->tail == node)
        return true;
        
    track = track->next;
    while (track)
    {
        if (track == node)
            return true;
        track = track->next;
    }
    return false;
}
