#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include<netinet/tcp.h>
#include<netinet/ip.h>

#define SOCKOPTS "p:r:a:h"
#define PORT 44000
#define RANGE 20

static inline void print_help() {
    puts("usage options:\n"
            "\t [p]ort <1-65535>        - the port to try and connect on [default 44444]\n"
            "\t [r]ange <1-65535>       - the range either side to try [default 20]\n"
            "\t [a]ddress <url || ip>   - the source address\n"
            "\t [h]elp                  - this message\n"
          );
}

int main(int argc, char ** argv) {
    char * addr = 0;
    int range = RANGE, port = PORT;
    int c;
    int option_index = 0;

    static struct option long_options[] = {
        {"port",    required_argument, 0, 'p'},
        {"range",   required_argument, 0, 'r'},
        {"address", required_argument, 0, 'a'},
        {"help",    no_argument,       0, 'h'},
        {0,         0,                 0, 0}
    };
    for (;;) {
        if ((c = getopt_long(argc, argv, SOCKOPTS, long_options, &option_index)) == -1) {
            break;
        }

        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'r':
                range = atoi(optarg);
                break;
            case 'a':
                addr = optarg;
                break;
            case 'h':
            case '?':
            default:
                print_help();
                return 0;
        }
    }
    if (!port || !addr) {
        print_help();
        return 1;
    }

    return 0;
}
