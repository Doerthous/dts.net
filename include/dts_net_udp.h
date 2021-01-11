#ifndef DTS_NETWORK_UDP_H_
#define DTS_NETWORK_UDP_H_

#include <stdint.h>
#include <dts_net_dblk.h>

typedef struct 
{
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
    uint8_t *data;
    uint32_t data_size;

    uint8_t *raw_data;
    uint32_t raw_data_size;
    dts_net_dblk_t *raw_data_ext;

    uint8_t *pseudo_header;
    uint32_t pseudo_header_size;
} dts_net_udp_datagram_t;

uint32_t dts_net_udp_pack(dts_net_udp_datagram_t *dg);
int dts_net_udp_unpack(dts_net_udp_datagram_t *dg);


#include <dts_net_ip.h>

typedef void (*dts_net_udp_recv_t)(void *, uint8_t *, uint32_t);

typedef struct
{
    void *next; // list

    void *low_level;
    int port;
    void *cb_data;
    dts_net_udp_recv_t callback;
} dts_net_udp_t;

dts_net_udp_t *
dts_net_udp_bind
( 
    dts_net_ip_t *ip, 
    int port,
    dts_net_udp_recv_t callback,
    void *cb_data
);
	
int 
dts_net_udp_ip_recv
(
    dts_net_ip_t *ip, 
    dts_net_ip_datagram_t *ip_datagram
);

int
dts_net_udp_sendto
(
    dts_net_ip_addr_t *dist,
    uint16_t port,
    uint8_t *data,
    uint16_t size
);

#endif // DTS_NETWORK_UDP_H_
