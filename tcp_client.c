#include "common.h"

volatile bool connected = false;
pthread_mutex_t lock1;

Client *ll_head = NULL;
Client *ll_tail = NULL;

Client * client_init(int server_port);
void client_thread_init(Client *client);
void * client_send(void *pclient);
void * client_recv(void *client);

bool get_active_users(Client *client);
void send_client_info(Client *client);

void client_sendline(Client *client, char buffer[], int limit);
void client_recvline(Client *client, char buffer[], int limit);

void client_send_partner(Client *client);

int main(int argc, char *argv[])
{
    char choice;
    char str_servaddr[MAXLINE];

    // Deciding on a server address 
    if (argc != 2)
    {
        printf("USAGE: %s <ipaddr>\n", argv[0]);
        printf("[?] Check localhost (y or n): ");
        if ((choice = getchar()) == 'y')
        {
            printf("[*] Checking localhost ....\n");
            snprintf(str_servaddr,IPV4_STRLEN, LOCALHOST) ;
        }
        else
        {
            printf("[!] Quiting!\n");
            return 1;
        }
        CLEAR_STDIN();
    }
    else
    {
        printf("[*] Checking %s .... \n", argv[1]);
        snprintf(str_servaddr, MAXLINE, "%s", argv[1]);
    }
    printf("[!] CONNECTED TO %s\n", 
            strcmp(str_servaddr,LOCALHOST) == 0 ? "localhost":str_servaddr);
    

    // Creating the client struct
    Client *client = client_init(SERVER_PORT);
    send_client_info(client);
    get_active_users(client);

    printf("[!] ENTER \"exit()\" TO QUIT '(^u^)'\n");
    client_thread_init(client);

    // Clean up
    printf("[!] Client Closed!\n");
    close(client->socket);
    free(client);

    return 0;
}

void client_thread_init(Client *client)
{
    pthread_t  tid1; 
    pthread_t  tid2;

    // Thread1: send message
    if (pthread_create(&tid2,NULL, client_send, client) != 0)
        ERROR_HANDLE();

    // Thread2: recieve message
    if (pthread_create(&tid1,NULL, client_recv, client) != 0)
        ERROR_HANDLE();

    pthread_join(tid2, NULL);
}

void * client_send(void *pclient)
{
    Client *client = (Client *)pclient;
    char sendline[MAXLINE];
    char chr;
    while (connected)
    {
        printf("%s : ", client->name);
        cstring_input(NULL, sendline);

        if (strcmp(sendline, "exit()") == 0)
        {
            printf("\n[?] Do you want to exit (y or n): ");
            if ((chr = getchar()) == 'n')
            {
                cstring_input(NULL, sendline);
                continue;
            }
            else
            {
                pthread_mutex_lock((&lock1));
                connected = false;
                pthread_mutex_unlock((&lock1));
                return NULL;
            }
            CLEAR_STDIN();
        }

        if (strcmp(sendline, "\n") == 0)
            continue;

        //TESTING
        else if (strcmp(sendline , "active") == 0)
        {
            strcpy(sendline, msg_to_cstr(CLIENT_ACTIVE_USERS));
            client_sendline(client, sendline, MAXWORD);
            get_active_users(client);
            continue;
        }
        else if (strcmp(sendline, "msg") == 0)
        {
            strcpy(sendline, msg_to_cstr(CLIENT_SET_PARTNER));
            client_sendline(client, sendline, MAXWORD);
            client_send_partner(client);
            continue;
        }

        client_sendline(client, sendline, strlen(sendline));

    }
    return NULL;
}

void * client_recv(void *pclient)
{
    Client *client = (Client *)pclient;
    char recvline[MAXLINE];
    char choice[4];

    while (connected)
    {
        client_recvline(client, recvline, MAXLINE);

        // TODO: have get_active_users func be handled by this thread  

        switch(cstr_to_msg(recvline))
        {
            case ASK:
                client_recvline(client, recvline, MAXWORD);
                cstring_input("[?] choice (yes or no): ", choice);
                client_sendline(client, choice, 4);
                client_recvline(client, recvline, MAXWORD);
                break;
            case WAIT:
                printf("[!] Waiting ....\n");
                client_recvline(client, recvline, MAXWORD);
                break;
            case SUCCESS:
                client->available = false;
                printf("[!] PRIVATE CONNECTION ESTABLISHED\n");
                break;
            default:
                fprintf(stderr, "client_recv: %s\n", recvline);
                break;
        }

    }
    return NULL;
}


Client * client_init(int server_port)
{
    Client *client = malloc(sizeof(Client));
    cstring_input("[?] Name: ", client->name);
    client->available = true;
    client->next = NULL;
    client->partner = NULL;

    if ((client->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERROR_HANDLE();

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);

    if (connect(client->socket, 
                (struct sockaddr *) &servaddr, 
                sizeof(servaddr)) < 0)
        ERROR_HANDLE();

    connected = true;

    return client;
}

void send_client_info(Client *client)
{
    char recvline[MAXLINE];
    char new_name[MAXWORD];
    char name_taken[MAXWORD];
    bool username_is_changed = false;

    client_sendline(client, client->name, MAXWORD);

    client_recvline(client, recvline, MAXLINE);
    
    strcpy(name_taken, client->name);

    // While username is taken
    while (strcmp(recvline, msg_to_cstr(CLIENT_USERNAME_TAKEN)) == 0)
    {
        username_is_changed = true;
        do {
            cstring_input("[?] That name is taken, make another: ", new_name);
        } while (strcmp(new_name, name_taken) == 0);

        client_sendline(client, new_name, MAXWORD);

        client_recvline(client, recvline, MAXLINE);

        if (strcmp(recvline,msg_to_cstr(CLIENT_USERNAME_TAKEN)) == 0)
            strcpy(name_taken, new_name);
    }
    if (username_is_changed) strcpy(client->name, new_name);

}

void client_send_partner(Client *client)
{
    char sendline[MAXWORD+1];

    client_sendline(client, msg_to_cstr(CLIENT_SET_PARTNER), MAXWORD);

    get_active_users(client);

    cstring_input("[?] Who do u want to talk with: ", sendline);

    client_sendline(client, sendline, MAXWORD);
}

bool get_active_users(Client *client)
{
    char buffer[MAXLINE];
    bool isempty = true;
    int i = 0;

    printf("\n-----------------\n");
    printf("  Active Users\n");
    printf("-----------------\n");
    client_recvline(client, buffer, MAXLINE);
    while (buffer[i] != '\0')
    {
        printf("%c", buffer[i]);
        if (buffer[i] == '\n')
            isempty = false;
        i++;
    }
    printf("\n-----------------\n\n");
    return isempty;
}

void client_sendline(Client *client, char buffer[], int limit)
{
    /*if (buffer[0] == '\0')*/
    /*{*/
        /*fprintf(stderr, "[!] client_sendline: buffer empty\n") ;*/
        /*exit(1);*/
    /*}*/
    if (send(client->socket, buffer, limit, 0) < 0)
        ERROR_HANDLE();
}

void client_recvline(Client *client, char buffer[], int limit)
{
    memset(buffer, 0, limit);
    switch(recv(client->socket, buffer, limit, 0))
    {
        case 0:
            pthread_mutex_lock((&lock1));
            connected = false;
            pthread_mutex_unlock((&lock1));
            printf("[!] Connection closed by server\n");
            exit(-1);
        case -1:
            ERROR_HANDLE();
    }
}

