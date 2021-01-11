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

#ifndef DTS_NET_DBLK_H_
#define DTS_NET_DBLK_H_

#include <stdint.h>

typedef struct dts_net_dblk
{
    struct dts_net_dblk *next;
    uint8_t *data;
    uint16_t size;
    uint16_t data_malloc:1;
    uint16_t more:1;
    uint16_t node_malloc:1;
} dts_net_dblk_t;

dts_net_dblk_t *dts_net_dblk_init(dts_net_dblk_t *dblk, uint8_t *data, uint16_t size);

uint32_t dts_net_dblk_size(dts_net_dblk_t *dblk);

#define dts_net_dblk_alloc_from_stack(ppdblk, _data, _size) \
    dts_net_dblk_t dblk##__LINE__; \
    *(ppdblk) = dts_net_dblk_init(&dblk##__LINE__, _data, _size)


dts_net_dblk_t *dts_net_dblk_new1(uint8_t *data, uint32_t size);
dts_net_dblk_t *dts_net_dblk_new2(uint32_t size);

void dts_net_dblk_delete1(dts_net_dblk_t * dblk);
dts_net_dblk_t *dts_net_dblk_delete2(dts_net_dblk_t * dblk);

#endif // DTS_NET_DBLK_H_
