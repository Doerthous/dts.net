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

#ifndef DTS_NET_PSEUDO_HEADER_H_
#define DTS_NET_PSEUDO_HEADER_H_

#include <dts_net_ip.h>
#include <dts/net/endian.h>
#include <string.h>

static inline uint32_t pseudo_header_construct
(
    dts_net_ip_header_t *ip_hdr,
    uint8_t *buff,
    uint32_t size
)
{
    if (size < 12) {
        return 0;
    }

    memcpy(buff, ip_hdr->source_address, 4);
    memcpy(buff+4, ip_hdr->destination_address, 4);
    buff[8] = 0;
    buff[9] = ip_hdr->protocol;
    bw16(0, buff+10); // This length field leave to high level protocol
    
    return 12;
}

static inline void pseudo_header_set_length
(
    uint8_t *data,
    uint32_t length
)
{
    bw16(length, data+10);
}

#endif // DTS_NET_PSEUDO_HEADER_H_
