#include "common.h"
#include "linkedlist.h"

typedef struct {
    struct client *client1;
    struct client *client2;
    volatile bool active;
    pthread_rwlock_t lock;
} Client_Pair;

List list = {
    .head = NULL,
    .count = 0,
    .lock = PTHREAD_RWLOCK_INITIALIZER
};

// Thread pool
pthread_t thread_pool[SERVER_BACKLOG];

// Server 
int         server_init(short port, int backlog);
int         server_sendline(Client *, char buffer[], size_t limit);
int         server_recvline(Client *, char buffer[], size_t limit);
Client *    accept_connection(int server_socket);
void        server_remove_client(Client *);
void        server_add_client(Client *);
void        server_send_message(Client *, MSG_TYPE request);

// Thread specific functions
void *      server_recv(void *client);
void *      client_pair_chat_handler(void *pclient_pair);

// Client
Client *    client_create_node(int socket, char *addr);
void        client_to_client_connection(Client *);
void        client_get_info(Client *);
    
// miscellaneous
void        print_active_users(List *list);
void        send_active_users(Client *);
int         does_user_exist(char *name);
Client *    client_get_by_name_from_list(char *name);

int main(void)
{
    // Initializing server
    int server_socket = server_init(SERVER_PORT, SERVER_BACKLOG);

    /*for (int i = 0;i < 2; i++)*/
    for(;;)
    {
        printf("\n[*] Listening @ %d ...\n", SERVER_PORT);

        // prints all acitve users in a menu
        print_active_users(&list);

        // Accepts incomming connection
        Client *client = accept_connection(server_socket);

        // Creates a server recv thread connection
        pthread_t thread1;
        if (pthread_create(&thread1, NULL, server_recv, client) != 0)
        {
            fprintf(stderr, "pthread_create: failed to create thread\n");
        }

    }

    // Freeing the client list
    ll_free(list.head);

}

void client_get_info(Client *client)
{
    char recvline[MAXWORD];

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

    strcpy(client->straddr , addr);
    memset(client->name, 0, MAXWORD);

    // Request for the client info (rn only username from the client)
    client_get_info(client);

    // Appends new client to the linked list DS 
    server_add_client(client);

    return client;
}


Client * accept_connection(int server_socket)
{
    int client_socket;
    struct sockaddr_in addr;
    socklen_t addr_len;
    char client_address[MAXLINE];

    if ((client_socket = 
                accept(server_socket,(struct sockaddr *)&addr, &addr_len)) < 0)
        ERROR_HANDLE();

    inet_ntop(AF_INET, &addr, client_address, MAXLINE);

    printf("[*] Client @ %s connecting ... \n", client_address);
    perror("[!] STATUS ");

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
    Client *tmp = list->head;
    printf("\n-----------------\n");
    printf("  Active Users\n");
    printf("-----------------\n");
    if (tmp == NULL)
        printf("(~ o ~) . z Z)\n");
    else
    {
        int si = 1;
        pthread_rwlock_rdlock(&list->lock);
        while (tmp)
        {
            printf("%i. %s\n", si++, tmp->name);
            tmp = tmp->next;
        }
        pthread_rwlock_unlock(&list->lock);
    }
    printf("-----------------\n\n");

}

void send_active_users(Client *client)
{
    char buffer[MAXLINE];
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
    char recvline[MAXLINE];

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
                client_to_client_connection(client);
                break;
             default:
                fprintf(stderr, "server_request: %s\n", recvline);
                break;
        }
    }
    return NULL;
}

int server_sendline(Client *client, char buffer[], size_t limit)
{
    if (buffer[0] == '\0')
    {
        fprintf(stderr, "[!] server_sendline: buffer empty\n") ;
        
    }
    if (send(client->socket, buffer, limit, 0) < 0)
    {
        fprintf(stderr, "server_sendline: error while sending\n");
        return -1;
    }
    
    printf("\n---------------------\n");
    printf("* send buffer: %s", buffer);
    printf("\n---------------------\n");
    
    return 0;
}

int server_recvline(Client *client, char buffer[], size_t limit)
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
    
    printf("\n---------------------\n");
    printf("* recv buffer: %s", buffer);
    printf("\n---------------------\n");
    
    return 0;
}

void server_send_message(Client *client, MSG_TYPE request)
{
    server_sendline(client, msg_to_cstr(request), MAXMSG);
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

void client_to_client_connection(Client *client)
{
   char recvline[MAXWORD+1];
   char sendline[MAXLINE+1];

   server_recvline(client, recvline, MAXWORD);
   Client *other_client = client_get_by_name_from_list(recvline);

   // Checking if the client choose is legit
   if (other_client == NULL)
   {
       fprintf(stderr, 
               "client_to_client_connection: client choose invalid client\n");
       server_send_message(client, CLIENT_NOT_FOUND);
       return ;
   } 
   else if (client == other_client)
   {
       fprintf(stderr, 
               "client_to_client_connection: client choose him/her self\n");
       server_send_message(client, DUMB_ASS);
       return ;
   }

   // Checking whether the other client wanted to talk with you
   if (other_client->partner != client)
   {
       snprintf(sendline, MAXLINE, "%s wants to talk to you",client->name);

       // other client
       server_sendline(other_client, sendline, MAXLINE);
       server_send_message(other_client, PAUSE_THREAD);
       server_send_message(other_client, ASK);
       server_recvline(other_client, recvline, 5);


       if (strcmp(recvline, "yes") == 0)
       {
           pthread_rwlock_wrlock(&list.lock);
           client->partner = other_client;
           other_client->partner = client;
           pthread_rwlock_unlock(&list.lock);
           server_send_message(other_client, CLIENT_CHOOSE_PARTNER);
           server_send_message(other_client, UNPAUSE_THREAD);
           // TODO: what if the otherclient choose someone else
           // this doesnt stop the first client from having a chat thread
       }
       else 
       {
           snprintf(sendline, MAXLINE, "%s declined", other_client->name);
           server_send_message(other_client, UNPAUSE_THREAD);
           server_sendline(client, sendline, MAXLINE);
           return ;
       }
   }
   else if (other_client->partner == client)
   {
       snprintf(sendline, MAXLINE, "(%s is talking... )", other_client->name);
       server_sendline(client, sendline, MAXLINE);
       server_send_message(other_client, UNPAUSE_THREAD);
       snprintf(sendline, MAXLINE, "(%s is talking... )", client->name);
       server_sendline(other_client, sendline, MAXLINE);
   }

   //TODO: have the thread creation happen after two clients decides to talk.
   printf("%s initiated chat mode\n", client->name);

   pthread_rwlock_wrlock(&list.lock);
   client->available = false;
   pthread_rwlock_unlock(&list.lock);

   Client_Pair client_pair_info = {
       .client1 = client, 
       .client2 = other_client, 
       .active = true, 
       .lock = PTHREAD_RWLOCK_INITIALIZER
   };

   pthread_t tid1;
   pthread_create(&tid1, NULL, client_pair_chat_handler, &client_pair_info);
   pthread_join(tid1, NULL);

}

void *client_pair_chat_handler(void *pclient_pair)
{
    char recvline[MAXLINE+1];
    char sendline[MAXSND+1];
    Client_Pair *clients = (Client_Pair *)pclient_pair;
    while (clients->active)
    {
        switch(recv(clients->client1->socket, recvline, MAXLINE, 0 ))
        {
            case -1:
                continue;
            case 0:
                pthread_rwlock_wrlock(&clients->lock);
                clients->active = false;
                pthread_rwlock_unlock(&clients->lock);
                return NULL;
        }
        snprintf(sendline, MAXSND, "%s: %s", clients->client1->name, recvline);
        if (send(clients->client2->socket, sendline, MAXLINE, 0) < 0)
        {
            fprintf(stderr, "client2 send error\n");
            break;
        }
            
    }
    return NULL;
}

void server_add_client(Client *client)
{
    pthread_rwlock_wrlock(&list.lock);
    ll_append(&list.head, client);
    list.count++;
    pthread_rwlock_unlock(&list.lock);
}

void server_remove_client(Client *client)
{
    printf("[!] Client (%s) disconnected\n", client->name);
    close(client->socket);

    pthread_rwlock_wrlock(&list.lock);
    list.count--;
    if (client->partner != NULL) 
    {
        client->partner->available = true;
        if (client->partner->partner == client)
            client->partner->partner = NULL;
    }
    switch (ll_deletion(&list.head, client))
    {
        case 0:
            fprintf(stderr, "[!] ll_deletion: element deleted\n");
            break;
        case 1:
            fprintf(stderr, "[!] ll_deletion: list empty\n");
            break;
        case 2:
            fprintf(stderr, "[!] ll_deletion: element not found\n");
            break;
    }
    pthread_rwlock_unlock(&list.lock);

}
