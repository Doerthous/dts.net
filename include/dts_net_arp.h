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

#ifndef DTS_NET_ARP_H_
#define DTS_NET_ARP_H_

#include <stdint.h>

#define ARP_OP_REQ 1
#define ARP_OP_RPL 2

typedef struct 
{
    uint16_t hrd; // Hardware address space. For Ethernet, this is 1.
    uint16_t pro; // Protocol address space. For Ethernet
                  // hardware, this is from the set of type
                  // fields of ether frame.
    uint8_t hln; // byte length of each hardware address
    uint8_t pln; // byte length of each protocol address
    uint16_t op; // opcode (ARP_OP_REQ | ARP_OP_RPL)
    uint8_t *sha; // Hardware address of sender of this
                  // packet, n from the hln field.
    uint8_t *spa; // Protocol address of sender of this
                  // packet, m from the pln field.
    uint8_t *tha; // Hardware address of target of this
                  // packet (if known).
    uint8_t *tpa; // Protocol address of target.

    uint8_t *data;
    uint8_t data_size;
} dts_net_arp_packet_t;

uint32_t dts_net_arp_pack(dts_net_arp_packet_t *pkt);
int dts_net_arp_unpack(dts_net_arp_packet_t *pkt);

#endif // DTS_NET_ARP_H_
