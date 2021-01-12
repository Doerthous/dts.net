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

#include <dts/net/arp.h>
#include <dts/net/endian.h>
#include <string.h>

uint32_t arp_pack(arp_packet_t *pkt)
{
    uint8_t *ptr;

    if (pkt->data_size < 8+pkt->hln*2+pkt->pln*2) {
        return 0;
    }
    pkt->data_size = 8+pkt->hln*2+pkt->pln*2;

    ptr = pkt->data;

    ptr = bw16(pkt->hrd, ptr);
    ptr = bw16(pkt->pro, ptr);
    *ptr++ = pkt->hln;
    *ptr++ = pkt->pln;
    ptr = bw16(pkt->op, ptr);
    memcpy(ptr, pkt->sha, pkt->hln); ptr += pkt->hln;
    memcpy(ptr, pkt->spa, pkt->pln); ptr += pkt->pln;
    memcpy(ptr, pkt->tha, pkt->hln); ptr += pkt->hln;
    memcpy(ptr, pkt->tpa, pkt->pln); ptr += pkt->pln;

    return pkt->data_size;
}

int arp_unpack(arp_packet_t *pkt)
{
    if (pkt->data_size < 8) {
        return 0;
    }

    pkt->hrd = br16(pkt->data);
    pkt->pro = br16(pkt->data+2);
    pkt->hln = pkt->data[4];
    pkt->pln = pkt->data[5];
    pkt->op = br16(pkt->data+6);

    if (pkt->data_size < 8+pkt->hln*2+pkt->pln*2) {
        return 0;
    }
    
    pkt->sha = pkt->data+8;
    pkt->spa = pkt->sha+pkt->hln;
    pkt->tha = pkt->spa+pkt->pln;
    pkt->tpa = pkt->tha+pkt->hln;

    return 1;
}
