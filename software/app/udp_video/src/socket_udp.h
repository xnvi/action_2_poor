// #include <inttypes.h>
// #include <stdarg.h>
// #include <stdbool.h>
#include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

/**
 * @brief Setup UDP connection with server
 *
 * @host    server address
 * @port    server port
 * @return  UPD socket handle (value>0) when success, or 0 otherwise
 */
uintptr_t socket_udp_open(const char *host, unsigned short port);

/**
 * @brief Disconnect with server and release resource
 *
 * @param fd UDP Socket handle
 * @return  0 when success
 */
void socket_udp_close(uintptr_t fd);

/**
 * @brief Write data via UDP connection
 *
 * @param fd            UDP socket handle
 * @param data          source data to write
 * @param len           length of data
 * @return              length of data written when success, or err code for
 * failure
 */
int socket_udp_write(uintptr_t fd, const unsigned char *data, unsigned int len);

/**
 * @brief Read data via UDP connection
 *
 * @param fd            UDP socket handle
 * @param data          destination data buffer where to put data
 * @param len           length of data
 * @return              length of data read when success, or err code for
 * failure
 */
int socket_udp_read(uintptr_t fd, unsigned char *data, unsigned int len);

/**
 * @brief Read data via UDP connection
 *
 * @param fd            UDP socket handle
 * @param data          destination data buffer where to put data
 * @param len           length of data
 * @param timeout_ms    timeout value in millisecond
 * @return              length of data read when success, or err code for
 * failure
 */
int socket_udp_read_timeout(uintptr_t fd, unsigned char *p_data, unsigned int datalen,
                        unsigned int timeout_ms);