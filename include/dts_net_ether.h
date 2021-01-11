#ifndef DTS_NETWORK_ETHER_H_
#define DTS_NETWORK_ETHER_H_

#include <stdint.h>
#include <dts_net_dblk.h>

#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IP   0x0800

typedef struct
{
    uint8_t *dest_mac_addr;
    uint8_t *src_mac_addr;
    dts_net_dblk_t *payload;
    uint16_t type;
    uint32_t crc;
    //
    uint8_t *data;
    uint32_t data_size;
} dts_net_ether_frame_t;

// there are used by ether module internally.
int dts_net_ether_pack(dts_net_ether_frame_t *frame);
int dts_net_ether_unpack(dts_net_ether_frame_t *frame);

typedef struct
{
    uint32_t (*send)(void *interface, uint8_t *data, uint32_t size);
    uint32_t (*recv)(void *interface, uint8_t *data, uint32_t size);
    void *interface;
    uint8_t mac_address[6];
} dts_net_ether_t;

// low level call me to notify me that there received a ether frame.
int 
dts_net_ether_ll_recv
(
    dts_net_ether_t* ether, 
    dts_net_ether_frame_t *frame
);

// high level call me to send ether frame.
int 
dts_net_ether_hl_send
(
    dts_net_ether_t* ether, 
    dts_net_ether_frame_t *frame
);

#endif // DTS_NETWORK_ETHER_H_
