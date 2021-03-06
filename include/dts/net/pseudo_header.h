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

#ifndef PSEUDO_HEADER_H_
#define PSEUDO_HEADER_H_

#include <dts_net_pseudo_header.h>
#include <dts_net_ip.h>
#include <dts/net/endian.h>
#include <string.h>

#define pseudo_header_t dts_net_pseudo_header_t

static inline size_t pseudo_header_pack(pseudo_header_t *ph)
{
    if (ph->data_size < 12) {
        return 0;
    }

    memcpy(ph->data, ph->src->addr.v4, 4);
    memcpy(ph->data+4, ph->dest->addr.v6, 4);
    ph->data[8] = 0;
    ph->data[9] = ph->protocol;
    bw16(ph->length, ph->data+10);
    
    return 12;
}

#define PSEUDO_HEADER_CONCAT_(x,y) x##y
#define PSEUDO_HEADER_CONCAT(x,y) PSEUDO_HEADER_CONCAT_(x,y)
#define pseudo_header_new_from_stack(ppphdr, _src, _dest, _proto, _len) \
    uint8_t PSEUDO_HEADER_CONCAT(phdr_buf, __LINE__)[12]; \
    pseudo_header_t PSEUDO_HEADER_CONCAT(phdr, __LINE__) = { \
        .src = _src, .dest = _dest, .protocol = _proto, .length = _len, \
        .data = PSEUDO_HEADER_CONCAT(phdr_buf, __LINE__), .data_size = 12, \
    }; \
    *(ppphdr) = &PSEUDO_HEADER_CONCAT(phdr, __LINE__);


#endif // PSEUDO_HEADER_H_
