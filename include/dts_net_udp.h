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

#ifndef DTS_NET_UDP_H_
#define DTS_NET_UDP_H_

#include <stdint.h>
#include <dts_net_dblk.h>

typedef struct 
{
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t *data;
    uint32_t data_size;

    uint8_t *raw_data;
    uint32_t raw_data_size;
    dts_net_dblk_t *raw_data_ext;

    uint8_t *pseudo_header;
    uint32_t pseudo_header_size;
} dts_net_udp_datagram_t;

uint32_t dts_net_udp_pack(dts_net_udp_datagram_t *dg);
int dts_net_udp_unpack(dts_net_udp_datagram_t *dg);


#include <dts_net_ip.h>

typedef void (*dts_net_udp_recv_t)(void *, uint8_t *, uint32_t);

typedef struct
{
    void *next; // list

    void *low_level;
    int port;
    void *cb_data;
    dts_net_udp_recv_t callback;
} dts_net_udp_t;

dts_net_udp_t *
dts_net_udp_bind
( 
    dts_net_ip_t *ip, 
    int port,
    dts_net_udp_recv_t callback,
    void *cb_data
);
	
int 
dts_net_udp_ip_recv
(
    dts_net_ip_t *ip, 
    dts_net_ip_datagram_t *ip_datagram
);

int
dts_net_udp_sendto
(
    dts_net_ip_addr_t *dist,
    uint16_t port,
    uint8_t *data,
    uint16_t size
);

#endif // DTS_NET_UDP_H_
