#include "common.h"

void cstring_input(char *message, char buffer[], int limit)
{
    if (message != NULL)
        printf("%s", message);
    fgets(buffer, limit, stdin);
    buffer[strlen(buffer)-1] = '\0';
}

char * msg_as_cstr(Msg_Type msg)
{
    switch (msg) {
        case SUCCESS:                       return "SUCCESS";
        case FAILED:                        return "FAILED";
        case ASK:                           return "ASK";
        case ACTIVE_USERS:                  return "ACTIVE_USERS";
        case CLIENT_CHOOSE_ITSELF:          return "CLIENT_CHOOSE_ITSELF";
        case CLIENT_UNAVAILABLE:            return "CLIENT_UNAVAILABLE";
        case CLIENT_NOT_FOUND:              return "CLIENT_NOT_FOUND";
        case CLIENT_ADD_FRIEND:             return "CLIENT_ADD_FRIEND";
        case CLIENT_REMOVE_FRIEND:          return "CLIENT_REMOVE_FRIEND";
        case CLIENT_CHOOSE_FRIEND:          return "CLIENT_CHOOSE_FRIEND";
        case CLIENT_USERNAME_TAKEN:         return "USERNAME_TAKEN";
        case CLIENT_REGISTERED:             return "CLIENT_REGISTERED";
        case CLIENT_CHAT_START:             return "CLIENT_CHAT_START";
        case CLIENT_CHAT_QUIT:              return "CLIENT_CHAT_QUIT";
        case CLIENT_CHAT_CLOSED:            return "CLIENT_CHAT_CLOSED";
        case CLIENT_CHAT_SETUP:             return "CLIENT_CHAT_SETUP";
        case CLIENT_PARTNER_NULL:           return "CLIENT_PARTNER_NULL";
        case CLIENT_PARTNER_SELECTED:       return "CLIENT_PARTNER_SELECTED";
        case CLIENT_PARTNERS_PARTNER_NULL:  return "CLIENT_PARTNERS_PARTNER_NULL";
        case CLIENT_GROUP_EMPTY:            return "CLIENT_GROUP_EMPTY";
        case CLIENT_GROUP_ADD_MEMBER:       return "CLIENT_GROUP_ADD_MEMBER";
        case CLIENT_GROUP_MEMBER_EXIST:     return "CLIENT_GROUP_MEMBER_EXIST";
        case CLIENT_GROUP_OVERFLOW:         return "CLIENT_GROUP_OVERFLOW";
        case CLIENT_GROUP_CHATROOM_SETUP:   return "CLIENT_GROUP_CHATROOM_SETUP";
        case CLIENT_GROUP_CHATROOM_START:   return "CLIENT_GROUP_CHATROOM_START";
        case CLIENT_GROUP_CHATROOM_CLOSE:   return "CLIENT_GROUP_CHATROOM_CLOSE";
        case CLIENT_GROUP_MEMBER_ADDED:     return "CLIENT_GROUP_MEMBER_ADDED";
        default:                            return "INVALID_MESSAGE";
    }
}

Msg_Type cstr_as_msg(char *cstring)
{
    if      (strcmp(cstring, "SUCCESS") == 0)                       return SUCCESS;
    else if (strcmp(cstring, "FAILED") == 0)                        return FAILED;
    else if (strcmp(cstring, "ACTIVE_USERS") == 0)                  return ACTIVE_USERS;
    else if (strcmp(cstring, "CLIENT_REGISTERED") == 0)             return CLIENT_REGISTERED;
    else if (strcmp(cstring, "CLIENT_CHOOSE_ITSELF") == 0)          return CLIENT_CHOOSE_ITSELF;
    else if (strcmp(cstring, "CLIENT_NOT_FOUND") == 0)              return CLIENT_NOT_FOUND;
    else if (strcmp(cstring, "CLIENT_UNAVAILABLE") == 0)            return CLIENT_UNAVAILABLE;
    else if (strcmp(cstring, "USERNAME_TAKEN") == 0)                return CLIENT_USERNAME_TAKEN;
    else if (strcmp(cstring, "CLIENT_CHAT_SETUP") == 0)             return CLIENT_CHAT_SETUP;
    else if (strcmp(cstring, "CLIENT_CHAT_START") == 0)             return CLIENT_CHAT_START;
    else if (strcmp(cstring, "CLIENT_CHAT_QUIT") == 0)              return CLIENT_CHAT_QUIT;
    else if (strcmp(cstring, "CLIENT_CHAT_CLOSED") == 0)            return CLIENT_CHAT_CLOSED;
    else if (strcmp(cstring, "CLIENT_ADD_FRIEND") == 0)             return CLIENT_ADD_FRIEND;
    else if (strcmp(cstring, "CLIENT_REMOVE_FRIEND") == 0)          return CLIENT_REMOVE_FRIEND;
    else if (strcmp(cstring, "CLIENT_CHOOSE_FRIEND") == 0)          return CLIENT_CHOOSE_FRIEND;
    else if (strcmp(cstring, "CLIENT_PARTNER_NULL") == 0)           return CLIENT_PARTNER_NULL;
    else if (strcmp(cstring, "CLIENT_PARTNER_SELECTED") == 0)       return CLIENT_PARTNER_SELECTED;
    else if (strcmp(cstring, "CLIENT_PARTNERS_PARTNER_NULL") == 0)  return CLIENT_PARTNERS_PARTNER_NULL;
    else if (strcmp(cstring ,"CLIENT_GROUP_EMPTY") == 0)            return CLIENT_GROUP_EMPTY;
    else if (strcmp(cstring ,"CLIENT_GROUP_OVERFLOW") == 0)         return CLIENT_GROUP_OVERFLOW;
    else if (strcmp(cstring, "CLIENT_GROUP_CHATROOM_START") == 0)  return CLIENT_GROUP_CHATROOM_START;
    else if (strcmp(cstring, "CLIENT_GROUP_CHATROOM_SETUP") == 0)  return CLIENT_GROUP_CHATROOM_SETUP;
    else if (strcmp(cstring, "CLIENT_GROUP_CHATROOM_CLOSE") == 0)  return CLIENT_GROUP_CHATROOM_CLOSE;
    else if (strcmp(cstring, "CLIENT_GROUP_ADD_MEMBER") == 0)       return CLIENT_GROUP_ADD_MEMBER;
    else if (strcmp(cstring, "CLIENT_GROUP_MEMBER_ADDED") == 0)     return CLIENT_GROUP_MEMBER_ADDED;
    else if (strcmp(cstring, "CLIENT_GROUP_MEMBER_EXIST") == 0)     return CLIENT_GROUP_MEMBER_EXIST;
    else                                                            return INVALID;
}
