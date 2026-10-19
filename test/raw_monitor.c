#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main(void)
{
    int sock;
    struct sockaddr_can addr = {0};
    struct ifreq ifr = {0};
    struct can_frame frame;

    /* Create raw CAN socket */
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (sock < 0) {
        perror("Raw CAN socket");
        return 1;
    }

    /* Find vcan0 */
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "vcan0");

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(sock);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* Bind socket */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Raw CAN bind");
        close(sock);
        return 1;
    }

    printf("Raw CAN monitor listening on vcan0...\n");

    while (1) {
        ssize_t n = read(sock, &frame, sizeof(frame));

        if (n < 0) {
            perror("CAN read");
            break;
        }

        if (n != sizeof(frame)) {
            fprintf(stderr, "Unexpected CAN frame size\n");
            continue;
        }

        printf("[RAW] ID=0x%03X DLC=%u DATA=",
               frame.can_id & CAN_SFF_MASK,
               frame.len);

        for (unsigned int i = 0; i < frame.len; i++)
            printf("%02X ", frame.data[i]);

        printf("\n");
    }

    close(sock);
    return 0;
}