#ifndef _LINKEDLIST_
#define _LINKEDLIST_

#include "common.h"

int ll_delete_node(struct list_header *list, Client *);
bool ll_append(struct list_header *, Client *);
void ll_free(struct list_header *);
bool ll_is_node_in_list(struct list_header *list, struct list_node *node);

// Queue (very scuffed) not used
/*
void ll_enqueue(Node **ll_head, Node **ll_tail, Node *client_node);
void * ll_dequeue(Node **llhead, Node **ll_tail);
*/

#endif
