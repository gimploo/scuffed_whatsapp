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
#include <assert.h>

#define SERVER_PORT 18000 
#define MAXMSG 25
#define MAXSND 6144
#define MAXLINE 4096
#define MAXWORD 1024
#define LOCALHOST "127.0.0.1"
#define IPV4_STRLEN 16
#define SERVER_BACKLOG 10

#define ERROR_HANDLE() {\
    fprintf(stderr, "[!] ERROR ("__FILE__" :%d): %s\n", __LINE__, strerror(errno));\
    exit(1);\
}

#define PERROR() (fprintf(stderr, "[!] ERROR ("__FILE__" :%d): %s\n", __LINE__, strerror(errno)))

// fflush doesnt work on wsl2 D: 
#define CLEAR_STDIN() {\
    while (getchar() != '\n');\
}

typedef enum {
    SUCCESS,
    FAILED,
    ASK,
    INVALID,
    ACTIVE_USERS,

    CLIENT_UNAVAILABLE,
    CLIENT_USERNAME_TAKEN,
    CLIENT_REGISTERED,

    CLIENT_CHAT_SETUP,
    CLIENT_CHAT_START,

    CLIENT_SAME_USER,
    CLIENT_NOT_FOUND,
    CLIENT_CHOOSE_PARTNER,
    CLIENT_PARTNER_SELECTED
} MSG_TYPE;

typedef struct client {

    // info
    char name[MAXWORD+1];
    int socket;
    char straddr[IPV4_STRLEN+1];
    bool available;

    // chat 
    struct client *partner;
    volatile bool chat_active;
    pthread_rwlock_t lock;

    // list
    struct client *next;

} Client;

// Takes input from the user and stores in buffer[]
void cstring_input(char *message, char buffer[]);

char * msg_to_cstr(MSG_TYPE msg);

MSG_TYPE cstr_to_msg(char *cstring);

#endif
