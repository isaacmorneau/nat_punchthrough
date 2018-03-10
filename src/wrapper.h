#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdlib.h>
#include <netdb.h>

//==>assert macros<==
#ifdef NDEBUG
#define ensure(expr) ((void)(expr))
#define ensure_nonblock(expr) ((void)(expr))
#else
#define ensure(expr)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "%s::%s::%d\n\t", __FILE__, __FUNCTION__, __LINE__);\
            perror(#expr);\
            exit(1);\
        }\
    } while(0)

#define ensure_nonblock(expr)\
    do {\
        if (!(expr) && errno != EAGAIN) {\
            fprintf(stderr, "%s::%s::%d\n\t", __FILE__, __FUNCTION__, __LINE__);\
            perror(#expr);\
            exit(1);\
        }\
    } while(0)
#endif

//==>fd ioctl wrappers<==
void set_fd_limit();
void set_non_blocking(int sfd);

//==>tcp wrappers<==
int make_bound(const char * port);
int make_connected(const char * address, const char * port);

//==>udp wrappers<==
int make_udp(int port);
int read_message(int sockfd, char * buffer, int len);
int send_message(int sockfd, const char * buffer, int len, const struct sockaddr * addr, socklen_t addr_len);


//==>epoll wrappers<==
#define MAXEVENTS 256
int make_epoll();
int wait_epoll(int efd, struct epoll_event * events);
int add_epoll_ptr(int efd, int ifd, void * ptr);
int add_epoll_fd(int efd, int ifd);


//==>connection forwarding wrappers<==
struct directional_buffer;
typedef struct directional_buffer {
    int sockfd;
    int pipefd[2];
    struct directional_buffer * paired;
} directional_buffer;

int forward_read(const directional_buffer * con);
int forward_flush(const directional_buffer * con);

void close_directional_buffer(directional_buffer * con);
void init_directional_buffer(directional_buffer * in_con, directional_buffer * out_con, int in_fd, int out_fd);

#endif
