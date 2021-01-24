#ifndef _LINKEDLIST_
#define _LINKEDLIST_

#include "common.h"

int ll_delete_node(List *,Client *);
bool ll_append(List *,Client *);
void ll_free(List *);

// Queue (very scuffed) not used
/*
void ll_enqueue(Node **ll_head, Node **ll_tail, Node *client_node);
void * ll_dequeue(Node **llhead, Node **ll_tail);
*/

#endif
