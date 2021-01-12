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


#include <dts/net/mem.h>
#include <dts/net/udp.h>
#include <dts/net/ether_arp.h>

void mem_init(void)
{

}


udp_t *dts_net_mem_alloc_udp()
{
    return malloc(sizeof(udp_t));
}
void dts_net_mem_free_udp(udp_t *udp)
{
    free(udp);
}


nif_t *dts_net_mem_alloc_nif()
{
    return malloc(sizeof(nif_t));
}
void dts_net_mem_free_nif(nif_t *nif)
{
    free(nif);
}


ether_arp_ti_t *mem_alloc_arpti()
{
    return malloc(sizeof(ether_arp_ti_t));
}
void mem_free_arpti(ether_arp_ti_t *arpti)
{
    free(arpti);
}
