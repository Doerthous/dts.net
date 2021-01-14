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

#ifndef INET_H_
#define INET_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static union
{
    char ch[4];
    uint32_t u32;
} endian = {{'L', '?', '?', 'B'}};
#define INET_ENDIAN ((char)endian.u32)



static inline uint32_t htonl(uint32_t hl)
{
    if (INET_ENDIAN == 'L') {
        hl = ((hl<<24)|(hl>>24))|((hl&0x00FF0000)>>8)|((hl&0x0000FF00)<<8);
    }

    return hl;
}
static inline uint16_t htons(uint16_t hs)
{
    if (INET_ENDIAN == 'L') {
        hs = (hs<<8)|(hs>>8);
    }

    return hs;
}
static inline uint32_t ntohl(uint32_t nl)
{
    return htonl(nl);
}
static inline uint16_t ntohs(uint16_t ns)
{
    return htons(ns);
}

static inline uint32_t inet_addr(const char *str)
{
    uint32_t hl = 0;
    #define pHL ((char *)&hl)
    for (int i = 0; i < 4 && str; ++i,++str) {
        pHL[i] = atoi(str);
        str = strchr(str, '.');
    }
    return htonl(hl);
}

#endif // INET_H_
