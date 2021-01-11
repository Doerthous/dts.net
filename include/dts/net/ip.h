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

#ifndef IP_H_
#define IP_H_

#include <dts_net_ip.h>

#define ip_header_t dts_net_ip_header_t
#define ip_datagram_t dts_net_ip_datagram_t

#define ip_t dts_net_ip_t
#define ip_addr_t dts_net_ip_addr_t
#define ip_hl_send dts_net_ip_hl_send
#define ip_ll_recv dts_net_ip_ll_recv
#define ip_addr_equals  dts_net_ip_addr_equals
#define ip_addr_same_lan  dts_net_ip_addr_same_lan
#define ip_find_netif dts_net_ip_find_netif

#define IPv4_ADDR_DEF DTS_NET_IPv4_ADDR_DEF
#define IPv4_ADDR DTS_NET_IPv4_ADDR
#define IPv4_NETMASK DTS_NET_IPv4_NETMASK
#define IP_IPv4_ADDR DTS_NET_IP_IPv4_ADDR
#define IP_IPv4_MASK DTS_NET_IP_IPv4_MASK

#endif // IP_H_
