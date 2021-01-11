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

#ifndef DTS_NET_ICMP_H_
#define DTS_NET_ICMP_H_

#include <stdint.h>
#include <stddef.h>

#define ICMP_TYPE_ECHO_REPLY    0
#define ICMP_TYPE_UNREACHABLE   3
#define ICMP_TYPE_ECHO          8

typedef struct
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t identifier;
            uint16_t sequence_number;
            uint8_t *data;
            size_t data_size;
        } echo_or_echo_reply;
        struct {
            uint8_t *data;
            size_t data_size;
        } unreachable;
    } message;

    uint8_t *data;
    size_t data_size;
} dts_net_icmp_t;

int dts_net_icmp_pack(dts_net_icmp_t *datagram);
int dts_net_icmp_unpack(dts_net_icmp_t *datagram);

#include <dts_net_ip.h>
void 
dts_net_icmp_ip_recv
(
    dts_net_ip_t *ip, 
    dts_net_ip_datagram_t *ip_datagram
);

#endif // DTS_NET_ICMP_H_
