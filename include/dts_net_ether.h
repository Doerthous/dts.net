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

#ifndef DTS_NET_ETHER_H_
#define DTS_NET_ETHER_H_

#include <stdint.h>
#include <stddef.h>
#include <dts_net_dblk.h>

#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IP   0x0800

typedef struct
{
    uint8_t *dest_mac_addr;
    uint8_t *src_mac_addr;
    dts_net_dblk_t *payload;
    uint16_t type;
    uint32_t crc;
    //
    uint8_t *data;
    size_t data_size;
} dts_net_ether_frame_t;

// there are used by ether module internally.
int dts_net_ether_pack(dts_net_ether_frame_t *frame);
int dts_net_ether_unpack(dts_net_ether_frame_t *frame);

typedef struct
{
    size_t (*send)(void *interface, uint8_t *data, size_t size);
    size_t (*recv)(void *interface, uint8_t *data, size_t size);
    void *interface;
    uint8_t mac_address[6];
} dts_net_ether_t;

// low level call me to notify me that there received a ether frame.
int 
dts_net_ether_ll_recv
(
    dts_net_ether_t* ether, 
    dts_net_ether_frame_t *frame
);

// high level call me to send ether frame.
int 
dts_net_ether_hl_send
(
    dts_net_ether_t* ether, 
    dts_net_ether_frame_t *frame
);

#endif // DTS_NET_ETHER_H_
