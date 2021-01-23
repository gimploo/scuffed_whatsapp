#include "common.h"
#include "linkedlist.h"

bool ll_append(List *list, Client *client_node)
{
    if (list == NULL)
    {
        fprintf(stderr, "[ERR] ll_append: list argument is null\n");
        return false;
    }

    if (list->head == NULL || list->tail == NULL)
    {
        list->head = client_node;
        list->tail = client_node;
        printf("[LOG] ll_append: head and tail is NULL\n");
        return true;
    } 
    else 
    {
        printf("[LOG] ll_append: appended to list\n");
        list->tail->next_client = client_node;
        list->tail = client_node;
        return true;
    }
}

int ll_delete_node(List *list, Client *client_node)
{
    Client **head = &list->head;
    Client **tail = &list->tail;
    Client *tmp = list->head, *prev_node = NULL;

    if (*head == NULL)
        return 1;

    if (*head == client_node)
    {
        Client *node = *head;
        *head = (*head)->next_client;
        free(node);
        return 0;
    }

    while (tmp)
    {
        if (tmp == client_node)
        {
            if (tmp == *tail)
            {
                *tail = prev_node;
                prev_node->next_client = NULL;
                free(tmp);
                return 0;
            }
            prev_node->next_client = tmp->next_client; 
            free(tmp);
            return 0;
        }
        prev_node = tmp;
        tmp = tmp->next_client;
    }
    return 2;
}

void ll_free(List *list)
{
    Client *prev_node = NULL;
    while (list->head != NULL)
    {
        prev_node = list->head;
        list->head = list->head->next_client;
        free(prev_node);
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}
