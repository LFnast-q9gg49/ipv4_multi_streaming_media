#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>

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
    fprintf(stdout,"-M --mgroup  multicast group\n"
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
    struct option arg_arr[] = {
            {"mgroup", 1, nullptr, 'M'},
            {"port",   1, nullptr, 'P'},
            {"player", 1, nullptr, 'p'},
            {"help",   0, nullptr, 'H'},
            {nullptr,  0, nullptr,  0}
    };

    while (true) {
        ret = getopt_long(argc, argv, "M:P:p:H", &arg_arr, &index);
        if (ret < 0){
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

    setsockopt();

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