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

#ifndef DTS_NET_IP_H_
#define DTS_NET_IP_H_

#include <stdint.h>
#include <dts_net_dblk.h>

#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17

#define IP_MKFLAGS(hdr, DF, MF) (hdr)->flags = (((DF)<<1)|(MF))

#define IP_TOS_P_NETWORK_CONTROL        0b111
#define IP_TOS_P_INTERNETWORK_CONTROL   0b110
#define IP_TOS_P_CRITIC_ECP             0b101
#define IP_TOS_P_FLASH_OVERRIDE         0b100
#define IP_TOS_P_FLASH                  0b011
#define IP_TOS_P_IMMEDIATE              0b010
#define IP_TOS_P_PRIORITY               0b001
#define IP_TOS_P_ROUTINE                0b000
#define IP_TOS_D_NORMAL_DELAY           (0)
#define IP_TOS_D_LOW_DELAY              (1<<3)
#define IP_TOS_T_NORMAL_THROUGHPUT      (0)
#define IP_TOS_T_LOW_THROUGHPUT         (1<<4)
#define IP_TOS_R_NORMAL_RELIBILITY      (0)
#define IP_TOS_R_LOW_RELIBILITY         (1<<5)
#define IP_MKTOS(hdr, P, D, T, R) do \
{ \
    (hdr)->type_of_service = 0; \
    (hdr)->type_of_service = (P|D|T|R); \
} while (0)

typedef struct
{
    uint8_t version:4;
    uint8_t ihl:4;
    uint8_t type_of_service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags:3;
    uint16_t fragment_offset:13;
    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t *source_address;
    uint8_t *destination_address;
    uint8_t *options;
    uint8_t option_size;
    // Padding
} dts_net_ip_header_t;

typedef struct
{
    dts_net_ip_header_t header;
    dts_net_dblk_t *payload;
    dts_net_dblk_t *raw_data;
} dts_net_ip_datagram_t;


typedef struct
{
    uint8_t version;
    union {
        uint8_t v4[4];
        uint8_t v6[1];
    } addr;
    union {
        uint8_t v4[4];
        uint8_t v6[1];
    } netmask;
} dts_net_ip_addr_t;

#define DTS_NET_IPv4_ADDR_DEF(name, a1,a2,a3,a4) \
dts_net_ip_addr_t name = { .version = 4, .addr.v4 = {a1,a2,a3,a4}, };

#define DTS_NET_IPv4_ADDR(a,a1,a2,a3,a4) do \
{ \
    (a)->version = 4; \
    (a)->addr.v4[0] = a1; \
    (a)->addr.v4[1] = a2; \
    (a)->addr.v4[2] = a3; \
    (a)->addr.v4[3] = a4; \
} while (0)
#define DTS_NET_IPv4_NETMASK(a,a1,a2,a3,a4) do \
{ \
    (a)->netmask.v4[0] = a1; \
    (a)->netmask.v4[1] = a2; \
    (a)->netmask.v4[2] = a3; \
    (a)->netmask.v4[3] = a4; \
} while (0)

/**
 * @brief IP address compare (don't care netmask)
 * 
 * @param a1 [in] address1
 * @param a2 [in] address2
 * @return int !0 if equals else 0
 */
int dts_net_ip_addr_equals(dts_net_ip_addr_t *a1, dts_net_ip_addr_t *a2);
int dts_net_ip_addr_same_lan(dts_net_ip_addr_t *a1, dts_net_ip_addr_t *a2);


typedef struct
{
    int (*ll_send)(void *, dts_net_ip_datagram_t *);
    void *ll; // low level
    dts_net_ip_addr_t addr;
} dts_net_ip_t;

#define DTS_NET_IP_IPv4_ADDR(a1,a2,a3,a4) \
    .addr.version = 4, \
    .addr.addr.v4[0] = a1, \
    .addr.addr.v4[1] = a2, \
    .addr.addr.v4[2] = a3, \
    .addr.addr.v4[3] = a4

#define DTS_NET_IP_IPv4_MASK(a1,a2,a3,a4) \
    .addr.netmask.v4[0] = a1, \
    .addr.netmask.v4[1] = a2, \
    .addr.netmask.v4[2] = a3, \
    .addr.netmask.v4[3] = a4

int 
dts_net_ip_hl_send
(
    dts_net_ip_t *ip,
    dts_net_ip_datagram_t *datagram
);
void 
dts_net_ip_ll_recv
(
    dts_net_ip_t *ip,
    dts_net_ip_datagram_t *datagram
);

/**
 * @brief Find ip instance by a LAN address
 *          example: 
 *          There is a ip instance whose address is 10.0.0.1 and net mask is
 *          255.255.255.0. The **addr** param which has addr 10.0.0.x will match
 *          the ip instance whose address is 10.0.0.1.
 * @param addr [in] address of device in the same LAN.
 * @return dts_net_ip_t* [out] ip instance or NULL.
 */
dts_net_ip_t *dts_net_ip_find_netif(dts_net_ip_addr_t *addr);

#endif // DTS_NET_IP_H_
