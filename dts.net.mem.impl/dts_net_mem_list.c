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


#include <dts/net/udp.h>
#include <dts/net/ether_arp.h>
#include <dts/net/ip.h>

MEM_ITEM_LIST_API(udp_t, udp, DTS_NET_SERVER_UDP_MAX_COUNT)
MEM_ITEM_LIST_API(nif_t, nif, DTS_NET_NET_IF_MAX_COUNT)
MEM_ITEM_LIST_API(ether_arp_ti_t, arpti, DTS_NET_ARP_TABLE_MAX_SIZE)
MEM_ITEM_LIST_API(ip_datagram_t, ip_datagram, DTS_NET_IP_DATAGRAM_MAX_COUNT)

void mem_init(void)
{
    MEM_ITEM_LIST_API_INIT(udp, DTS_NET_SERVER_UDP_MAX_COUNT);
    MEM_ITEM_LIST_API_INIT(nif, DTS_NET_NET_IF_MAX_COUNT);
    MEM_ITEM_LIST_API_INIT(arpti, DTS_NET_ARP_TABLE_MAX_SIZE);
    MEM_ITEM_LIST_API_INIT(ip_datagram, DTS_NET_IP_DATAGRAM_MAX_COUNT);
}
