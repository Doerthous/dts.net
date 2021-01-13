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

#ifndef DTS_NET_MEM_H_
#define DTS_NET_MEM_H_

#include <dts_net_conf.h>
#include <stddef.h>
#include <stdint.h>

extern void dts_net_mem_init(void);

extern void *dts_net_malloc(size_t size);
extern void dts_net_free(void *ptr);

#include <dts_net_udp.h>
extern dts_net_udp_t *dts_net_mem_alloc_udp(void);
extern void dts_net_mem_free_udp(dts_net_udp_t *udp);

#include <dts_net_ether_arp.h>
extern dts_net_nif_t *dts_net_mem_alloc_nif(void);
extern void dts_net_mem_free_nif(dts_net_nif_t *nif);
extern dts_net_ether_arp_ti_t *dts_net_mem_alloc_arpti(void);
extern void dts_net_mem_free_arpti(dts_net_ether_arp_ti_t *nif);

#include <dts_net_ip.h>
extern dts_net_ip_datagram_t *dts_net_mem_alloc_ip_datagram(void);
extern void dts_net_mem_free_ip_datagram(dts_net_ip_datagram_t *datagram);

#endif // DTS_NET_MEM_H_
