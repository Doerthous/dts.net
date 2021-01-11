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

   Authour: Doerthous <doerthous@gmail.com>
*/

#ifndef DTS_NET_ETHER_ARP_H_
#define DTS_NET_ETHER_ARP_H_

#include <dts_net_ether.h>
#include <dts_net_arp.h>
#include <dts_net_ip.h>

#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IP   0x0800

#define ARP_HADDR_ETHERNET 1

typedef struct 
{
    void *next; // list
    
    dts_net_ip_t *ip;
    dts_net_ether_t *eth;
} dts_net_nif_t;

typedef struct 
{
    void *next; // list
    
    uint8_t ip[4];
    uint8_t mac[6];
    uint16_t lease;
} dts_net_ether_arp_ti_t; // arp table item

#include <string.h>
static inline uint32_t 
dts_net_ether_arp_ask_ip
(
    dts_net_arp_packet_t *arp_pkt,
    uint8_t *source_mac,
    uint8_t *source_ip,
    uint8_t *target_ip
)
{
    arp_pkt->hrd = ARP_HADDR_ETHERNET;
    arp_pkt->pro = ETHER_TYPE_IP;
    arp_pkt->hln = 6;
    arp_pkt->pln = 4;
    arp_pkt->op = ARP_OP_REQ;
    arp_pkt->sha = source_mac;
    arp_pkt->spa = source_ip;
    arp_pkt->tha = arp_pkt->data+18;
    arp_pkt->tpa = target_ip;
    // For an unknown target, target hardware address fill will 
    // 00:00:00:00:00:00
    memset(arp_pkt->data, 0, 28); 
    return dts_net_arp_pack(arp_pkt);
}


uint8_t *
dts_net_ether_arp_get_mac_address
(
    dts_net_ether_t *ether,
    uint8_t *target_ip
);

void 
dts_net_ether_arp_ether_recv
(
    dts_net_ether_t *ether, 
    dts_net_arp_packet_t *pkt
);

void 
dts_net_ether_arp_resgiter_pair
(
    dts_net_ip_t *ip, 
    dts_net_ether_t *eth
);

dts_net_ip_t *
dts_net_ether_arp_get_ip
(
    dts_net_ether_t *eth
);

dts_net_ip_t *
dts_net_ether_arp_get_ip_by_addr
(
    dts_net_ip_addr_t *addr
);


#endif // DTS_NET_ETHER_ARP_H_
