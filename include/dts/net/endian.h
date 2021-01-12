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

#ifndef ENDIAN_H_
#define ENDIAN_H_

#include <stdint.h>

static inline uint16_t lr16(uint8_t *data)
{
    return (data[1] << 8) | data[0];
}

static inline uint8_t * lw16(uint16_t data, uint8_t *buff)
{
    buff[0] = data & 0xFF;
    buff[1] = (data >> 8) & 0xFF;
    return buff+2;
}

static inline uint32_t lr32(uint8_t *data)
{
    return (data[3] << 24) | (data[2] << 16) |
        (data[1] << 8) | data[0];
}

static inline uint8_t * lw32(uint32_t data, uint8_t *buff)
{
    buff[0] = data & 0xFF;
    buff[1] = (data >> 8) & 0xFF;
    buff[2] = (data >> 16) & 0xFF;
    buff[3] = (data >> 24) & 0xFF;
    return buff+4;
}

static inline uint16_t br16(uint8_t *data)
{
    return (data[0] << 8) | data[1];
}

static inline uint8_t * bw16(uint16_t data, uint8_t *buff)
{
    buff[1] = data & 0xFF;
    buff[0] = (data >> 8) & 0xFF;
    return buff+2;
}

static inline uint32_t br32(uint8_t *data)
{
    return (data[0] << 24) | (data[1] << 16) |
        (data[2] << 8) | data[3];
}

static inline uint8_t * bw32(uint32_t data, uint8_t *buff)
{
    buff[3] = data & 0xFF;
    buff[2] = (data >> 8) & 0xFF;
    buff[1] = (data >> 16) & 0xFF;
    buff[0] = (data >> 24) & 0xFF;
    return buff+4;
}

#endif // ENDIAN_H_
