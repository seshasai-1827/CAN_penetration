#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

static int open_can_socket(void)
{
    int sock;
    struct ifreq ifr = {0};
    struct sockaddr_can addr = {0};

    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (sock < 0) {
        perror("CAN socket");
        return -1;
    }

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "vcan0");

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(sock);
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("CAN bind");
        close(sock);
        return -1;
    }

    return sock;
}

static int send_frame(int sock, struct can_frame *frame)
{
    ssize_t n = write(sock, frame, sizeof(*frame));

    if (n != sizeof(*frame)) {
        perror("CAN write");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int sock = open_can_socket();

    if (sock < 0)
        return EXIT_FAILURE;

    struct can_frame frame = {0};

    if (argc != 2 ||
        (strcmp(argv[1], "orphan") != 0 &&
         strcmp(argv[1], "sequence") != 0)) {

        fprintf(stderr,
                "Usage: %s orphan|sequence\n", argv[0]);

        close(sock);
        return EXIT_FAILURE;
    }

    frame.can_id = 0x7E0;
    frame.len = 8;

    if (strcmp(argv[1], "orphan") == 0) {

        /* Test A: Consecutive Frame without a First Frame */
        frame.data[0] = 0x21;
        frame.data[1] = 0x11;
        frame.data[2] = 0x22;
        frame.data[3] = 0x33;
        frame.data[4] = 0x44;
        frame.data[5] = 0x55;
        frame.data[6] = 0x66;
        frame.data[7] = 0x77;

        printf("[TEST A] Sending isolated Consecutive Frame\n");

        if (send_frame(sock, &frame) < 0) {
            close(sock);
            return EXIT_FAILURE;
        }

    } else {

        /*
         * Test B: First Frame declares a 20-byte message.
         * The first Consecutive Frame should use sequence 1.
         */

        frame.data[0] = 0x10;
        frame.data[1] = 0x14;
        frame.data[2] = 0x01;
        frame.data[3] = 0x02;
        frame.data[4] = 0x03;
        frame.data[5] = 0x04;
        frame.data[6] = 0x05;
        frame.data[7] = 0x06;

        printf("[TEST B] Sending First Frame\n");

        if (send_frame(sock, &frame) < 0) {
            close(sock);
            return EXIT_FAILURE;
        }

        /*
         * Allow the receiver to process the First Frame.
         * This is a controlled test, not a timing guarantee.
         */
        usleep(100000);

        /* Send sequence 2 instead of sequence 1 */
        memset(&frame, 0, sizeof(frame));

        frame.can_id = 0x7E0;
        frame.len = 8;

        frame.data[0] = 0x22;
        frame.data[1] = 0x10;
        frame.data[2] = 0x20;
        frame.data[3] = 0x30;
        frame.data[4] = 0x40;
        frame.data[5] = 0x50;
        frame.data[6] = 0x60;
        frame.data[7] = 0x70;

        printf("[TEST B] Sending unexpected sequence number\n");

        if (send_frame(sock, &frame) < 0) {
            close(sock);
            return EXIT_FAILURE;
        }
    }

    printf("[TEST] Frames submitted to vcan0\n");

    close(sock);
    return EXIT_SUCCESS;
}