#ifndef IPV4_PROTO_H
#define IPV4_PROTO_H

#include "site_type.h"

char * DEFAULT_MGROUP = "224.2.2.2";
const int default_port = 2023;

const int rcv_channel = 100; // 100 is reserved for rcv channel
const int list_channel = 0; // 0 is reserved for list channel
const int min_channel = 1; // 1 is reserved for min channel
const int max_channel = min_channel + rcv_channel - 1; // 100 is reserved for max channel

const int max_msg_size = 65536 - 20 - 8; // 65536 is max UDP packet size, 20 is IP header size, 8 is UDP header size
const int max_data_size = max_msg_size - sizeof(chnid_t);

const int max_list_size = 65536 - 20 - 8; // 65536 is max UDP packet size, 20 is IP header size, 8 is UDP header size
const int max_list_entry_size = max_list_size - sizeof(chnid_t);

class proto {
public:
    struct msg_channel_st {
        chnid_t chnid; // must between min_channel and max_channel
        uint8_t data[1]; // variable length
    } __attribute__((packed));

    struct msg_list_entry_st {
        chnid_t chnid;
        uint16_t desc_len;
        uint8_t desc[1];
    } __attribute__((packed));

    /* example:
     * 1. music: xxxxxxxxxxxxxxxxxxxxxx
     * 2. video: xxxxxxx
     * 3. text: xxxxxxxxxxxxx
     * ...
     * */
    struct msg_list_st {
        chnid_t chnid; // must be list_channel
        msg_list_entry_st entry[1]; // variable length
    } __attribute__((packed));

};


#endif //IPV4_PROTO_H
