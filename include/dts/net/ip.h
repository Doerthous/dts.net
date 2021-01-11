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
