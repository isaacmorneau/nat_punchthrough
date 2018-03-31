#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "punching_engine.h"
#include "wrappers/wrapper.h"

#define BUFF 512

typedef struct {
    int out_port;
    int in_port;
} punch_packet;

void start_punch(const char * host, const int port, const int range) {
    int efd = make_epoll();
    struct epoll_event * events = make_epoll_events();

    struct sockaddr_storage * addresses = (struct sockaddr_storage*)calloc(range, sizeof(struct sockaddr_storage));
    int n = 0, offset = 0;
    int * sockets = calloc(range, sizeof(int));


    //fill out the addresses we are going to try and send to
    for (int i = 0; i < range; ++i) {
        make_storage(addresses + i, host, port + i);
        sockets[i] = make_bound_udp(port + i);
        add_epoll_fd_flags(efd, sockets[i], EVENT_ONLY_IN);
    }

    punch_packet inpack = {0}, outpack = {0};

try:
    if (inpack.out_port) {
        //we got a message telling us what worked
        outpack.in_port = inpack.out_port;
    }
    for (int i = 0; i < range; ++i) {
        outpack.out_port = htonl(port + i);
        send_message(sockets[i], (char *)&outpack, sizeof(punch_packet), addresses + i);
    }
    n = wait_epoll_timeout(efd, events, 1000);
    for (int i = 0; i < n; ++i) {
        if (EVENT_IN(events, i)) {
            read_message(EVENT_FD(events, i), (char *)&inpack, sizeof(punch_packet));
            printf("recived op %d ip %d\n", ntohl(inpack.out_port), ntohl(inpack.in_port));
            if (inpack.in_port) {
                offset = ntohl(inpack.in_port) - port;
                printf("%d-%d=offset%d\n", ntohl(inpack.in_port), port, offset);
                goto maintain;
            }
        }
    }
    goto try;
maintain:
    send_message(sockets[offset], (char *)&outpack, sizeof(punch_packet), addresses + offset);
    n = wait_epoll_timeout(efd, events, 1000);
    for (int i = 0; i < n; ++i) {
        if (EVENT_IN(events, i)) {
            read_message(EVENT_FD(events, i), (char *)&inpack, sizeof(punch_packet));
            printf("connection betweeen op %d ip %d steady\n", inpack.out_port, inpack.in_port);
        }
    }
    goto maintain;
}
