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

void mem_init(void)
{

}

#define MEM_ITEM_ARR_API(type, name, count) \
 static struct { \
    type name; \
    int used; \
 } name##s[count]; \
 type *mem_alloc_##name() { \
    for (int i = 0; i < count; ++i) { \
        if (!name##s[i].used) { \
            name##s[i].used = 1; \
            return &name##s[i].name; \
        } \
    } \
    return NULL; \
 } \
 void mem_free_##name(type *name) { \
    for (int i = 0; i < count; ++i) { \
        if (&name##s[i].name == name) { \
            name##s[i].used = 0; \
        } \
    } \
 }

#include <dts/net/udp.h>
MEM_ITEM_ARR_API(udp_t, udp, DTS_NET_SERVER_UDP_MAX_COUNT)

#include <dts/net/ether_arp.h>
MEM_ITEM_ARR_API(nif_t, nif, DTS_NET_NET_IF_MAX_COUNT)
MEM_ITEM_ARR_API(ether_arp_ti_t, arpti, DTS_NET_ARP_TABLE_MAX_SIZE)

#include <dts/net/ip.h>
MEM_ITEM_ARR_API(ip_datagram_t, ip_datagram, DTS_NET_IP_DATAGRAM_MAX_COUNT)
