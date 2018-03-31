#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "punching_engine.h"
#include "wrappers/wrapper.h"

#define BUFF 1024


void start_punch(const char * host, const int port, const int range) {
    int efd = make_epoll();
    struct epoll_event * events = make_epoll_events();

    struct sockaddr_storage * addresses = (struct sockaddr_storage*)calloc(range, sizeof(struct sockaddr_storage));
    int n, * sockets = calloc(range, sizeof(int));

    char buffer[BUFF];
    for (int i = 0; i < range; ++i) {
        make_storage(addresses + i, host, port + i);
        sockets[i] = make_bound_udp(port + i);
        add_epoll_fd(efd, sockets[i]);
    }
try:
    strncpy(buffer, "message", BUFF);
    for (int i = 0; i < range; ++i) {
        send_message(sockets[i], buffer, strlen(buffer), addresses + i);
    }
    n = wait_epoll_timeout(efd, events, 1000);
    for (int i = 0; i < n; ++i) {
        memset(buffer, 0, BUFF);
        if (EVENT_IN(events, i)) {
            read_message(EVENT_FD(events, i), buffer, BUFF);
        }
        printf("recived %s\n", buffer);
    }
    goto try;
}
