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
#define MAX_MEMBERS 3
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

    CLIENT_CHOOSE_PARTNER,
    CLIENT_CHOOSE_ITSELF,
    CLIENT_PARTNER_NULL,
    CLIENT_PARTNERS_PARTNER_NULL,
    CLIENT_PARTNER_SELECTED,

    CLIENT_GROUP_EMPTY,
    CLIENT_GROUP_OVERFLOW,
    CLIENT_GROUP_ADD_MEMBER,
    CLIENT_GROUP_BROADCAST_SETUP,
    CLIENT_GROUP_BROADCAST_START,
    CLIENT_GROUP_BROADCAST_CLOSE,
    CLIENT_GROUP_MEMBER_ADDED
} Msg_Type;


typedef struct client {

    // info
    char name[MAXWORD+1];
    int socket;
    char straddr[IPV4_STRLEN+1];
    bool available;
    pthread_rwlock_t lock;

    // Private chat 
    struct client *friend;
    volatile bool private_chat_active;

    // Group chat
    struct client *group[MAX_MEMBERS];
    int top;
    volatile bool group_chat_active;

    // list
    struct client *next_client;

} Client;

// List keeps track of linkedlist 
typedef struct {
    Client *head;
    Client *tail;
    int count;
    pthread_rwlock_t lock;
} List;


// Takes input from the user and stores in buffer[]
void cstring_input(char *message, char buffer[], int limit);

char * msg_to_cstr(Msg_Type msg);

Msg_Type cstr_to_msg(char *cstring);

#endif
