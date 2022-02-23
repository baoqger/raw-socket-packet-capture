#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <net/if.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int sock, n;
    char buffer[2048];
    unsigned char *iphead, *ethhead;

    if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
        perror("socket");
        exit(1);
    }
    // bind to eth0 interface only
    const char *opt;
    opt = "eth0";
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt)) < 0) {
        perror("setsockopt");
        close(sock);
        exit(1);
    }
    // An ioctl() request has encoded in it whether the argument is an in parameter or out parameter, 
    // and the size of the argument argp in bytes.
    /* set the network card in promiscuos mode*/
    
    struct ifreq ethreq;
    strncpy(ethreq.ifr_name, "eth0", IF_NAMESIZE);
    if (ioctl(sock, SIOCGIFFLAGS, &ethreq) == -1) {
        perror("ioctl");
        close(sock);
        exit(1);
    }
    ethreq.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock, SIOCGIFFLAGS, &ethreq) == -1) {
        perror("ioctl");
        close(sock);
        exit(1);
    }

    // attach the filter to the socket
    // struct sock_filter BPF_code[] = {
    //     { 0x28, 0, 0, 0x0000000c },
    //     { 0x15, 0, 12, 0x00000800 },
    //     { 0x30, 0, 0, 0x00000017 },
    //     { 0x15, 0, 10, 0x00000011 },
    //     { 0x20, 0, 0, 0x0000001a },
    //     { 0x15, 2, 0, 0xc0a8090a },
    //     { 0x20, 0, 0, 0x0000001e },
    //     { 0x15, 0, 6, 0xc0a8090a },
    //     { 0x28, 0, 0, 0x00000014 },
    //     { 0x45, 4, 0, 0x00001fff },
    //     { 0xb1, 0, 0, 0x0000000e },
    //     { 0x48, 0, 0, 0x0000000e },
    //     { 0x15, 0, 1, 0x00001388 },
    //     { 0x6, 0, 0, 0x00000044 },
    //     { 0x6, 0, 0, 0x00000000 }
    // };    
    // struct sock_fprog Filter;
    // Filter.len = 15;
    // Filter.filter = BPF_code;


    // if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter)) < 0) {
    //     perror("setsockopt");
    //     close(sock);
    //     exit(1);
    // } 

    while(1) {
        printf("-----------\n");
        n = recvfrom(sock, buffer, 2048, 0, NULL, NULL);
        printf("%d bytes read\n", n);

        /* Check to see if the packet contains at least
        * complete Ethernet (14), IP (20) and TCP/UDP
        * (8) headers.
        */
        if (n < 42) {
            perror("recvfrom():");
            printf("Incomplete packet (errno is %d)\n", errno);
            close(sock);
            exit(0);
        }

        ethhead = buffer;
        printf("Source MAC address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
            ethhead[0], ethhead[1], ethhead[2], ethhead[3], ethhead[4], ethhead[5]
        );
        printf("Destination MAC address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
            ethhead[6], ethhead[7], ethhead[8], ethhead[9], ethhead[10], ethhead[11]
        );

        iphead = buffer + 14; 

        if (*iphead==0x45) { /* Double check for IPv4
                            * and no options present */
            printf("Source host %d.%d.%d.%d\n",
                    iphead[12],iphead[13],
                    iphead[14],iphead[15]);
            printf("Dest host %d.%d.%d.%d\n",
                    iphead[16],iphead[17],
                    iphead[18],iphead[19]);
            printf("Source,Dest ports %d,%d\n",
                    (iphead[20]<<8)+iphead[21],
                    (iphead[22]<<8)+iphead[23]);
            printf("Layer-4 protocol %d\n",iphead[9]);
        }

    }
}


