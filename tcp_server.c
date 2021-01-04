#include "common.h"
#include "linkedlist.h"

pthread_mutex_t lock_ll = PTHREAD_MUTEX_INITIALIZER;
volatile uint32_t total_clients = 0;
Client *ll_head = NULL;

// Server 
int server_init(short port, int backlog);
int server_sendline(Client *client, char buffer[], size_t limit);
int server_recvline(Client *client, char buffer[], size_t limit);
Client * accept_connection(int server_socket);
void server_remove_client(Client *client);
void server_add_client(Client *client);
void server_message_handler(Client *client, char recvline[], MSG_TYPE msg);

// Thread specific functions
void * server_recv(void *client);

// Client
Client * client_create_node(int socket, char *addr);
void get_client_info(Client *this_client);
void client_get_partner( Client *client );
    
// miscellaneous
void print_active_users(Client *head);
void send_active_users(Client *client);
int does_user_exist(char *name);
Client * get_client_from_list( char *name );

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

        // Request for the client info (rn only username from the client)
        get_client_info(client);

        // TODO: need to find a better way to get over this shitty bug
        // NOTE: the bug is the client recieves Username registered message
        //       along with active user buffer at the same time.
        sleep(1);
        send_active_users(client);

        // Creates a server recv thread connection
        pthread_t thread1;
        pthread_create(&thread1, NULL, server_recv, client);

    }

    // Freeing the client list
    ll_free(ll_head);

}

void get_client_info(Client *client)
{
    char recvline[MAXWORD];

    // Gets user name from client
    server_recvline(client, recvline, MAXWORD);

    // Checks if the name already exist
    while (does_user_exist(recvline) == 0)
    {
        server_message_handler(client, recvline, CLIENT_USERNAME_TAKEN);
    }

    server_message_handler(client, recvline, CLIENT_REGISTERED);

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
    strcpy(client->straddr , addr);
    client->partner = NULL;
    client->next = NULL;
    client->available = true;
    memset(client->name, 0, MAXWORD);

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
    if (ll_head == NULL && total_clients == 1)
    {
        strcpy(buffer,"(~ o ~) . z Z)");
        server_sendline(client, buffer, MAXLINE);
        return ;
    }

    int j,k;
    int i = 0;
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

    server_sendline(client, buffer, MAXLINE);
}

void * server_recv(void *pclient)
{
    Client *client = (Client *)pclient;
    char buffer[MAXLINE];
    char left_client[MAXWORD];

    strcpy(left_client, client->name);

    while (true)
    {
        // Error handling when recieving from a client
        switch(server_recvline(client, buffer, MAXLINE)) {
            // error of some kind dk
            case -1: 
            {
                return NULL;
            } break;

            // if client disconnects 
            case -2: 
            {
                Client *link2 = ll_head;
                char message[MAXLINE];
                snprintf(message, MAXLINE, "[!] %s left server\n", left_client);
                while (link2) {
                    server_sendline(link2, message, strlen(message));
                    link2 = link2->next;
                }
                return NULL;
            } break;
        }

        server_message_handler(client, buffer, cstr_to_msg(buffer));
    }
    return NULL;
}

int server_sendline(Client *client, char buffer[], size_t limit)
{
    if (buffer[0] == '\0')
    {
        fprintf(stderr, "[!] server_sendline: buffer empty\n") ;
        exit(1);
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

void server_message_handler(Client *client, char buffer[], MSG_TYPE msg)
{
    switch (msg) {
        case CLIENT_ACTIVE_USERS:
            send_active_users(client);
            break;
        case CLIENT_USERNAME_TAKEN:
            server_sendline(client, msg_to_cstr(msg), MAXLINE);
            server_recvline(client, buffer, MAXWORD);
            break;
        case CLIENT_REGISTERED:
            server_sendline(client, msg_to_cstr(msg), MAXLINE);
            break;
         case CLIENT_SET_PARTNER:
            client_get_partner(client);
            break;
         default:
            fprintf(stderr, "server_request: %s\n", buffer);
            break;
    }

}
Client * get_client_from_list( char *name )
{
    Client *node = ll_head;
    while (node)
    {
        if (strcmp(node->name, name) == 0)
            return node;
        node = node->next;
    }
    return NULL;
}

void client_get_partner( Client *client )
{
    char recvline[MAXWORD];
    Client *partner = NULL;
    send_active_users( client );
    server_recvline( client, recvline, MAXWORD );

    if ( does_user_exist( recvline ) == 0 &&  
            (partner = get_client_from_list( recvline )) != NULL)
    {
        if ( partner->available == true )
        {
            if ( partner->partner == client)
            {
                client->partner = partner;
                server_sendline( client, msg_to_cstr( SUCCESS ), MAXWORD );
            }
            else 
            {
                // TODO: maybe have the client wait untill partner is available
                fprintf(stderr, "client_select_partner: partner == false\n");
                server_sendline( client, msg_to_cstr( FAILED ), MAXWORD );
            }
        }
        else 
        {
            fprintf(stderr, "client_select_partner: partner unavailable\n");
            server_sendline(client, msg_to_cstr(CLIENT_UNAVAILABLE), MAXWORD);
        }
    }
    else 
    {
        fprintf(stderr, "client_select_partner: partner not found\n");
        server_sendline( client, msg_to_cstr( CLIENT_NOT_FOUND ), MAXWORD );
    }

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
            printf("[!] ll_deletion: element deleted\n");
            break;
        case 1:
            printf("[!] ll_deletion: list empty\n");
            break;
        case 2:
            printf("[!] ll_deletion: element not found\n");
            break;
    }
    pthread_mutex_unlock(&lock_ll);
}
