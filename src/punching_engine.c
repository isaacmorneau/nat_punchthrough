#include <pthread.h>
#include <limits.h>
#include "punching_engine.h"
#include "wrapper.h"

#define BUFF 1024
void * start_sending(const punching_info * restrict info) {
    int efd = make_epoll();
    struct epoll_event * events = make_epoll_events();
    struct sockaddr;
    char buffer[BUFF];
    struct sockaddr_storage * addresses = calloc(info->range, sizeof(struct sockaddr_storage));
    for (int i = 0; i < info->range; ++i) {
        make_storage(&addresses[i], info->host, info->port + i);
    }
sending:
    for (int i = 0; i < info->range; ++i) {
        //send_message(EVENT_FD(events, i), buffer, BUFF, );
    }
    goto sending;
    free(events);
    return 0;
}
//create all ports in the range and listen on them
void * start_listening(const punching_info * restrict info) {
    int efd = make_epoll();
    struct epoll_event * events = make_epoll_events();
    char buffer[BUFF];

    int sock;
    for (int i = 0; i < info->range; ++i) {
        sock = make_bound_udp(info->port + i);
        add_epoll_fd(efd, sock);
    }

    int n;
listening:
    n = wait_epoll(efd, events);
    for (int i = 0; i < n; ++i) {
        if (EVENT_IN(events,i)) {
            read_message(EVENT_FD(events, i), buffer, BUFF);
        }
    }
    goto listening;
    free(events);
    return 0;
}

void start_punch(const punching_info * restrict info) {

}
