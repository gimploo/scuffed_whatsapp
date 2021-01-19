#include "common.h"
#include "linkedlist.h"
#include "thread_pool.h"

List list = {
    .head = NULL,
    .count = 0,
    .lock = PTHREAD_RWLOCK_INITIALIZER
};

// Thread pool
pthread_t thread_pool[SERVER_BACKLOG];

// Server 
int         server_init(short port, int backlog);
int         server_sendline(Client *, char buffer[], int limit);
int         server_recvline(Client *, char buffer[], int limit);
Client *    server_accept_connection(int server_socket);
void        server_remove_client(Client *);
void        server_add_client(Client *);
void        server_send_message(Client *, MSG_TYPE request);
void *      server_recv(void *client);

// Client
Client *    client_create_node(int socket, char *addr);
void        client_get_info(Client *);
Client *    client_get_by_name_from_list(char *name);
Client *    client_set_partner(Client *client);

int         client_partner_chat_setup(Client *client);
int         client_partner_chat_thread_create(Client *client);
void *      client_partner_chat_thread_handler(void *pclient_pair);
    
// miscellaneous
void        print_active_users(List *);
void        send_active_users(Client *);
int         does_user_exist(char *);

int main(void)
{
    system("clear");
    printf("[LOG] Listening @ PORT %d ...\n", SERVER_PORT);

    // Initializing server
    int server_socket = server_init(SERVER_PORT, SERVER_BACKLOG);

    // TODO: Implement Thread_pool

    /*for (int i = 0;i < 2; i++)*/
    for(;;)
    {
        // prints all acitve users in a menu
        print_active_users(&list);

        // Accepts incomming connection
        Client *client = server_accept_connection(server_socket);

        // Creates a server recv thread connection
        pthread_t thread1;
        if (pthread_create(&thread1, NULL, server_recv, client) != 0)
        {
            fprintf(stderr, "[ERR] pthread_create: failed to create thread\n");
        }

    }
    // Freeing the client list
    ll_free(list.head);

}

void client_get_info(Client *client)
{
    char recvline[MAXWORD+1];

    // Gets user name from client
    server_recvline(client, recvline, MAXWORD);

    // Checks if the name already exist
    while (does_user_exist(recvline) == 0)
    {
        server_send_message(client, CLIENT_USERNAME_TAKEN);
        server_recvline(client, recvline, MAXWORD);
    }

    server_send_message(client, CLIENT_REGISTERED);

    // sets the name to the client struct
    strcpy(client->name, recvline);
}

int does_user_exist(char *name)
{
    pthread_rwlock_rdlock(&list.lock);
    Client *tmp = list.head;
    while (tmp)
    {
        if (strcmp(tmp->name, name) == 0)
        {
            pthread_rwlock_unlock(&list.lock);
            return 0;
        }
        tmp = tmp->next;
    }
    pthread_rwlock_unlock(&list.lock);
    return 1;
}


Client * client_create_node(int socket, char addr[])
{
    Client *client = malloc(sizeof(Client));
    client->socket = socket;
    client->partner = NULL;
    client->next = NULL;
    client->available = true;
    client->chat_active = false;

    strcpy(client->straddr , addr);
    memset(client->name, 0, MAXWORD);

    // Request for the client info (rn only username from the client)
    client_get_info(client);

    // Appends new client to the linked list DS 
    server_add_client(client);

    return client;
}


Client * server_accept_connection(int server_socket)
{
    int client_socket;
    struct sockaddr_in addr;
    socklen_t addr_len;
    char client_address[MAXLINE+1];

    if ((client_socket = 
                accept(server_socket,(struct sockaddr *)&addr, &addr_len)) < 0)
        ERROR_HANDLE();

    inet_ntop(AF_INET, &addr, client_address, MAXLINE);

    printf("[LOG] Client @ %s connecting ... \n", client_address);

    return client_create_node(client_socket, client_address);
}

int server_init(short port, int backlog)
{
    int server_socket;
    struct sockaddr_in servaddr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERROR_HANDLE();
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) 
            < 0)
        ERROR_HANDLE();

    if (listen(server_socket, backlog) < 0)
        ERROR_HANDLE();

    return server_socket;
}

void print_active_users(List *list)
{
    printf("[LOG] Active user list: ");
    Client *tmp = list->head;
    if (tmp == NULL)
        printf("none\n");
    else
    {
        pthread_rwlock_rdlock(&list->lock);
        while (tmp)
        {
            printf(" %s ->", tmp->name);
            tmp = tmp->next;
        }
        pthread_rwlock_unlock(&list->lock);
        printf(" NULL\n");
    }

}

void send_active_users(Client *client)
{
    char buffer[MAXLINE+1];
    Client *link1 = list.head;
    int j, k, i = 0;
    
    pthread_rwlock_rdlock(&list.lock);
        while (link1)
        {
            k = j = 0;
            while (link1->name[k] != '\0')
            {
                buffer[i++] = link1->name[k++];
            }
            buffer[i++] = '\n';
            link1 = link1->next;
        }
        buffer[i-1]= '\0';
    pthread_rwlock_unlock(&list.lock);

    server_sendline(client, buffer, MAXLINE);
}

void * server_recv(void *pclient)
{
    Client *client = (Client *)pclient;
    char recvline[MAXLINE+1];

    while (true)
    {
        // Recieves the request
        switch(server_recvline(client, recvline, MAXLINE)) 
        {
            // error 
            case -1: 
            // Client disconnected
            case -2: 
                return NULL;
                break;
        }

        MSG_TYPE request = cstr_to_msg(recvline);

        // Handles the request accordingly
        printf("[LOG] (%s) messaged server: %s\n", client->name, recvline);
        switch (request) 
        {
            case ACTIVE_USERS:
                server_send_message(client, ACTIVE_USERS);
                send_active_users(client);
                break;
            case CLIENT_USERNAME_TAKEN:
                server_send_message(client, CLIENT_USERNAME_TAKEN);
                server_recvline(client, recvline, MAXWORD);
                break;
            case CLIENT_REGISTERED:
                server_send_message(client, CLIENT_REGISTERED);
                break;
             case CLIENT_CHOOSE_PARTNER:
                server_send_message(client, CLIENT_CHOOSE_PARTNER);
                client_set_partner(client);
                break;
             case CLIENT_CHAT_SETUP:
                if (client_partner_chat_setup(client) != 0)
                    server_send_message(client, CLIENT_PARTNER_NOT_SET);
                break;
             case CLIENT_CHAT_START:
                client_partner_chat_thread_create(client);
                break;
             default:
                fprintf(stderr, "[ERR] server_request: %s\n", recvline);
                break;
        }
    }
    return NULL;
}

int server_sendline(Client *client, char buffer[], int limit)
{
    if (buffer[0] == '\0')
    {
        fprintf(stderr, "[ERR] server_sendline: buffer empty\n") ;
        
    }
    if (send(client->socket, buffer, limit, 0) < 0)
    {
        fprintf(stderr, "[ERR] server_sendline: error while sending\n");
        return -1;
    }
    
    /*printf("\n---------------------\n");*/
    /*printf("* send buffer: %s", buffer);*/
    /*printf("\n---------------------\n");*/
    
    return 0;
}

int server_recvline(Client *client, char buffer[], int limit)
{
    int n;
    memset(buffer, 0, limit);

    if ((n = recv(client->socket, buffer, limit, 0)) < 0)
    {
        PERROR();
        server_remove_client(client);
        return -1;
    }
    else if (n == 0)
    {
        server_remove_client(client);
        return -2;
    }
    
    /*printf("\n---------------------\n");*/
    /*printf("* recv buffer: %s", buffer);*/
    /*printf("\n---------------------\n");*/
    
    return 0;
}

void server_send_message(Client *client, MSG_TYPE request)
{
    char *str_msg = msg_to_cstr(request);
    server_sendline(client, str_msg, MAXMSG);
    printf("[LOG] server messaged %s : %s\n", client->name[0] == 0 ? "UNKOWN":client->name, str_msg);
}

Client * client_get_by_name_from_list(char *name)
{
    Client *node = list.head;
    pthread_rwlock_rdlock(&list.lock);
    while (node)
    {
        if (strcmp(node->name, name) == 0)
        {
            pthread_rwlock_unlock(&list.lock);
            return node;
        }
        node = node->next;
    }
    pthread_rwlock_unlock(&list.lock);
    return NULL;
}

Client * client_set_partner(Client *client)
{
    char recvline[MAXLINE+1];

    server_recvline(client, recvline, MAXWORD);
    Client *other_client = client_get_by_name_from_list(recvline);
    if (other_client == NULL)
    {
       fprintf(stderr, "[ERR] client_set_partner: client choose invalid client\n");
       server_send_message(client, CLIENT_NOT_FOUND);
       return NULL;
    } 
    else if (client == other_client)
    {
       fprintf(stderr, "[ERR] client_set_partner: client choose him/her self\n");
       server_send_message(client, CLIENT_SAME_USER);
       return NULL;
    } 

    pthread_rwlock_wrlock(&list.lock);
        client->partner = other_client;
    pthread_rwlock_unlock(&list.lock);

    server_send_message(client, CLIENT_PARTNER_SELECTED);

    return 0;
}

int client_partner_chat_setup(Client *client)
{
    // YOU havent choose a partner 
    if (client->partner == NULL)
    {
        fprintf(stderr, "[ERR] client_partner_chat_setup: client partner is null\n");
        return -1;
    }
    Client *other_client = client->partner;

    pthread_rwlock_wrlock(&list.lock);
    {
        // he/she is available and wants to talk to you
        if (other_client->partner == client && other_client->available == true)
        {
            client->available = other_client->available = false;
            client->chat_active = other_client->chat_active = true;
        }

        // he/she hasnt made anyone his/her friend 
        else if (other_client->partner == NULL)
        {
            fprintf(stderr, "[LOG] client_partner_chat_setup: other clients partner is null\n");
            pthread_rwlock_unlock(&list.lock);
            return -3;
        }

        // he/she hasnt made you his/her friend
        else if (other_client->partner != client)
        {
            fprintf(stderr, "[LOG] client_partner_chat_setup: other clients partner is not client\n");
            pthread_rwlock_unlock(&list.lock);
            return -4;
        }
        else 
        {
            fprintf(stderr, "[LOG] client_partner_chat_setup: unkown error\n");
            return -5;
        }
    }
    pthread_rwlock_unlock(&list.lock);

    server_send_message(client, CLIENT_CHAT_START);
    server_send_message(other_client, CLIENT_CHAT_START);
    return 0;
}

int client_partner_chat_thread_create(Client *client)
{
    pthread_t tid1;

    if (pthread_create(&tid1, NULL, client_partner_chat_thread_handler, client) != 0)
    {
        fprintf(stderr, "[ERR] client_partner_chat_thread_handler: unable to create thread\n");
       return errno;
    }

    if (pthread_join(tid1, NULL) != 0)
    {
        fprintf(stderr, "[ERR] client_partner_chat_thread_handler: unable to join client thread\n");
       return errno;
    }

    return 0;
}

void *client_partner_chat_thread_handler(void *pclient)
{
    char recvline[MAXLINE+1];
    char sendline[MAXSND+1];
    char mssg[MAXLINE+1];

    Client *client = (Client *)pclient;
    snprintf(mssg, MAXLINE, "%s left\n", client->name);

    printf("[LOG] %s <-> %s chat initiated\n", client->name, client->partner->name);
    while (client->chat_active)
    {
        switch(recv(client->socket, recvline, MAXLINE, 0 ))
        {
            case -1:
                return NULL;
            case 0:
                pthread_rwlock_wrlock(&client->lock);
                {
                    client->chat_active = false;
                }
                pthread_rwlock_unlock(&client->lock);

                server_sendline(client->partner, mssg, MAXLINE);
                return NULL;
        }

        snprintf(sendline, MAXSND, "%s: %s", client->name, recvline);
        if (client->partner == NULL)
            break;
        else if (send(client->partner->socket, sendline, MAXLINE, 0) < 0)
        {
            pthread_rwlock_wrlock(&list.lock);
            {
                client->chat_active = false;
            }
            pthread_rwlock_unlock(&list.lock);
            fprintf(stderr, "[ERR] client2 send error\n");
            return NULL;
        }
    }
    return NULL;
}

void server_add_client(Client *client)
{
    pthread_rwlock_wrlock(&list.lock);
    {
        ll_append(&list.head, client);
        list.count++;
    }
    pthread_rwlock_unlock(&list.lock);
}

void server_remove_client(Client *client)
{
    fprintf(stderr, "[LOG] Client (%s) disconnected\n", client->name);
    close(client->socket);

    pthread_rwlock_wrlock(&list.lock);
    {
        list.count--;
        if (client->partner != NULL) 
        {
            client->partner->available = true;
            if (client->partner->partner == client)
            {
                client->partner->chat_active = false;
                client->partner->partner = NULL;
            }
        }
        switch (ll_deletion(&list.head, client))
        {
            case 0:
                fprintf(stderr, "[LOG] ll_deletion: element deleted\n");
                break;
            case 1:
                fprintf(stderr, "[LOG] ll_deletion: list empty\n");
                break;
            case 2:
                fprintf(stderr, "[LOG] ll_deletion: element not found\n");
                break;
        }
    }
    pthread_rwlock_unlock(&list.lock);

}
