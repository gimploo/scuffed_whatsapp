#include "common.h"

volatile bool connected = false;
pthread_mutex_t lock1;

Client *ll_head = NULL;
Client *ll_tail = NULL;

Client * client_init(int server_port);
void client_thread_init(Client *client);
void * client_send(void *pclient);
void * client_recv(void *client);

void print_active_users(char buffer[]);
void client_send_info(Client *client);

void client_sendline(Client *client, char buffer[], int limit);
void client_recvline(Client *client, char buffer[], int limit);

void client_send_message(Client *client, MSG_TYPE request);
void client_set_partner(Client *client);

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
        else if (strcmp(sendline , "a") == 0)
        {
            client_send_message(client, ACTIVE_USERS);
            continue;
        }
        else if (strcmp(sendline, "m") == 0)
        {
            client_send_message(client, CLIENT_SET_PARTNER);
            client_set_partner(client);
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

        switch(cstr_to_msg(recvline))
        {
            case ACTIVE_USERS:
                client_recvline(client, recvline, MAXLINE);
                print_active_users(recvline);
                break;
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
                fprintf(stderr, "\nclient_recv: %s\n", recvline);
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

    // Sending client info to server
    client_send_info(client);
    connected = true;

    return client;
}

void client_send_info(Client *client)
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
            cstring_input("[!] That name is taken\n[?] New Name: ", new_name);
        } while (strcmp(new_name, name_taken) == 0);

        client_sendline(client, new_name, MAXWORD);

        client_recvline(client, recvline, MAXLINE);

        if (strcmp(recvline,msg_to_cstr(CLIENT_USERNAME_TAKEN)) == 0)
            strcpy(name_taken, new_name);
    }
    if (username_is_changed) strcpy(client->name, new_name);

}

void client_set_partner(Client *client)
{
    char sendline[MAXWORD+1];
    cstring_input("[?] Who do u want to talk with: ", sendline);
    client_sendline(client, sendline, MAXWORD);
}

void print_active_users(char buffer[])
{
    int i = 0;

    printf("\n-----------------\n");
    printf("  Active Users\n");
    printf("-----------------\n");
    while (buffer[i] != '\0')
    {
        printf("%c", buffer[i]);
        i++;
    }
    printf("\n-----------------\n\n");
}

void client_sendline(Client *client, char buffer[], int limit)
{
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

void client_send_message(Client *client, MSG_TYPE request)
{
    client_sendline(client, msg_to_cstr(request), MAXMSG);
}
