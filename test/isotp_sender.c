#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/isotp.h>

int main(void)
{
    int sock;
    struct sockaddr_can addr = {0};
    struct ifreq ifr = {0};

    const unsigned char payload[] =
        "ISO-TP baseline diagnostic message";

    /* Create ISO-TP socket */
    sock = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);

    if (sock < 0) {
        perror("ISO-TP socket");
        return 1;
    }

    /* Find vcan0 */
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "vcan0");

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(sock);
        return 1;
    }

    /* Configure addressing */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.tx_id = 0x7E0;
    addr.can_addr.tp.rx_id = 0x7E8;

    /* Bind socket */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("ISO-TP bind");
        close(sock);
        return 1;
    }

    printf("[TX] Sending legitimate ISO-TP message...\n");

    ssize_t n = write(sock, payload, sizeof(payload) - 1);

    if (n < 0) {
        perror("ISO-TP write");
        close(sock);
        return 1;
    }

    printf("[TX] Submitted %zd-byte ISO-TP payload\n", n);

    close(sock);
    return 0;
}