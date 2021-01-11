#ifndef DTS_NETWORK_ICMP_H_
#define DTS_NETWORK_ICMP_H_

#include <stdint.h>
#include <stddef.h>

#define ICMP_TYPE_ECHO_REPLY    0
#define ICMP_TYPE_UNREACHABLE   3
#define ICMP_TYPE_ECHO          8

typedef struct
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t identifier;
            uint16_t sequence_number;
            uint8_t *data;
            size_t data_size;
        } echo_or_echo_reply;
        struct {
            uint8_t *data;
            size_t data_size;
        } unreachable;
    } message;

    uint8_t *data;
    size_t data_size;
} dts_net_icmp_t;

int dts_net_icmp_pack(dts_net_icmp_t *datagram);
int dts_net_icmp_unpack(dts_net_icmp_t *datagram);

#include <dts_net_ip.h>
void 
dts_net_icmp_ip_recv
(
    dts_net_ip_t *ip, 
    dts_net_ip_datagram_t *ip_datagram
);

#endif // DTS_NETWORK_ICMP_H_
