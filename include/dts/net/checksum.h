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

#ifndef DTS_NET_CHECKSUM_H_
#define DTS_NET_CHECKSUM_H_

#include <stdint.h>
#include <dts/net/endian.h>

static inline uint16_t checksum(uint8_t *data, uint32_t size)
{
    uint32_t sum = 0;
    for (int i = 0; i < size; i+=2) {
        sum += br16(data);
        data += 2;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
	return ~sum;
}

static inline uint64_t checksum_accmulate
(
    uint64_t acc, 
    uint8_t *data, 
    uint32_t size
)
{
    for (int i = 0; i < size; i+=2) {
        acc += br16(data);
        data += 2;
    }
    return acc;
}
static inline uint16_t checksum_complete
(
    uint64_t acc
)
{
    while (acc >> 16) {
        acc = (acc & 0xFFFF) + (acc >> 16);
    }
	return ~acc;
}

#endif // DTS_NET_CHECKSUM_H_
