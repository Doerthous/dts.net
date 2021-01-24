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
#include <dts/net/tcp.h>
#include <dts/net/ip.h>
#include <dts/net/ether_arp.h>
#include <dts/net/mem.h>
#include <arpa/inet.h>

typedef struct scoket {
    void *next;

    int domain;
    int type;
    int protocol;
    union {
        udp_t udp;
        tcp_t *tcp;
    } module;

    // init data
    ip_t *ip;
    int port;

    uint8_t *rx_buff;
    uint32_t rx_size;
    uint32_t rx_cnt;
} socket_t;

#include <dts/datastruct/list.h>

#define MEM_ITEM_LIST_API(type, name, count) \
 static list_t name##_list; \
 static type name##s[count]; \
 type *mem_alloc_##name(void) { \
     return list_dequeue(&name##_list); \
 } \
 void mem_free_##name(type *name) { \
     list_enqueue(&name##_list, name); \
 }
#define MEM_ITEM_LIST_API_INIT(name, count) \
 for (int i = 0; i < count; ++i) { \
     list_enqueue(&name##_list, &name##s[i]); \
 }

static int socket_api_inited;
#define DTS_NET_SOCKET_MAX_COUNT 10
MEM_ITEM_LIST_API(socket_t, socket, DTS_NET_SOCKET_MAX_COUNT);


#define to_socket(fd) ((socket_t *)fd)

#define IS_UDP_SOCK(fd) ( \
    (to_socket(fd)->domain == AF_INET) && \
    (to_socket(fd)->domain == PF_INET) && \
    (to_socket(fd)->type == SOCK_DGRAM) && \
    (to_socket(fd)->protocol == 0) \
)

#define IS_TCP_SOCK(fd) ( \
    (to_socket(fd)->domain == AF_INET) && \
    (to_socket(fd)->domain == PF_INET) && \
    (to_socket(fd)->type == SOCK_STREAM) && \
    (to_socket(fd)->protocol == 0) \
)

#include <string.h>
static void udp_recv(void * cb_data, uint8_t *data, uint32_t size)
{
    int fd = (int)cb_data;

    if (to_socket(fd)->rx_buff) {
        size_t remain_size = to_socket(fd)->rx_size - to_socket(fd)->rx_cnt;
        size = size > remain_size ? remain_size : size;

        memcpy(to_socket(fd)->rx_buff, data, size);

        to_socket(fd)->rx_cnt += size;
    }
}


int socket(int domain, int type, int protocol)
{
    socket_t *sk;

    if (!socket_api_inited) {
        socket_api_inited = 1;
        MEM_ITEM_LIST_API_INIT(socket, DTS_NET_SOCKET_MAX_COUNT);
    }

    if ((domain == AF_INET || domain == PF_INET) && protocol == 0) {
        if (type == SOCK_DGRAM || type == SOCK_STREAM) {
            sk = mem_alloc_socket(); // will sk < 0?
            if (sk) {
                sk->domain = domain;
                sk->type = type;
                sk->protocol = protocol;
                return (int)sk;
            }
            return -ENOMEM;
        }
    }

    return -EINVAL;
}

static void nacv(struct sockaddr_in *in, ip_addr_t *out)
{
    uint8_t *a = (uint8_t *)&(in->sin_addr.s_addr);
    IPv4_ADDR(out, a[3], a[2], a[1], a[0]);
}

int bind(int fd, const struct sockaddr *host_addr, int addrlen)
{
    socket_t *sk = to_socket(fd);
    struct sockaddr_in *sin = (struct sockaddr_in *)host_addr;
    ip_addr_t ipa;
    nacv(sin, &ipa);
    ip_t *ip = ip_find_netif(&ipa);

    if (IS_UDP_SOCK(fd)) {
        if (ip) {
            sk->rx_buff = NULL;
            sk->rx_size = 0;
            sk->rx_cnt = 0;
            if (udp_bind(ip, ntohs(sin->sin_port), udp_recv, (void *)fd)) {
                return 0;
            }
        }
    }

    if (IS_TCP_SOCK(fd)) {
        if (ip) {
            sk->ip = ip;
            sk->port = ntohs(sin->sin_port);
            return 0;
        }
    }

    return -EINVAL;
}

int listen(int fd, int backlog)
{
    socket_t *sk = to_socket(fd);
    if (IS_TCP_SOCK(fd)) {
        sk->module.tcp = tcp_open(&sk->ip->addr, sk->port, NULL, 0);
        if (sk->module.tcp) {
            return 0;
        }
    }

    return -1;
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    socket_t *sk = to_socket(fd);
    if (IS_TCP_SOCK(fd)) {
        if (tcp_status(sk->module.tcp) == DTS_NET_TCP_STATE_ESTABLISHED) {

            int c = socket(sk->domain, sk->type, sk->protocol);
            if (c > 0) {
                to_socket(c)->module.tcp = sk->module.tcp;
                sk->module.tcp = tcp_open(&sk->ip->addr, sk->port, NULL, 0);
            }
            return c;
        }
    }

    return -1;
}

int close(int fd)
{
    socket_t *sk = to_socket(fd);
    if (IS_TCP_SOCK(fd)) {
        if (sk->module.tcp) {
            sk->module.tcp->time_wait_seconds=1;
            tcp_close(sk->module.tcp);
        }
    }

    mem_free_socket(to_socket(fd));

    return 0;
}

#include <dts/net/ether_arp.h>
ssize_t sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    socket_t *sk = to_socket(fd);

    if (IS_UDP_SOCK(fd)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)dest_addr;
        ip_addr_t ipa;
        nacv(sin, &ipa);
        if (udp_sendto(&ipa, ntohs(sin->sin_port), (uint8_t *)buf, len)) {
            return len;
        }
        return 0;
    }

    if (IS_TCP_SOCK(fd)) {
        if (sk->module.tcp) {
            if (tcp_status(sk->module.tcp) == DTS_NET_TCP_STATE_ESTABLISHED) {
                return tcp_send(sk->module.tcp, (uint8_t *)buf, len);
            }
            return 0;
        }
    }
    
    return -EINVAL;
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen)
{
    socket_t *sk = to_socket(fd);
    ssize_t rc = 0;

    if (IS_UDP_SOCK(fd)) {
        if (!(flags & MSG_DONTWAIT)) {
            return -EINVAL;
        }
        sk->rx_buff = (uint8_t *)buf;
        sk->rx_size = len;

        if (sk->rx_cnt > 0) {
            rc = sk->rx_cnt;
            sk->rx_cnt = 0;
        }
    }

    if (IS_TCP_SOCK(fd)) {
        if (tcp_status(sk->module.tcp) == DTS_NET_TCP_STATE_ESTABLISHED) {
            rc = tcp_recv(sk->module.tcp, (uint8_t*)buf, len);
            return rc;
        }
        if (tcp_status(sk->module.tcp) > DTS_NET_TCP_STATE_ESTABLISHED) {
            return -1;
        }
    }

    return rc;
}

#include <stdlib.h>
int connect(int fd, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    socket_t *sk = to_socket(fd);

    if (IS_TCP_SOCK(fd)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)dest_addr;
        ip_addr_t dest;
        nacv(sin, &dest);
        ip_t *ip = ip_find_netif(&dest);
        if (ip) {
            for (int i = 0; i < 3; ++i) {
                sk->module.tcp = tcp_open(&ip->addr, (rand()&0xFBFF)+1024, 
                    &dest, ntohs(sin->sin_port));
                if (sk->module.tcp) {
                    return 0;
                }
            }
        }
    }

    return -1;
}

ssize_t send(int fd, const void *buf, size_t len, int flags)
{
    return sendto(fd, buf, len, flags, NULL, 0);
}

ssize_t recv(int fd, void *buf, size_t len, int flags)
{
    return recvfrom(fd, buf, len, flags, NULL, 0);
}
