#ifndef IPV4_CLIENT_H
#define IPV4_CLIENT_H

static char* DEFAULT_PLAYER_CMD = "/usr/bin/mpg123 - > /dev/null 2>&1";

class  client {
public:
    struct client_conf_st{
        int rcvport;
        char *mgroup;
        char *player_cmd;
    };
};

#endif //IPV4_CLIENT_H