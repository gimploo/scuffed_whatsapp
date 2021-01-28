#include "common.h"
#include "linkedlist.h"

bool ll_append(struct list_header *list, Client *client)
{
    List_Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
        return false;
    new_node->client = client;
    new_node->next = NULL;

    if (list == NULL)
    {
        fprintf(stderr, "[ERR] ll_append: list argument is null\n");
        return false;
    }

    if (list->head == NULL || list->tail == NULL)
    {
        printf("[LOG] ll_append: head and tail changed\n");
        list->head = new_node;
        list->tail = new_node;
        return true;
    } 
    else 
    {
        printf("[LOG] ll_append: appended to list\n");
        list->tail->next = new_node;
        list->tail = new_node;
        return true;
    }
}

int ll_delete_node(struct list_header *list, Client *client)
{
    List_Node *this_node = list->head, *prev_node = NULL;

    if (list->head == NULL)
        return 1;

    if (list->head->client == client)
    {
        struct list_node *node = list->head;
        list->head = list->head->next;
        free(node);
        return 0;
    }

    while (this_node)
    {
        if (this_node->client == client)
        {
            if (this_node == list->tail)
            {
                list->tail = prev_node;
                prev_node->next = NULL;
                free(this_node);
                return 0;
            }
            prev_node->next = this_node->next; 
            free(this_node);
            return 0;
        }
        prev_node = this_node;
        this_node = this_node->next;
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
