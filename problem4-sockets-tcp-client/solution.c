#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int send_all(int socket_fd, const unsigned char *data, size_t size) {
    size_t sent = 0;

    while (sent < size) {
        ssize_t result = send(
            socket_fd,
            data + sent,
            size - sent,
            0
        );

        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (result == 0) {
            return 0;
        }

        sent += (size_t) result;
    }

    return 1;
}

static int receive_all(
    int socket_fd,
    unsigned char *data,
    size_t size
) {
    size_t received = 0;

    while (received < size) {
        ssize_t result = recv(
            socket_fd,
            data + received,
            size - received,
            0
        );

        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (result == 0) {
            return 0;
        }

        received += (size_t) result;
    }

    return 1;
}

static void encode_little_endian(
    int32_t number,
    unsigned char bytes[4]
) {
    uint32_t value = (uint32_t) number;

    bytes[0] = (unsigned char) (value & 0xFFu);
    bytes[1] = (unsigned char) ((value >> 8) & 0xFFu);
    bytes[2] = (unsigned char) ((value >> 16) & 0xFFu);
    bytes[3] = (unsigned char) ((value >> 24) & 0xFFu);
}

static int32_t decode_little_endian(
    const unsigned char bytes[4]
) {
    uint32_t value =
        ((uint32_t) bytes[0]) |
        ((uint32_t) bytes[1] << 8) |
        ((uint32_t) bytes[2] << 16) |
        ((uint32_t) bytes[3] << 24);

    int32_t result;
    memcpy(&result, &value, sizeof(result));

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(
            stderr,
            "Usage: %s <ipv4_addr> <port>\n",
            argv[0]
        );
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    char *port_end = NULL;
    long port = strtol(argv[2], &port_end, 10);

    if (
        port_end == argv[2] ||
        *port_end != '\0' ||
        port < 1 ||
        port > 65535
    ) {
        fprintf(stderr, "Invalid port\n");
        return 1;
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((uint16_t) port);

    int address_result = inet_pton(
        AF_INET,
        argv[1],
        &server_address.sin_addr
    );

    if (address_result != 1) {
        fprintf(stderr, "Invalid IPv4 address\n");
        close(socket_fd);
        return 1;
    }

    if (
        connect(
            socket_fd,
            (struct sockaddr *) &server_address,
            sizeof(server_address)
        ) == -1
    ) {
        perror("connect");
        close(socket_fd);
        return 1;
    }

    int32_t number;

    while (scanf("%" SCNd32, &number) == 1) {
        unsigned char outgoing[4];
        encode_little_endian(number, outgoing);

        int send_result = send_all(
            socket_fd,
            outgoing,
            sizeof(outgoing)
        );

        if (send_result == 0) {
            close(socket_fd);
            return 0;
        }

        if (send_result == -1) {
            if (errno == EPIPE || errno == ECONNRESET) {
                close(socket_fd);
                return 0;
            }

            perror("send");
            close(socket_fd);
            return 1;
        }

        unsigned char incoming[4];

        int receive_result = receive_all(
            socket_fd,
            incoming,
            sizeof(incoming)
        );

        if (receive_result == 0) {
            close(socket_fd);
            return 0;
        }

        if (receive_result == -1) {
            if (errno == ECONNRESET) {
                close(socket_fd);
                return 0;
            }

            perror("recv");
            close(socket_fd);
            return 1;
        }

        int32_t response = decode_little_endian(incoming);

        printf("%" PRId32 "\n", response);
        fflush(stdout);
    }

    close(socket_fd);
    return 0;
}