/*
   Includes common headerfiles 
*/

#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>

#define SERVER_PORT 18000 
#define MAXMSG 35
#define MAXSND 6144
#define MAXLINE 4096
#define MAXWORD 1024
#define MAX_GROUPS 3
#define LOCALHOST "127.0.0.1"
#define IPV4_STRLEN 16
#define SERVER_BACKLOG 10

#define ERROR_HANDLE() {\
    fprintf(stderr, "[!] ERROR ("__FILE__" :%d): %s\n", __LINE__, strerror(errno));\
    exit(1);\
}

#define PERROR() (fprintf(stderr, "[!] ERROR ("__FILE__" :%d): %s\n", __LINE__, strerror(errno)))

typedef enum {
    SUCCESS,
    FAILED,
    ASK,
    INVALID,
    ACTIVE_USERS,

    CLIENT_UNAVAILABLE,
    CLIENT_USERNAME_TAKEN,
    CLIENT_NOT_FOUND,
    CLIENT_REGISTERED,

    CLIENT_CHAT_SETUP,
    CLIENT_CHAT_START,
    CLIENT_CHAT_QUIT,
    CLIENT_CHAT_CLOSED,

    CLIENT_ADD_FRIEND,
    CLIENT_REMOVE_FRIEND,
    CLIENT_CHOOSE_FRIEND,
    CLIENT_CHOOSE_ITSELF,
    CLIENT_PARTNER_NULL,
    CLIENT_PARTNERS_PARTNER_NULL,
    CLIENT_PARTNER_SELECTED,

    CLIENT_GROUP_EMPTY,
    CLIENT_GROUP_OVERFLOW,
    CLIENT_GROUP_ADD_MEMBER,
    CLIENT_GROUP_MEMBER_ADDED,
    CLIENT_GROUP_MEMBER_EXIST,
    CLIENT_GROUP_CHATROOM_SETUP,
    CLIENT_GROUP_CHATROOM_START,
    CLIENT_GROUP_CHATROOM_CLOSE,
} Msg_Type;

struct list_node {
    struct client_node *client;
    struct list_node *next;
};

struct list_header {
    struct list_node *head;
    struct list_node *tail;
    char name[25];
    unsigned count;
    pthread_rwlock_t lock;
};


struct client_node {
    int socket;
    char name[MAXWORD+1];
    char straddr[IPV4_STRLEN+1];
    bool available;

    // Client lists
    struct list_header friends_list;
    struct list_header my_group;

    // Group
    struct list_header groups_iam_in; //holds user names
    
    // Current partner
    struct client_node *_friend;
};


typedef struct client_node Client;
typedef struct list_header List;
typedef struct list_node Node;


// Takes input from the user and stores in buffer[]
void cstring_input(char *message, char buffer[], int limit);

char * msg_as_cstr(Msg_Type msg);

Msg_Type cstr_as_msg(char *cstring);

#endif
