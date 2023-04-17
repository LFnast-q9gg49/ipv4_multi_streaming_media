#ifndef IPV4_CLIENT_H
#define IPV4_CLIENT_H

class  client {
public:
    struct client_conf_st{
        int rcvport;
        char *mgroup;
        char *player_cmd;
    };
};

#endif //IPV4_CLIENT_H