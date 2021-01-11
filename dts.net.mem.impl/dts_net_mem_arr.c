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

#include <dts/net/mem.h>

void mem_init(void)
{

}

#include <dts/net/udp.h>
static struct 
{
    udp_t udp;
    int used;
} udps[DTS_NET_SERVER_UDP_MAX_COUNT];
udp_t *dts_net_mem_alloc_udp()
{
    for (int i = 0; i < DTS_NET_SERVER_UDP_MAX_COUNT; ++i) {
        if (!udps[i].used) {
            udps[i].used = 1;
            return &udps[i].udp;
        }
    }
	
	return NULL;
}
void dts_net_mem_free_udp(udp_t *udp)
{
    for (int i = 0; i < DTS_NET_SERVER_UDP_MAX_COUNT; ++i) {
        if (&udps[i].udp == udp) {
            udps[i].used = 0;
        }
    }
}


#include <dts/net/ether_arp.h>
static struct { nif_t nif; int used; } nifs[DTS_NET_NET_IF_MAX_COUNT];
nif_t *dts_net_mem_alloc_nif()
{
    for (int i = 0; i < DTS_NET_NET_IF_MAX_COUNT; ++i) {
        if (!nifs[i].used) {
            nifs[i].used = 1;
            return &nifs[i].nif;
        }
    }
	
	return NULL;
}
void dts_net_mem_free_nif(nif_t *nif)
{
    for (int i = 0; i < DTS_NET_NET_IF_MAX_COUNT; ++i) {
        if (&nifs[i].nif == nif) {
            nifs[i].used = 0;
        }
    }
}

static struct 
{
    ether_arp_ti_t arpti;
    int used;
} arptis[DTS_NET_ARP_TABLE_MAX_SIZE];
ether_arp_ti_t *mem_alloc_arpti()
{
    for (int i = 0; i < DTS_NET_ARP_TABLE_MAX_SIZE; ++i) {
        if (!arptis[i].used) {
            arptis[i].used = 1;
            return &arptis[i].arpti;
        }
    }
	
	return NULL;
}
void mem_free_arpti(ether_arp_ti_t *arpti)
{
    for (int i = 0; i < DTS_NET_ARP_TABLE_MAX_SIZE; ++i) {
        if (&arptis[i].arpti == arpti) {
            arptis[i].used = 0;
        }
    }
}
