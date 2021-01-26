#include "common.h"
#include "linkedlist.h"
#include "thread_pool.h"

List db = {
    .head = NULL,
    .tail = NULL,
    .count = 0,
    .lock = PTHREAD_RWLOCK_INITIALIZER
};

// Thread pool
pthread_t thread_pool[SERVER_BACKLOG];

// Server 
int         server_init(short port, int backlog);
int         server_sendline(Client *, char buffer[], int limit);
int         server_recvline(Client *client, char buffer[], int limit);
Client *    server_accept_connection(int server_socket);
void        server_send_response(Client *, Msg_Type request);
void *      server_recv(void *client);
void        server_add_client(Client *client);
void        server_remove_client(Client *client);

// Client
Client *    client_create_node(int socket, char *addr);
void        client_get_info(Client *);
Client *    client_add_friend(Client *client);

// Private chat
int         client_friend_chat_setup(Client *client);
int         client_friend_chat_thread_create(Client *client);
void *      client_friend_chat_thread_handler(void *pclient_pair);
    
// Group chat
bool        client_group_add_member(Client *client, Client *member);
void *      client_group_broadcast_thread_handler(void *pclient);
int         client_group_broadcast_thread_create(Client *client);
int         client_group_setup(Client *client);
Client *    client_group_get_member(Client *client);
void        client_group_print_members(Client *client);

// miscellaneous
void        list_print(List *);
void        list_send_client_names(Client *);
bool        list_does_client_name_exist(char *name, List *list);
Client *    list_get_client_by_name(char *name, List *list);
bool        list_is_client_in_list(Client *client, List *list);


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
        list_print(&db);

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
    ll_free(&db);

}

void client_get_info(Client *client)
{
    char recvline[MAXWORD+1];

    // Gets user name from client
    server_recvline(client, recvline, MAXWORD);

    // Checks if the name already exist
    while (list_does_client_name_exist(recvline, &db))
    {
        server_send_response(client, CLIENT_USERNAME_TAKEN);
        server_recvline(client, recvline, MAXWORD);
    }

    server_send_response(client, CLIENT_REGISTERED);

    // sets the name to the client struct
    strcpy(client->name, recvline);
}

bool list_does_client_name_exist(char *name, struct list_header *list)
{
    pthread_rwlock_rdlock(&list->lock);
    {
        List_Node *tmp = list->head;
        while (tmp)
        {
            if (strcmp(tmp->client->name, name) == 0)
            {
                pthread_rwlock_unlock(&list->lock);
                return true;
            }
            tmp = tmp->next;
        }
    }
    pthread_rwlock_unlock(&list->lock);
    return false;
}


Client * client_create_node(int socket, char addr[])
{
    Client *client = malloc(sizeof(Client));
    client->available = true;
    client->_friend = NULL;
    client->friends_list = (List) {
        .head = NULL,
        .tail = NULL,
        .count = 0,
        .lock = PTHREAD_RWLOCK_INITIALIZER
    };
    client->group_members = (List){
        .head = NULL,
        .tail = NULL,
        .count = 0,
        .lock = PTHREAD_RWLOCK_INITIALIZER
    };

    // info
    client->socket = socket;
    snprintf(client->name, MAXWORD, "User%0i", db.count);
    strcpy(client->straddr , addr);

    // Appends new client to the linked list DS 
    server_add_client(client);

    // Request for the client info (rn only username from the client)
    client_get_info(client);

    return client;
}

/*
bool client_group_add_member(Client *client, Client *member)
{
    pthread_rwlock_wrlock(&db.lock);
    {
        if (client->top == MAX_MEMBERS-1)
        {
            fprintf(stderr, "[ERR] client_group_add_member: OVERFLOW\n");
            pthread_rwlock_unlock(&list.lock);
            return false;
        }
        printf("[LOG] %s added %s to the group\n", client->name, member->name);
        client->group[++client->top] = member;
        server_send_response(client, CLIENT_GROUP_MEMBER_ADDED);
    }
    pthread_rwlock_unlock(&db.lock);
    client_group_print_members(client);
    return true;
}

void client_group_print_members(Client *client)
{
    List_Node *node = NULL;

    printf("---- GROUP MEMBERS ----\n");
    pthread_rwlock_rdlock(&db.lock);
    {
        node = client->group_members.head;
    }
    pthread_rwlock_unlock(&db.lock);

    if (node == NULL)
    {
        fprintf(stderr, "client_group_print_members: node is null\n");
        return ;
    }
    while (node)
    {
        printf("| %s |", node->client->name);
        node = node->next;
    }
    printf("\n");
    printf("-----------------------\n");
}

int client_group_setup(Client *client)
{
    pthread_rwlock_rdlock(&db.lock);
    {
        if (client->group_members.head == NULL) 
        {
            server_send_response(client, CLIENT_GROUP_EMPTY);
            pthread_rwlock_unlock(&db.lock);
            return -1;
        }
        client->available = false;
    }
    pthread_rwlock_unlock(&db.lock);
    
    server_send_response(client, CLIENT_GROUP_BROADCAST_START);
    return 0;
}
*/

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

void list_print(List *list)
{
    printf("[LOG] Active user list: ");
    List_Node *node = list->head;
    if (node == NULL)
        printf("none\n");
    else
    {
        pthread_rwlock_rdlock(&list->lock);
        {
            while (node)
            {
                printf(" %s ->", node->client->name);
                node = node->next;
            }
        }
        pthread_rwlock_unlock(&list->lock);
        printf(" NULL\n");
    }

}

void list_send_client_names(Client *client)
{
    char buffer[MAXLINE+1];
    List_Node *node = db.head;
    int j, k, i = 0;
    
    pthread_rwlock_rdlock(&db.lock);
    {
        while (node)
        {
            k = j = 0;
            while (node->client->name[k] != '\0')
            {
                buffer[i++] = node->client->name[k++];
            }
            buffer[i++] = '\n';
            node = node->next;
        }
        buffer[i-1]= '\0';
    }
    pthread_rwlock_unlock(&db.lock);

    server_sendline(client, buffer, MAXLINE);
}

void * server_recv(void *pclient)
{
    Client *client = (Client *)pclient;
    char recvline[MAXLINE+1];
    Client *member = NULL;

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

        Msg_Type request = cstr_as_msg(recvline);

        // Handles the request accordingly
        printf("[LOG] (%s) messaged server: %s\n", client->name, recvline);
        switch (request) 
        {
            case ACTIVE_USERS:
                server_send_response(client, ACTIVE_USERS);
                list_send_client_names(client);
                break;

            case CLIENT_USERNAME_TAKEN:
                server_send_response(client, CLIENT_USERNAME_TAKEN);
                server_recvline(client, recvline, MAXWORD);
                break;

            case CLIENT_REGISTERED:
                server_send_response(client, CLIENT_REGISTERED);
                break;

             case CLIENT_ADD_FRIEND:
                server_send_response(client, CLIENT_ADD_FRIEND);
                client_add_friend(client);
                break;

             case CLIENT_CHAT_SETUP:
                switch (client_friend_chat_setup(client))
                {
                    case -1:
                        server_send_response(client, CLIENT_PARTNER_NULL);
                        break;
                    case -2:
                        server_send_response(client, CLIENT_PARTNERS_PARTNER_NULL);
                        break;
                    case -3:
                        server_send_response(client, CLIENT_UNAVAILABLE);
                        break;
                    case -4:
                        server_send_response(client, FAILED);
                        break;
                    case -5:
                        server_send_response(client, CLIENT_NOT_FOUND);
                        break;
                }
                break;

             case CLIENT_CHAT_START:
                client_friend_chat_thread_create(client);
                break;
/*
             case CLIENT_GROUP_ADD_MEMBER:
                server_send_response(client, CLIENT_GROUP_ADD_MEMBER);
                member = client_group_get_member(client);
                if (member == NULL)
                    break;
                client_group_add_member(client, member);
                break;

             case CLIENT_GROUP_BROADCAST_SETUP:
                client_group_setup(client);
                break;

             case CLIENT_GROUP_BROADCAST_START:
                client_group_broadcast_thread_create(client);
                break;
*/

             default:
                fprintf(stderr, "[ERR] server_request: request is invalid\n");
                break;
        }
    }
    return NULL;
}

Client *client_group_get_member(Client *client)
{
    char recvline[MAXWORD+1];
    server_recvline(client, recvline, MAXWORD);

    Client *member = list_get_client_by_name(recvline, &client->group_members);
    if (member == NULL)
    {
        fprintf(stderr, "[ERR] client_group_add_member: member not found\n");
        server_send_response(client, CLIENT_NOT_FOUND);
        return NULL;
    }
    else if (member == client)
    {
        fprintf(stderr, "[ERR] client_group_add_member: client choose itself\n");
        server_send_response(client, CLIENT_CHOOSE_ITSELF);
        return NULL;
    }
    return member;
}

int server_sendline(Client *client, char buffer[], int limit)
{
    if (buffer[0] == '\0')
    {
        fprintf(stderr, "[ERR] server_sendline: buffer empty\n") ;
        return -2;
        
    }
    if (send(client->socket, buffer, limit, 0) < 0)
    {
        fprintf(stderr, "[ERR] server_sendline: error while sending\n");
        server_remove_client(client);
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

void server_send_response(Client *client, Msg_Type request)
{
    char *str_msg = msg_as_cstr(request);
    server_sendline(client, str_msg, MAXMSG);
    printf("[LOG] server messaged %s : %s\n", client->name[0] == 0 ? "UNKOWN":client->name, str_msg);
}

Client * list_get_client_by_name(char *name, List *list)
{
    List_Node *node = list->head;
    pthread_rwlock_rdlock(&db.lock);
    {
        while (node)
        {
            if (strcmp(node->client->name, name) == 0)
            {
                pthread_rwlock_unlock(&db.lock);
                return node->client;
            }
            node = node->next;
        }
    }
    pthread_rwlock_unlock(&db.lock);
    return NULL;
}

Client * client_add_friend(Client *client)
{
    char recvline[MAXLINE+1];

    server_recvline(client, recvline, MAXWORD);
    Client *other_client = list_get_client_by_name(recvline, &db);
    if (other_client == NULL)
    {
       fprintf(stderr, "[ERR] client_add_friend: client choose invalid client\n");
       server_send_response(client, CLIENT_NOT_FOUND);
       return NULL;
    } 
    else if (client == other_client)
    {
       fprintf(stderr, "[ERR] client_add_friend: client choose him/her self\n");
       server_send_response(client, CLIENT_CHOOSE_ITSELF);
       return NULL;
    } 
    
    List_Node *friend_node = malloc(sizeof(List_Node));
    if (friend_node == NULL)
    {
        fprintf(stderr, "[client_add_friend: failed to allocate\n");
        return NULL;
    }
    friend_node->client = other_client;
    friend_node->next = NULL;

    pthread_rwlock_wrlock(&db.lock);
    {
        if (ll_append(&client->friends_list, friend_node))
            client->friends_list.count++;
    }
    pthread_rwlock_unlock(&db.lock);

    server_send_response(client, CLIENT_PARTNER_SELECTED);

    return 0;
}

int client_friend_chat_setup(Client *client)
{
    char recvline[MAXLINE+1];
    Client *friend = NULL;

    if (client->friends_list.head == NULL)
    {
        // If the client has not friend
        fprintf(stderr, "[ERR] client_friend_chat_setup: clients friend list is empty\n");
        return -1;
    }
    else if (client->friends_list.head != NULL && client->friends_list.head->next == NULL)
    {
        // If the client has only a single friend
        friend = client->friends_list.head->client;
    }
    else if (client->friends_list.head != NULL)
    {
        // If the client has more than one friend
        server_send_response(client, CLIENT_CHOOSE_FRIEND);
        server_recvline(client, recvline, MAXWORD);
        friend = list_get_client_by_name(recvline, &client->friends_list);
        if (friend == NULL)
        {
            fprintf(stderr, "[ERR] client_friend_chat_setup: friend not found\n");
            return -5;
        }
        if (friend->friends_list.head == NULL || friend->friends_list.tail == NULL)
        {
            fprintf(stderr, "[ERR] client_friend_chat_setup: friends friends list is empty\n");
            return -2;
        }
    }

    pthread_rwlock_wrlock(&db.lock);
    {
        // Checking whether the client is in "friend`s" friends list
        if (list_is_client_in_list(client, &friend->friends_list) == false)
        {
            fprintf(stderr, "[LOG] client_friend_chat_setup: client not found in friend`s friends list\n");
            pthread_rwlock_unlock(&db.lock);
            return -2;
        }
        else if (friend->available == true)
        {
            // he/she is available and wants to talk to you
            client->available = friend->available = false;
            client->_friend = friend;
            friend->_friend = client;
        }
        else if (friend->available == false)
        {
            fprintf(stderr, "[LOG] client_friend_chat_setup: client is busy\n");
            pthread_rwlock_unlock(&db.lock);
            return -3;
        }
        else 
        {
            fprintf(stderr, "[LOG] client_friend_chat_setup: unkown error\n");
            return -4;
        }
    }
    pthread_rwlock_unlock(&db.lock);

    server_send_response(client, CLIENT_CHAT_START);
    server_send_response(friend, CLIENT_CHAT_START);
    return 0;
}

int client_friend_chat_thread_create(Client *client)
{
    pthread_t tid1;

    if (pthread_create(&tid1, NULL, client_friend_chat_thread_handler, client) != 0)
    {
        fprintf(stderr, "[ERR] client_friend_chat_thread_handler: unable to create thread\n");
       return errno;
    }

    if (pthread_join(tid1, NULL) != 0)
    {
        fprintf(stderr, "[ERR] client_friend_chat_thread_handler: unable to join client thread\n");
       return errno;
    }

    return 0;
}
/*
int client_group_broadcast_thread_create(Client *client)
{
    pthread_t tid1;

    printf("[LOG] %s thread %i created\n", client->name, 1);
    if (pthread_create(&tid1, NULL, client_group_broadcast_thread_handler, client) != 0)
    {
        fprintf(stderr, "[ERR] client_group_broadcast_thread_handler: unable to create thread\n");
        return errno;
    }

    if (pthread_join(tid1, NULL) != 0)
    {
        fprintf(stderr, "[ERR] client_group_broadcast_thread_handler: unable to join client thread\n");
        return errno;
    }
    printf("[LOG] %s thread %i closed\n", client->name, 1);

    return 0;
}
void *client_group_broadcast_thread_handler(void *pclient)
{
    char recvline[MAXLINE+1];
    char sendline[MAXSND+1];
    char mssg[MAXLINE+1];
    int recv_output;

    Client *client = (Client *)pclient;
    Client **node = client->group;
    snprintf(mssg, MAXLINE, "%s left\n", client->name);

    printf("[LOG] %s initiated group chat\n", client->name);
    while (client->group_chat_active)
    {
        // Recieves text from the client
        recv_output = recv(client->socket, recvline, MAXLINE, 0 );
        if (recv_output == -1)
            continue;
        else if (recv_output == 0)
            break;

        // Users wants to quit
        if (strcmp(recvline, "CLIENT_CHAT_QUIT") == 0)
            break;

        // Sends the text to every client members
        snprintf(sendline, MAXSND, "%s: %s", client->name, recvline);
        for (int i = client->top; i > -1; i--)
        {
            if (node[i]->group_chat_active == false)
                continue;
            if (send(node[i]->socket, sendline, MAXLINE, 0) < 0)
            {
                pthread_rwlock_wrlock(&client->lock);
                {
                    client->group_chat_active = false;
                    if (client->friend != NULL)
                        server_sendline(client->friend, mssg, MAXLINE);
                    server_send_response(client, CLIENT_GROUP_BROADCAST_CLOSE);
                }
                pthread_rwlock_unlock(&client->lock);
                fprintf(stderr, "[ERR] client2 send error\n");
                return NULL;
            }
        }
    }
    pthread_rwlock_wrlock(&client->lock);
    {
        client->group_chat_active = false;
        if (client->friend != NULL)
            server_sendline(client->friend, mssg, MAXLINE);
        server_send_response(client, CLIENT_GROUP_BROADCAST_CLOSE);
    }
    pthread_rwlock_unlock(&client->lock);
    return NULL;
}
*/

void *client_friend_chat_thread_handler(void *pclient)
{
    char recvline[MAXLINE+1];
    char sendline[MAXSND+1];
    char mssg[MAXLINE+1];
    int recv_output;

    Client *client = (Client *)pclient;
    snprintf(mssg, MAXLINE, "%s left\n", client->name);

    printf("[LOG] %s <-> %s chat initiated\n", client->name, client->_friend->name);
    while (client->available == false)
    {
        // get the buffer from the client
        recv_output = recv(client->socket, recvline, MAXLINE, 0 );
        if(recv_output == -1)
            continue;
        else if (recv_output == 0)
            break;

        // if the client quits
        if (strcmp(recvline, "CLIENT_CHAT_QUIT") == 0)
            break;

        // Checking if the other client exits from the chat or the server
        if (client->_friend == NULL || client->_friend->available == true )
        {
            server_send_response(client, CLIENT_CHAT_CLOSED);
            return NULL;
        }
        else 
            snprintf(sendline, MAXSND, "%s: %s", client->name, recvline);

        // send the buffer to the other client
        if (send(client->_friend->socket, sendline, MAXLINE, 0) < 0)
        {
            fprintf(stderr, "[ERR] client2 send error\n");
            break;
        }
    }
    pthread_rwlock_wrlock(&client->friends_list.lock);
    {
        client->available = true;
        client->_friend->available = true;
        server_sendline(client->_friend, mssg, MAXLINE);
        server_send_response(client, CLIENT_CHAT_CLOSED);
        client->_friend = NULL;
        client->_friend->_friend = NULL;
    }
    pthread_rwlock_unlock(&client->friends_list.lock);
    return NULL;
}

void server_add_client(Client *client)
{
    List_Node *node = malloc(sizeof(List_Node));
    if (node == NULL)
    {
        fprintf(stderr, "server_add_client: failed to allocate memory for node \n");
        return ;
    }
    node->client = client;
    node->next = NULL;
    client->db_node_ref = node;
    pthread_rwlock_wrlock(&db.lock);
    {
        ll_append(&db, node);
        db.count++;
    }
    pthread_rwlock_unlock(&db.lock);
}

void server_remove_client(Client *client)
{
    fprintf(stderr, "[LOG] Client (%s) disconnected\n", client->name);
    close(client->socket);

    pthread_rwlock_wrlock(&db.lock);
    {
        db.count--;
        if (client->friends_list.head != NULL) 
        {
            client->_friend->available = true;
            if (client->_friend->_friend == client)
            {
                client->_friend->available = true;
                client->_friend->_friend = NULL;
            }
        }
        
        switch (ll_delete_node(&db, client->db_node_ref))
        {
            case 0:
                fprintf(stderr, "[LOG] ll_delete_node: element deleted\n");
                break;
            case 1:
                fprintf(stderr, "[LOG] ll_delete_node: list empty\n");
                break;
            case 2:
                fprintf(stderr, "[LOG] ll_delete_node: element not found\n");
                break;
        }
    }
    pthread_rwlock_unlock(&db.lock);

}

bool list_is_client_in_list(Client *client, List *list)
{
    List_Node *node = list->head;
    if (node == NULL)
        return false;

    if (list->head->client == client || list->tail->client == client)
        return true;
    while (node)
    {
        if (node->client == client)
            return true;
        node = node->next;
    }
    return false;
}
