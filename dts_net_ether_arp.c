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

#include <dts/net/ether_arp.h>
#include <dts/net/ip.h>
#include <dts/net/ether.h>
#include <dts/net/arp.h>
#include <dts/net/dblk.h>
#include <dts/net/mem.h>

static int ip_ether_send(ether_t *eth, ip_datagram_t *datagram)
{
    ether_frame_t frame;

    frame.dest_mac_addr = ether_arp_get_mac_address(eth,
		datagram->header.destination_address);
    if (!frame.dest_mac_addr) {
        return 0;
    }
    frame.payload = datagram->raw_data;
    frame.type = ETHER_TYPE_IP;
    frame.data = malloc(2048);
    if (!frame.data) {
        return 0;
    }
    frame.data_size = 2048;
    
    int retry = 3;
    while (frame.payload && retry) {
        if (ether_hl_send(eth, &frame)) {
            frame.payload = dblk_delete(frame.payload);
            retry = 3;
        }
        else {
            --retry;
        }
    }

    free(frame.data);
	
	return 1;
}

#include <dts/datastruct/list.h>
static list_t arp_table;
static int list_match_arpti(void *n, void *d)
{
    #define ARPTI(n) ((ether_arp_ti_t *)(n))
    #define to_u32(u8arr) (*((uint32_t *)(u8arr)))
    return (to_u32(ARPTI(n)->ip) == to_u32(d));
}
uint8_t *ether_arp_get_mac_address(ether_t *ether, uint8_t *target_ip)
{
    // try to find mac address in local map table
    ether_arp_ti_t *arpti = list_find(&arp_table, list_match_arpti, target_ip);
    if (arpti) {
        return arpti->mac;
    }

    // if not found, send a arp packet
    ip_t *localhost = ether_arp_get_ip(ether);

    uint8_t arp_pkt_data[32];
    arp_packet_t arp_pkt;
    arp_pkt.data = arp_pkt_data;
    arp_pkt.data_size = 32;

    uint8_t ether_frm_data[64];
    ether_frame_t frame;
    frame.dest_mac_addr = (uint8_t *)"\xFF\xFF\xFF\xFF\xFF\xFF";
    frame.type = ETHER_TYPE_ARP;
	dblk_t dblk = DATA_BLOCK(arp_pkt_data, 0);
    frame.payload = &dblk;
    frame.payload->size = ether_arp_ask_ip(&arp_pkt, 
        ether->mac_address, localhost->addr.addr.v4, target_ip);
    frame.data = ether_frm_data;
    frame.data_size = 64;
    ether_hl_send(ether, &frame);

    return NULL;
}

static list_t nif_list;
static int list_match_ip_addr(void *n, void *d)
{
    nif_t *nif = (nif_t *)n;
    ip_addr_t *ipaddr = (ip_addr_t *)d;

    ipaddr->netmask = nif->ip->addr.netmask;
    return ip_addr_equals(ipaddr, &nif->ip->addr);
}
void ether_arp_ether_recv(ether_t *ether, arp_packet_t *pkt)
{
    if (arp_unpack(pkt)) {
        if (pkt->op == ARP_OP_RPL) {
            // receive an arp reply, save it to arp table.
            while (1) {
                ether_arp_ti_t *arpti = mem_alloc_arpti();
                if (arpti) {
                    memcpy(arpti->ip, pkt->spa, 4);
                    memcpy(arpti->mac, pkt->sha, 6);
                    // TODO: start time
                    arpti->lease = 7200; // 2h
                    list_add(&arp_table, arpti);
                    break;
                }
                else {
                    // TODO: delete the shortest one
                    //mem_free_arpti(arpti);
					break;
                }
            }
            return;
        }
        if (pkt->op == ARP_OP_REQ) {
            IPv4_ADDR_DEF(tipa, 
                pkt->tpa[0], pkt->tpa[1], pkt->tpa[2], pkt->tpa[3]);
            nif_t *nif = (nif_t *)list_find(&nif_list, 
                list_match_ip_addr, &tipa);
            if (nif) {
					uint8_t arp_pkt_data[32];
                    pkt->op = ARP_OP_RPL;
                    pkt->tha = pkt->sha;
                    pkt->tpa = pkt->spa;
                    pkt->sha = nif->eth->mac_address;
                    pkt->spa = nif->ip->addr.addr.v4;
					pkt->data = arp_pkt_data;
					pkt->data_size = 32;
					
                    uint8_t ether_frm_data[64];
                    ether_frame_t frame;
                    dblk_new_from_stack(&frame.payload,
                        pkt->data, arp_pack(pkt));
                    frame.dest_mac_addr = pkt->tha;
                    frame.type = ETHER_TYPE_ARP;
                    frame.data = ether_frm_data;
                    frame.data_size = 64;
                    ether_hl_send(ether, &frame);
            }
            return;
        }
    }
}

#include <dts/net/mem.h>
void ether_arp_resgiter_pair(ip_t *ip, ether_t *eth)
{
    nif_t *nif = mem_alloc_nif();
    if (nif) {
        ip->ll = (void *)eth;
        ip->ll_send = (int (*)(void *, ip_datagram_t *))ip_ether_send;
        nif->ip = ip;
        nif->eth = eth;
        list_add(&nif_list, nif);
    }
}

static int list_match_eth(void *n, void *d)
{
    return (((nif_t *)n)->eth == ((ether_t *)d));
}
ip_t *ether_arp_get_ip(ether_t *eth)
{
    return ((nif_t * )list_find(&nif_list, list_match_eth, eth))->ip;
}

static int list_match_lan_addr(void *n, void *d)
{
    nif_t *nif = (nif_t *)n;
    ip_addr_t *ipaddr = (ip_addr_t *)d;
    
    ipaddr->netmask = nif->ip->addr.netmask;
    return ip_addr_same_lan(ipaddr, &nif->ip->addr);
}
ip_t * ether_arp_get_ip_by_addr(ip_addr_t *addr)
{
    return ((nif_t * )list_find(&nif_list, list_match_lan_addr, addr))->ip;
}
