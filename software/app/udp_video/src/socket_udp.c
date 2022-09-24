#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
// #include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket_udp.h"

uintptr_t socket_udp_open(const char *host, unsigned short port)
{
#define NETWORK_ADDR_LEN (16)

    int             ret;
    struct addrinfo hints, *addr_list, *cur;
    int             fd = 0;

    if (host == NULL) {
        return 1;
    }

    char port_str[6] = {0};
    snprintf(port_str, 6, "%d", port);

    memset((char *)&hints, 0x00, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family   = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;

    printf("udp connect (host=%s port=%s)\n", host, port_str);

    if (getaddrinfo(host, port_str, &hints, &addr_list) != 0) {
        // printf("getaddrinfo error,errno:%s", STRING_PTR_PRINT_SANITY_CHECK(strerror(errno)));
        printf("getaddrinfo error, errno: %d\n", errno);
        return 0;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            ret = 0;
            continue;
        }

        if (0 == connect(fd, cur->ai_addr, cur->ai_addrlen)) {
            ret = fd;
            break;
        }

        close(fd);
        ret = 0;
    }

    if (ret <= 0) {
        printf("fail to open udp connect\n");
    }

    freeaddrinfo(addr_list);

    return ret;

#undef NETWORK_ADDR_LEN
}

void socket_udp_close(uintptr_t fd)
{
    long socket_id = -1;

    socket_id = (int)fd;
    close(socket_id);
}

int socket_udp_write(uintptr_t fd, const unsigned char *p_data, unsigned int datalen)
{
    int  rc        = -1;
    long socket_id = -1;

    socket_id = (int)fd;
    rc        = send(socket_id, (char *)p_data, (int)datalen, 0);
    if (-1 == rc) {
        return -1;
    }

    return rc;
}

int socket_udp_read(uintptr_t fd, unsigned char *p_data, unsigned int datalen)
{
    long socket_id = -1;
    int  count     = -1;

    socket_id = (int)fd;
    count     = (int)read(socket_id, p_data, datalen);

    return count;
}

int socket_udp_read_timeout(uintptr_t fd, unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms)
{
    int            ret;
    struct timeval tv;
    fd_set         read_fds;
    int            socket_id = -1;

    socket_id = (int)fd;

    if (socket_id < 0) {
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(socket_id, &read_fds);

    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(socket_id + 1, &read_fds, NULL, NULL, timeout_ms == 0 ? NULL : &tv);

    if (ret <= 0) {
        return ret;
    }

    /* Zero fds ready means we timed out */
    // if (ret == 0) {
    //     return QCLOUD_ERR_SSL_READ_TIMEOUT; /* receive timeout */
    // }

    // if (ret < 0) {
    //     if (errno == EINTR) {
    //         return -3; /* want read */
    //     }

    //     return QCLOUD_ERR_SSL_READ; /* receive failed */
    // }

    /* This call will not block */
    return socket_udp_read(fd, p_data, datalen);
}
