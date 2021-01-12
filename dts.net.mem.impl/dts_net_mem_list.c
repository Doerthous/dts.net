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

#include <dts/net/mem.h>
#include <dts/datastruct/list.h>

#include <dts/net/udp.h>
#include <dts/net/ether_arp.h>
static list_t udp_list;
static udp_t udps[DTS_NET_SERVER_UDP_MAX_COUNT];
static list_t nif_list;
static nif_t nifs[DTS_NET_NET_IF_MAX_COUNT];
static list_t arpti_list;
static ether_arp_ti_t arptis[DTS_NET_ARP_TABLE_MAX_SIZE];
void mem_init(void)
{
    for (int i = 0; i < DTS_NET_SERVER_UDP_MAX_COUNT; ++i) {
        list_enqueue(&udp_list, &udps[i]);
    }

    for (int i = 0; i < DTS_NET_NET_IF_MAX_COUNT; ++i) {
        list_enqueue(&nif_list, &nifs[i]);
    }

    for (int i = 0; i < DTS_NET_ARP_TABLE_MAX_SIZE; ++i) {
        list_enqueue(&arpti_list, &arptis[i]);
    }
}


udp_t *dts_net_mem_alloc_udp()
{
    return list_dequeue(&udp_list);
}
void dts_net_mem_free_udp(udp_t *udp)
{
    list_enqueue(&udp_list, udp);
}


nif_t *dts_net_mem_alloc_nif()
{
    return list_dequeue(&nif_list);
}
void dts_net_mem_free_nif(nif_t *nif)
{
    list_enqueue(&nif_list, nif);
}


ether_arp_ti_t *mem_alloc_arpti()
{
    return list_dequeue(&arpti_list);
}
void mem_free_arpti(ether_arp_ti_t *arpti)
{
    list_enqueue(&arpti_list, arpti);
}
