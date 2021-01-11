#ifndef DTS_NETWORK_CHECKSUM_H_
#define DTS_NETWORK_CHECKSUM_H_

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

#endif // DTS_NETWORK_CHECKSUM_H_
