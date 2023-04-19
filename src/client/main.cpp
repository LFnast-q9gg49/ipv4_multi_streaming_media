#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>


#include "client.h"
#include "../include/proto.h"

/*
 * -M --mgroup  multicast group
 * -P --port    port
 * -p --player  named player
 * -H --help    help
 * */


struct client::client_conf_st client_conf = {
        .rcvport = default_port,
        .mgroup = DEFAULT_MGROUP,
        .player_cmd = DEFAULT_PLAYER_CMD,
};

static void usage() {
    fprintf(stdout, "-M --mgroup  multicast group\n"
                    "-P --port    port\n"
                    "-p --player  named playe\n"
                    "-H --help    help\n");
}

static ssize_t writen(int fd, const void *buf, size_t count) {
    ssize_t ret, pos;
    while (true) {
        ret = write(fd, (char *) buf + pos, count);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("write");
            return -1;
        }
        count -= ret;
        pos += ret;
    }
    return pos;
}

int main(int argc, char *argv[]) {
    /*
     * init level :
     * default,
     * config,
     * environment,
     * command line
     * */
    int pd[2];
    pid_t pid;
    ssize_t len;
    int index, ret, sid, loop = 1, chosen_id;
    struct ip_mreqn mreq{};
    struct sockaddr_in laddr{}, raddr{}, araddr{};
    socklen_t raddr_len = sizeof(raddr);
    socklen_t araddr_len = sizeof(araddr);
    auto *list = (struct proto::msg_list_st *) malloc(max_msg_size);

    struct option arg_arr[] = {
            {"mgroup", 1, nullptr, 'M'},
            {"port",   1, nullptr, 'P'},
            {"player", 1, nullptr, 'p'},
            {"help",   0, nullptr, 'H'},
            {nullptr,  0, nullptr, 0}
    };

    while (true) {
        ret = getopt_long(argc, argv, "M:P:p:H", reinterpret_cast<const option *>(&arg_arr), &index);
        if (ret < 0) {
            break;
        }
        switch (ret) {
            case 'M':
                client_conf.mgroup = optarg;
                break;
            case 'P':
                client_conf.rcvport = atoi(optarg);
                break;
            case 'p':
                client_conf.player_cmd = optarg;
                break;
            case 'H':
                usage();
                exit(0);
            default:
                abort();
                _exit(1);
                break;
        }
    }

    sid = socket(AF_INET, SOCK_DGRAM, 0);
    if (sid < 0) {
        perror("socket");
        exit(1);
    }


    if (inet_pton(AF_INET, client_conf.mgroup, &mreq.imr_multiaddr.s_addr) != 1) {
        perror("inet_pton");
        exit(1);
    }
    mreq.imr_address.s_addr = htonl(INADDR_ANY);
    mreq.imr_ifindex = if_nametoindex("eth0");
    if (mreq.imr_ifindex == 0) {
        perror("if_nametoindex");
        exit(1);
    }

    if (setsockopt(sid, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt ADD_MEMBERSHIP");
        exit(1);
    }

    if (setsockopt(sid, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
        perror("setsockopt MULTICAST_LOOP");
        exit(1);
    }


    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(client_conf.rcvport);
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sid, reinterpret_cast<sockaddr *>(&laddr), sizeof(laddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (pipe(pd) < 0) {
        perror("pipe");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // child

        close(sid);
        close(pd[1]); // close write
        dup2(pd[0], STDIN_FILENO); // redirect stdin to pipe
        if (pd[0] != STDIN_FILENO) {
            close(pd[0]);
        }

        // exec mpeg
        if (execl("/bin/sh", "sh", "-c", client_conf.player_cmd, nullptr) < 0) {
            perror("execl");
            exit(1);
        }
    }

    // parent
    close(pd[0]); // close read


    while (true) {
        // rcv list
        if (list == nullptr) {
            perror("malloc");
            exit(1);
        }

        len = recvfrom(sid, list, max_msg_size, 0, reinterpret_cast<sockaddr *>(&raddr), &raddr_len);
        if (len < sizeof(struct proto::msg_list_st)) {
            fprintf(stderr, "message too short");
            exit(1);
        }
        if (list->chnid != list_channel) {
            fprintf(stderr, "message not list");
            exit(1);
        }
        break;
    }
    // print list
    struct proto::msg_list_entry_st *pos;
    for (pos = list->entry;
         pos < (proto::msg_list_entry_st *) ((char *) list + len); pos = reinterpret_cast<proto::msg_list_entry_st *>(
            (char *) pos + ntohs(pos->desc_len))) {
        fprintf(stdout, "channel %d : %s\n", pos->chnid, pos->desc);
    }

    free(list);

    // choose channel
    while (true) {
        fprintf(stdout, "choose channel id : \n");
        int tmp = scanf("%d", &chosen_id);
        if (tmp != 1) {
            exit(1);
        }
        break;
    }

    // rcv chosen channel, and write to pipe
    auto *chn = (struct proto::msg_channel_st *) malloc(max_msg_size);
    if (chn == nullptr) {
        perror("malloc");
        exit(1);
    }

    while (true) {
        len = recvfrom(sid, chn, max_msg_size, 0, reinterpret_cast<sockaddr *>(&araddr), &araddr_len);
        if (araddr.sin_addr.s_addr != raddr.sin_addr.s_addr || araddr.sin_port != raddr.sin_port) {
            fprintf(stderr, "message from unknown source");
            continue;
        }

        if (len < sizeof(struct proto::msg_channel_st)) {
            fprintf(stderr, "message too short");
            continue;
        }

        if (chn->chnid == chosen_id) {
            fprintf(stdout, "accept msg: %d\n", chn->chnid);
            if (writen(pd[1], chn->data, len - sizeof(chnid_t)) < 0) {
                perror("writen");
                exit(1);
            }
        }

    }

    free(chn);
    close(sid);

    exit(0);
}