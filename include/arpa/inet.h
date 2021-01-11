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

#ifndef INET_H_
#define INET_H_

union _4barr
{
    uint32_t mem;
    struct {
        uint8_t arr[4];
    } arr;
};

union _2barr
{
    uint16_t mem;
    struct {
        uint8_t arr[2];
    } arr;
};

static uint32_t htonl(uint32_t hostlong)
{
    uint8_t b;
    union _4barr *ba = (union _4barr *)(&hostlong);
    b = ba->arr.arr[0];
    ba->arr.arr[0] = ba->arr.arr[3];
    ba->arr.arr[3] = b;
    b = ba->arr.arr[1];
    ba->arr.arr[1] = ba->arr.arr[2];
    ba->arr.arr[2] = b;
    return hostlong;
}
static inline uint16_t htons(uint16_t hostshort)
{
    uint8_t b;
    union _2barr *ba = (union _2barr *)(&hostshort);
    b = ba->arr.arr[0];
    ba->arr.arr[0] = ba->arr.arr[1];
    ba->arr.arr[1] = b;
    return hostshort;
}
static inline uint32_t ntohl(uint32_t netlong)
{
    return htonl(netlong);
}
static inline uint16_t ntohs(uint16_t netshort)
{
    return htons(netshort);
}

#endif // INET_H_
