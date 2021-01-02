#ifndef _LINKEDLIST_
#define _LINKEDLIST_

#include "common.h"

typedef struct node {
    void *data;
    struct node *next;
} Node;

int ll_insertion(Client **,Client *, uint32_t position);
int ll_deletion(Client **,Client *);
void ll_append(Client **,Client *);
void ll_free(Client *);

// Queue (very scuffed) not used
void ll_enqueue(Node **ll_head, Node **ll_tail, Node *client_node);
void * ll_dequeue(Node **llhead, Node **ll_tail);

#endif
