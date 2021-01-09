#include "common.h"
#include "linkedlist.h"

typedef struct {
    struct client *client1;
    struct client *client2;
    volatile bool connected;
    pthread_mutex_t lock;
} Client_Pair;
pthread_mutex_t client2client_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t client2client_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock_ll = PTHREAD_MUTEX_INITIALIZER;
uint32_t total_clients = 0;
Client *ll_head = NULL;

// Server 
int server_init(short port, int backlog);
int server_sendline(Client *client, char buffer[], size_t limit);
int server_recvline(Client *client, char buffer[], size_t limit);
Client * accept_connection(int server_socket);
void server_remove_client(Client *client);
void server_add_client(Client *client);
void server_service_handler(Client *client, char recvline[], MSG_TYPE msg);
void server_send_message(Client *client, MSG_TYPE request);

// Thread specific functions
void * server_recv(void *client);
void *client_pair_client1_handler(void *pclient_pair);
void *client_pair_client2_handler(void *pclient_pair);

// Client
Client * client_create_node(int socket, char *addr);
void client_to_client_connection(Client *client1);
void client_get_info(Client *this_client);
    
// miscellaneous
void print_active_users(Client *head);
void send_active_users(Client *client);
int does_user_exist(char *name);
Client * client_get_by_name_from_list(char *name);

int main(void)
{
    // Initializing server
    int server_socket = server_init(SERVER_PORT, SERVER_BACKLOG);

    /*for (int i = 0;i < 2; i++)*/
    for(;;)
    {
        printf("\n[*] Listening @ %d ...\n", SERVER_PORT);

        // prints all acitve users in a menu
        print_active_users(ll_head);

        // Accepts incomming connection
        Client *client = accept_connection(server_socket);

        // Creates a server recv thread connection
        pthread_t thread1;
        pthread_create(&thread1, NULL, server_recv, client);

    }

    // Freeing the client list
    ll_free(ll_head);

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
    Client *tmp = ll_head;
    while (tmp)
    {
        if (strcmp(tmp->name, name) == 0)
            return 0;
        tmp = tmp->next;
    }
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

void print_active_users(Client *head)
{
    printf("\n-----------------\n");
    printf("  Active Users\n");
    printf("-----------------\n");
    if (head == NULL)
        printf("(~ o ~) . z Z)\n");
    else
    {
        int si = 1;
        while (head)
        {
            printf("%i. %s\n", si++, head->name);
            head = head->next;
        }
    }
    printf("-----------------\n\n");

}

void send_active_users(Client *client)
{
    char buffer[MAXLINE];
    Client *link1 = ll_head;
    int j, k, i = 0;
    
    if (ll_head == NULL && total_clients == 1)
    {
        strcpy(buffer,"(~ o ~) . z Z)");
        server_sendline(client, buffer, MAXLINE);
        return ;
    }
    

    pthread_rwlock_rdlock(&client->lock);
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
    pthread_rwlock_unlock(&client->lock);

    /*printf("send_active_users: sending active users to %s\n", client->name);*/
    printf("----\nOUTPUT:\n%s\n---\n", buffer);
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
             case CLIENT_SET_PARTNER:
                pthread_mutex_lock(&client2client_lock);
                client_to_client_connection(client);
                while (!client->available)
                    pthread_cond_wait(&client2client_cond, &client2client_lock);
                pthread_mutex_unlock(&client2client_lock);
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
        return -1;
    }
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
    return 0;
}

void server_send_message(Client *client, MSG_TYPE request)
{
    server_sendline(client, msg_to_cstr(request), MAXMSG);
}
Client * client_get_by_name_from_list(char *name)
{
    Client *node = ll_head;
    while (node)
    {
        if (strcmp(node->name, name) == 0)
        {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void client_to_client_connection(Client *client1)
{
   char recvline[MAXWORD+1];
   server_recvline(client1, recvline, MAXWORD);
   Client *client2 = client_get_by_name_from_list(recvline);
   if (client2 == NULL)
   {
       fprintf(stderr, 
               "client_to_client_connection: client choose invalid client\n");
       server_send_message(client1, CLIENT_NOT_FOUND);
       return ;
   } 
   else if (client1 == client2)
   {
       fprintf(stderr, 
               "client_to_client_connection: client choose him/her self\n");
       server_send_message(client1, DUMB_ASS);
       return ;
   }
   server_send_message(client1, SUCCESS);
   server_send_message(client2, SUCCESS);
   client1->available = client2->available = false;

   Client_Pair clients = {client1, client2, true, PTHREAD_MUTEX_INITIALIZER};

   pthread_t tid1;
   pthread_t tid2;

   pthread_create(&tid1, NULL, client_pair_client1_handler, &clients);
   pthread_create(&tid2, NULL, client_pair_client2_handler, &clients);

   pthread_join(tid1, NULL);
   pthread_cond_signal(&client2client_cond);

}

void *client_pair_client2_handler(void *pclient_pair)
{
    char recvline[MAXLINE+1];
    Client_Pair *clients = (Client_Pair *)pclient_pair;
    while (clients->connected)
    {
        switch(recv(clients->client2->socket, recvline, MAXLINE, 0 ))
        {
            case -1:
                fprintf(stderr, "client2 recv error\n");
                continue;
            case 0:
                fprintf(stderr, "client2 disconnected\n");
                pthread_mutex_lock(&clients->lock);
                clients->connected = false;
                pthread_mutex_unlock(&clients->lock);
                return NULL;
        }
        if (send(clients->client1->socket, recvline, MAXLINE, 0) < 0)
            fprintf(stderr, "client1 send error\n");
    }
    return NULL;
}
void *client_pair_client1_handler(void *pclient_pair)
{
    char recvline[MAXLINE+1];
    Client_Pair *clients = (Client_Pair *)pclient_pair;
    while(clients->connected) 
    {
        switch(recv(clients->client1->socket, recvline, MAXLINE, 0 ))
        {
            case -1:
                fprintf(stderr, "client1 recv error\n");
                continue;
            case 0:
                fprintf(stderr, "client1 disconnected\n");
                pthread_mutex_lock(&clients->lock);
                clients->connected = false;
                pthread_mutex_unlock(&clients->lock);
                return NULL;
        }
        if (send(clients->client2->socket, recvline, MAXLINE, 0) < 0)
            fprintf(stderr, "client2 send error\n");
    }
    return NULL;
}


void server_add_client(Client *client)
{
    pthread_mutex_lock(&lock_ll);
    ll_append(&ll_head,client);
    total_clients++;
    pthread_mutex_unlock(&lock_ll);
}
void server_remove_client(Client *client)
{
    printf("[!] Client (%s) disconnected\n", client->name);
    close(client->socket);

    pthread_mutex_lock(&lock_ll);
    total_clients--;
    switch (ll_deletion(&ll_head, client))
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
    pthread_mutex_unlock(&lock_ll);
}
