#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <fcntl.h>


#include "client.h"
#include "../include/proto.h"

/*
 * -M --mgroup  multicast group
 * -P --port    port
 * -p --player  named player
 * -H --help    help
 * */

static char *DEFAULT_PLAYER = "/usr/bin/mpg123 > /dev/null 2>&1";

struct client::client_conf_st client_conf = {
        .rcvport = default_port,
        .mgroup = DEFAULT_MGROUP,
        .player_cmd = DEFAULT_PLAYER
};

static void usage() {
    fprintf(stdout, "-M --mgroup  multicast group\n"
                    "-P --port    port\n"
                    "-p --player  named playe\n"
                    "-H --help    help\n");
}

int main(int argc, char *argv[]) {
    /*
     * init level :
     * default,
     * config,
     * environment,
     * command line
     * */
    pid_t pid;
    int index, ret, sid;
    struct ip_mreqn mreq{};
    struct sockaddr_in laddr{};


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
        perror("setsockopt");
        exit(1);
    }

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(client_conf.rcvport);
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sid, reinterpret_cast<sockaddr *>(&laddr), sizeof(laddr)) < 0) {
        perror("bind");
        exit(1);
    }

    pipe();

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // child
        close(0);
        close(1);
        close(2);
        setsid();
        chdir("/");
        umask(0);
        int fd = open("/dev/null", O_RDWR);
        dup(fd);
        dup(fd);
        close(fd);
        // exec mpeg
    }

    // parent


    exit(0);
}