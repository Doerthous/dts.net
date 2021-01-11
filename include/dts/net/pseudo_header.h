#ifndef DTS_NETWORK_PSEUDO_HEADER_H_
#define DTS_NETWORK_PSEUDO_HEADER_H_

#include <dts_net_ip.h>
#include <dts/net/endian.h>
#include <string.h>

static inline uint32_t pseudo_header_construct
(
    dts_net_ip_header_t *ip_hdr,
    uint8_t *buff,
    uint32_t size
)
{
    if (size < 12) {
        return 0;
    }

    memcpy(buff, ip_hdr->source_address, 4);
    memcpy(buff+4, ip_hdr->destination_address, 4);
    buff[8] = 0;
    buff[9] = ip_hdr->protocol;
    bw16(0, buff+10); // This length field leave to high level protocol
    
    return 12;
}

static inline void pseudo_header_set_length
(
    uint8_t *data,
    uint32_t length
)
{
    bw16(length, data+10);
}

#endif // DTS_NETWORK_PSEUDO_HEADER_H_
