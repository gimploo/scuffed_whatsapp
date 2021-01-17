#include "common.h"

void cstring_input(char *message, char buffer[])
{
    if (message != NULL)
        printf("%s", message);
    fgets(buffer, MAXLINE, stdin);
    buffer[strlen(buffer)-1] = '\0';
}

char * msg_to_cstr(MSG_TYPE msg)
{
    switch (msg) {
        case SUCCESS:                   return "SUCCESS";
        case FAILED:                    return "FAILED";
        case ASK:                       return "ASK";
        case ACTIVE_USERS:              return "ACTIVE_USERS";
        case CLIENT_SAME_USER:          return "CLIENT_SAME_USER";
        case CLIENT_UNAVAILABLE:        return "CLIENT_UNAVAILABLE";
        case CLIENT_NOT_FOUND:          return "CLIENT_NOT_FOUND";
        case CLIENT_CHOOSE_PARTNER:     return "CLIENT_CHOOSE_PARTNER";
        case CLIENT_USERNAME_TAKEN:     return "USERNAME_TAKEN";
        case CLIENT_REGISTERED:         return "CLIENT_REGISTERED";
        case CLIENT_CHAT_START:         return "CLIENT_CHAT_START";
        case CLIENT_CHAT_SETUP:         return "CLIENT_CHAT_SETUP";
        case CLIENT_PARTNER_SELECTED:   return "CLIENT_PARTNER_SELECTED";
        default:                        return "INVALID_MESSAGE";
    }
}

MSG_TYPE cstr_to_msg(char *cstring)
{
    if      (strcmp(cstring, "SUCCESS") == 0)                   return SUCCESS;
    else if (strcmp(cstring, "FAILED") == 0)                    return FAILED;
    else if (strcmp(cstring, "ACTIVE_USERS") == 0)              return ACTIVE_USERS;
    else if (strcmp(cstring, "CLIENT_REGISTERED") == 0)         return CLIENT_REGISTERED;
    else if (strcmp(cstring, "CLIENT_SAME_USER") == 0)          return CLIENT_SAME_USER;
    else if (strcmp(cstring, "CLIENT_NOT_FOUND") == 0)          return CLIENT_NOT_FOUND;
    else if (strcmp(cstring, "CLIENT_UNAVAILABLE") == 0)        return CLIENT_UNAVAILABLE;
    else if (strcmp(cstring, "USERNAME_TAKEN") == 0)            return CLIENT_USERNAME_TAKEN;
    else if (strcmp(cstring, "CLIENT_CHAT_SETUP") == 0)         return CLIENT_CHAT_SETUP;
    else if (strcmp(cstring, "CLIENT_CHAT_START") == 0)         return CLIENT_CHAT_START;
    else if (strcmp(cstring, "CLIENT_CHOOSE_PARTNER") == 0)     return CLIENT_CHOOSE_PARTNER;
    else if (strcmp(cstring, "CLIENT_PARTNER_SELECTED") == 0)   return CLIENT_PARTNER_SELECTED;
    else                                                        return INVALID;
}
