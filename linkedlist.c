#include "common.h"
#include "linkedlist.h"

void ll_enqueue(Node **ll_head, Node **ll_tail, Node *client_node)
{
    if (*ll_tail == NULL) 
        *ll_head = client_node;
    else 
        (*ll_tail)->next = client_node;

    *ll_tail = client_node;
}

void ll_append(Client **llhead, Client *client_node)
{
    Client *tmp = *llhead, *prev_node = NULL;

    if (*llhead == NULL)
    {
        *llhead = client_node;
        return ;
    }
   
    while (tmp)
    {
        prev_node = tmp;
        tmp = tmp->next;
    }
    prev_node->next = client_node;
}

void * ll_dequeue(Node **llhead, Node **ll_tail)
{
    if (*llhead == NULL)
        return NULL;
    void *node = *llhead;
    *llhead = (*llhead)->next; 
    if (*llhead == NULL)
        ll_tail = NULL;
    return node;    
}

int ll_insertion(Client **llhead, Client *client_node, unsigned int position)
{
    int counter = 0;
    if (*llhead == NULL)
    {
        *llhead = client_node;
        return 0;
    }
    Client *tmp = *llhead, *prev_node = NULL;
    while (tmp != NULL)
    {
        if (position == counter++)
        {
            prev_node->next = client_node;
            return 0;
        }
        prev_node = tmp; 
        tmp = tmp->next;
    }
    if (position == (counter+1))
    {
        prev_node->next = client_node;
        return 0;
    }
    return 1;
}

int ll_deletion(Client **llhead, Client *client_node)
{
    Client *tmp = *llhead, *prev_node = NULL;

    if (*llhead == NULL)
        return 1;

    if (*llhead == client_node)
    {
        Client *node = *llhead;
        *llhead = (*llhead)->next;
        free(node);
        return 0;
    }

    while (tmp)
    {
        if (tmp == client_node)
        {
           prev_node->next = tmp->next; 
           free(tmp);
           return 0;
        }
        prev_node = tmp;
        tmp = tmp->next;
    }
    return 2;
}

void ll_free(Client *llhead)
{
    Client *prev_node = NULL;
    while (llhead != NULL)
    {
        prev_node = llhead;
        free(llhead);
        llhead = llhead->next;
    }
    llhead = NULL;
}
