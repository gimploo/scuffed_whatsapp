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
    WAIT,
    ASK,
    ACTIVE_USERS,
    CLIENT_UNAVAILABLE,
    CLIENT_NOT_FOUND,
    CLIENT_SET_PARTNER,
    CLIENT_USERNAME_TAKEN,
    CLIENT_REGISTERED,
    INVALID,
    DUMB_ASS
} MSG_TYPE;

typedef struct client {
    char name[MAXWORD+1];
    int socket;
    char straddr[IPV4_STRLEN+1];
    bool available;

    struct client *partner;
    struct client *next;

} Client;

// Takes input from the user and stores in buffer[]
void cstring_input(char *message, char buffer[]);

char * msg_to_cstr(MSG_TYPE msg);

MSG_TYPE cstr_to_msg(char *cstring);

#endif
