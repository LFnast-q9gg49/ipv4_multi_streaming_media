#ifndef IPV4_PROTO_H
#define IPV4_PROTO_H

include "site_type.h"

#define DEFAULT_MGROUP "224.2.2.2";
const int default_port = 5000;

const int rcv_channel = 100; // 100 is reserved for rcv channel
const int list_channel = 0; // 0 is reserved for list channel
const int min_channel = 1; // 1 is reserved for min channel
const int max_channel = min_channel + rcv_channel - 1; // 99 is reserved for max channel

class proto {
    struct msg_channel_st {
        chnid_t chnid;
        char msg[1024];
    };
};


#endif //IPV4_PROTO_H
