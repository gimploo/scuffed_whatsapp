#include "common.h"

char * msg_to_cstr(MSG_TYPE msg)
{
    switch (msg) {
        case SUCCESS:               return "SUCCESS";
        case FAILED:                return "FAILED";
        case CLIENT_NOT_FOUND:      return "CLIENT_NOT_FOUND";
        case CLIENT_UNAVAILABLE:    return "CLIENT_UNAVAILABLE";
        case CLIENT_RECIEVE:        return "CLIENT_RECIEVE";
        case CLIENT_ACTIVE_USERS:   return "GIVE_ACTIVE_USERS";
        case CLIENT_SET_PARTNER:    return "CLIENT_SET_PARTNER";
        case CLIENT_USERNAME_TAKEN: return "USERNAME_TAKEN";
        case CLIENT_REGISTERED:     return "CLIENT_REGISTERED";
        default:                    return "INVALID_MESSAGE";
    }
}

MSG_TYPE cstr_to_msg(char *cstring)
{
    if (strcmp(cstring, "SUCCESS") == 0)
        return SUCCESS;
    else if (strcmp(cstring, "FAILED") == 0)
        return FAILED;
    else if (strcmp(cstring, "CLIENT_SET_PARTNER") == 0)
        return CLIENT_SET_PARTNER;
    else if (strcmp(cstring, "GIVE_ACTIVE_USERS") == 0)
        return CLIENT_ACTIVE_USERS;
    else if (strcmp(cstring, "CLIENT_REGISTERED") == 0)
        return CLIENT_REGISTERED;
    else if (strcmp(cstring, "USERNAME_TAKEN") == 0)
        return CLIENT_USERNAME_TAKEN;
    else if (strcmp(cstring,  "CLIENT_NOT_FOUND") == 0)
        return CLIENT_NOT_FOUND;
    else if (strcmp(cstring,  "CLIENT_UNAVAILABLE") == 0)
        return CLIENT_UNAVAILABLE;
    else if (strcmp(cstring,  "CLIENT_RECIEVE") == 0)
        return CLIENT_RECIEVE;
    else 
        return INVALID;
}
