#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/isotp.h>

int main(void)
{
    int sock;
    struct sockaddr_can addr = {0};
    struct ifreq ifr = {0};
    unsigned char buffer[4096];

    /* Create ISO-TP socket */
    sock = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);

    if (sock < 0) {
        perror("ISO-TP socket");
        return 1;
    }

    /* Find vcan0 interface index */
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "vcan0");

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(sock);
        return 1;
    }

    /* Configure ISO-TP addressing */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = 0x7E0;
    addr.can_addr.tp.tx_id = 0x7E8;

    /* Bind socket */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("ISO-TP bind");
        close(sock);
        return 1;
    }

    /* Set receive timeout */
    struct timeval timeout = {
        .tv_sec = 2,
        .tv_usec = 0
    };

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("SO_RCVTIMEO");
        close(sock);
        return 1;
    }

    printf("ISO-TP receiver listening on vcan0\n");
    printf("RX ID: 0x7E0 | TX ID: 0x7E8\n");

    while (1) {
        ssize_t n = read(sock, buffer, sizeof(buffer));

        if (n < 0) {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[RX] No complete message within timeout\n");
                continue;
            }

            if (errno == EILSEQ) {
                printf("[RX] Sequence-number error\n");
                continue;
            }

            if (errno == EBADMSG) {
                printf("[RX] Invalid message format or padding\n");
                continue;
            }

            perror("ISO-TP read");
            break;
        }

        printf("[RX] Complete ISO-TP message (%zd bytes): ", n);

        for (ssize_t i = 0; i < n; i++)
            printf("%02X ", buffer[i]);

        printf("\n");
    }

    close(sock);
    return 0;
}