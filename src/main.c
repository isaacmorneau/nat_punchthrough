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

#define SOCKOPTS "p:r:s:d:h"
#define PORT 44444
#define RANGE 20

static inline void print_help() {
    puts("usage options:\n"
            "\t [p]ort <1-65535>        - the port to try and connect on [default 44444]\n"
            "\t [r]ange <1-65535>       - the range either side to try [default 20]\n"
            "\t [s]address <url || ip>   - the source address\n"
            "\t [d]address <url || ip>   - the destination address\n"
            "\t [h]elp                  - this message\n"
          );
}
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};
unsigned short csum(unsigned short *ptr,int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while (nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if (nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return answer;
}

void prime(const char * restrict src_address, const char * restrict dest_address, const int port) {
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

    if (s == -1) {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket, ensure run as root");
        exit(1);
    }

    //Datagram to represent the packet
    char datagram[48], * pseudogram;

    //zero out the packet buffer
    memset(datagram, 0, 48);

    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;

    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
    struct sockaddr_in sin;
    struct pseudo_header psh;

    //some address resolution
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(dest_address);

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->id = htonl(54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 1;//just make the outbound dont actually get there
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;

    iph->saddr = inet_addr(src_address);
    iph->daddr = sin.sin_addr.s_addr;

    //Ip checksum
    iph->check = csum ((unsigned short *) datagram, iph->tot_len);

    //TCP Header
    tcph->source = htons (port);
    tcph->dest = htons(port);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;  //tcp header size
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons(5840);
    tcph->check = 0; //leave checksum 0 now, filled later by pseudo header
    tcph->urg_ptr = 0;

    //Now the TCP checksum
    psh.source_address = inet_addr(src_address);
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr));

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
    pseudogram = malloc(psize);

    memcpy(pseudogram, (char*) &psh , sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

    tcph->check = csum((unsigned short*) pseudogram , psize);

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;

    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }

    for (int i = 0; i < 5; ++i) {
        //Send the packet
        if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0){
            perror("sendto failed");
        }
        //Data send successfully
        else {
            printf("Packet Send. Length : %d \n" , iph->tot_len);
        }
    }
    close(s);
}
void punch(const char * restrict address, const int port, const int range) {
    //connect at above and below the expected port to see if we can break through
}

int main(int argc, char ** argv) {
    char * src_addr = 0, * dest_addr = 0;
    int range = RANGE, port = PORT;

    //handle the arguments in its own scope
    {
        int c;
        for (;;) {
            int option_index = 0;

            static struct option long_options[] = {
                {"port",     required_argument, 0, 'p'},
                {"range",    required_argument, 0, 'r'},
                {"saddress", required_argument, 0, 's'},
                {"daddress", required_argument, 0, 'd'},
                {"help",     no_argument,       0, 'h'},
                {0,          0,                 0, 0}
            };

            c = getopt_long(argc, argv, SOCKOPTS, long_options, &option_index);
            if (c == -1) {
                break;
            }

            switch (c) {
                case 'p':
                    port = atoi(optarg);
                    break;
                case 'r':
                    range = atoi(optarg);
                    break;
                case 's':
                    src_addr = optarg;
                    break;
                case 'd':
                    dest_addr = optarg;
                    break;
                case 'h':
                case '?':
                default:
                    print_help();
                    return 0;
            }
        }
        if (!port || !src_addr || !dest_addr) {
            print_help();
            return 1;
        }
    }

    puts("press ENTER to start\n");
    (void)getchar();
    puts("priming\n");
    prime(src_addr, dest_addr, port);
    puts("punching\n");
    punch(dest_addr, port, range);
    return 0;
}
