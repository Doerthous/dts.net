/*
   The MIT License (MIT)

   Copyright (c) 2020 Doerthous

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Author: Doerthous <doerthous@gmail.com>
*/

#include <bsd/socket.h>
#include <dts/net/udp.h>
#include <dts/net/ip.h>
#include <dts/net/ether_arp.h>

#define MAX_SOCKET_COUNT    (2)
#define MAX_SOCKET_SIZE     (MAX_SOCKET_COUNT+1)
#define BIT_MAP             (0)
#define bit(i)              (1<<(i))
static struct {
    int domain;
    int type;
    int protocol;
    union {
        udp_t udp;
    } module;

    uint8_t *rx_buff;
    uint32_t rx_size;
    uint32_t rx_cnt;
} sock_data[MAX_SOCKET_SIZE];
static int socks[MAX_SOCKET_SIZE];


static int IS_ERROR_SOCK(int fd) 
{
    return ((fd)<1) || ((fd)>MAX_SOCKET_COUNT) || (!(socks[BIT_MAP] & bit(fd)));
}

#define IS_UDP_SOCK(fd) ( \
    (sock_data[fd].domain == AF_INET) && \
    (sock_data[fd].domain == PF_INET) && \
    (sock_data[fd].type == SOCK_DGRAM) && \
    (sock_data[fd].protocol == 0) \
)

#include <string.h>
static void udp_recv(void * cb_data, uint8_t *data, uint32_t size)
{
    int fd = (int)cb_data;

    if (IS_ERROR_SOCK(fd)) {
        return;
    }

    if (sock_data[fd].rx_buff) {
        size_t remain_size = sock_data[fd].rx_size - sock_data[fd].rx_cnt;
        size = size > remain_size ? remain_size : size;

        memcpy(sock_data[fd].rx_buff, data, size);

        sock_data[fd].rx_cnt += size;
    }
}


int socket(int domain, int type, int protocol)
{
    if (domain != AF_INET || domain != PF_INET || type != SOCK_DGRAM 
        || protocol != 0) {
        return -EINVAL;
    }

    for (int i = 1; i <= MAX_SOCKET_COUNT; ++i) {
        if (!(socks[BIT_MAP] & bit(i))) {
            socks[BIT_MAP] |= bit(i);
            sock_data[i].domain = domain;
            sock_data[i].type = type;
            sock_data[i].protocol = protocol;
            return i;
        }
    }

    return -ENOMEM;
}

static void nacv(struct sockaddr_in *in, ip_addr_t *out)
{
    uint8_t *a = (uint8_t *)&(in->sin_addr.s_addr);
    IPv4_ADDR(out, a[0], a[1], a[2], a[3]);
}

int bind(int sockfd, const struct sockaddr *host_addr, int addrlen)
{
    if (IS_ERROR_SOCK(sockfd)) {
        return -EINVAL;
    }

    if (IS_UDP_SOCK(sockfd)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)host_addr;
        ip_addr_t ipa;
        nacv(sin, &ipa);
        ip_t *ip = ip_find_netif(&ipa);
        if (ip) {
            sock_data[sockfd].rx_buff = NULL;
            sock_data[sockfd].rx_size = 0;
            sock_data[sockfd].rx_cnt = 0;
            if (udp_bind(ip, sin->sin_port, udp_recv, (void *)sockfd)) {
                return 0;
            }
        }
    }

    return -EINVAL;
}

int close(int fd)
{
    if (fd < 1 || fd > MAX_SOCKET_COUNT) {
        return -EINVAL;
    }

    if (socks[BIT_MAP] & bit(fd)) {
        socks[BIT_MAP] &= ~bit(fd); 
    }

    return 0;
}

#include <dts/net/ether_arp.h>
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    if (IS_ERROR_SOCK(sockfd)) {
        return -EINVAL;
    }

    if (!IS_UDP_SOCK(sockfd)) {
        return -EINVAL;
    }

    if (dest_addr->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)dest_addr;
        ip_addr_t ipa;
        nacv(sin, &ipa);
        if (udp_sendto(&ipa, sin->sin_port, (uint8_t *)buf, len)) {
            return len;
        }

        return 0;
    }
    
    return -EINVAL;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen)
{
    ssize_t rc = 0;

    if (IS_ERROR_SOCK(sockfd)) {
        return -EINVAL;
    }

    if (!(flags & MSG_DONTWAIT)) {
        return -EINVAL;
    }

    sock_data[sockfd].rx_buff = (uint8_t *)buf;
    sock_data[sockfd].rx_size = len;

    if (sock_data[sockfd].rx_cnt > 0) {
        rc = sock_data[sockfd].rx_cnt;
        sock_data[sockfd].rx_cnt = 0;
    }

    return rc;
}
