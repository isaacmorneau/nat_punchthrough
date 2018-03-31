#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "punching_engine.h"
#include "wrappers/wrapper.h"

#define BUFF 512

const char * mesg = "hello";
const char * mesg2 = "connected";

void start_punch(const char * host, const int port, const int range) {
    char buffer[BUFF];
    int efd = make_epoll();
    struct epoll_event * events = make_epoll_events();

    struct sockaddr_storage * addresses = (struct sockaddr_storage*)calloc(range, sizeof(struct sockaddr_storage));
    struct sockaddr_storage stable_addr;
    int n = 0, s = 0;
    int * sockets = calloc(range, sizeof(int));


    //fill out the addresses we are going to try and send to
    for (int i = 0; i < range; ++i) {
        make_storage(addresses + i, host, port + i);
        sockets[i] = make_bound_udp(port + i);
        add_epoll_fd_flags(efd, sockets[i], EVENT_ONLY_IN);
    }

    int stable_port = 0;

try:
    for (int j = 0; j < range; ++j) {
        for (int i = 0; i < j; ++i) {
            send_message(sockets[i], (char *)mesg, strlen(mesg), addresses + i);
        }
        n = wait_epoll_timeout(efd, events, 1000);
        for (int i = 0; i < n; ++i) {
            if (EVENT_IN(events, i)) {
                read_message_port(EVENT_FD(events, i), buffer, BUFF, &stable_port);
                int offset = stable_port - port;

                s = sockets[offset];
                memcpy(&stable_addr, addresses + offset, sizeof(struct sockaddr_storage));

                for (int k = 0; k < range; ++k)
                    if (k != offset)
                        close(sockets[k]);

                free(addresses);
                free(sockets);
                goto maintain;
            }
        }
    }
    goto try;
maintain:
    send_message(s, (char*)mesg2, strlen(mesg2), &stable_addr);
    n = wait_epoll_timeout(efd, events, 1000);
    for (int i = 0; i < n; ++i) {
        if (EVENT_IN(events, i)) {
            read_message(EVENT_FD(events, i), buffer, BUFF);
            puts("connection steady");
        }
    }
    if (!n) {
        puts("connection timing out");
    }
    goto maintain;
}
