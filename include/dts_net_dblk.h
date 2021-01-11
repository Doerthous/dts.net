#ifndef DTS_NETWORK_DATA_BLOCK_H_
#define DTS_NETWORK_DATA_BLOCK_H_

#include <stdint.h>

typedef struct dts_net_dblk
{
    struct dts_net_dblk *next;
    uint8_t *data;
    uint16_t size;
    uint16_t data_malloc:1;
    uint16_t more:1;
    uint16_t node_malloc:1;
} dts_net_dblk_t;

dts_net_dblk_t *dts_net_dblk_init(dts_net_dblk_t *dblk, uint8_t *data, uint16_t size);

uint32_t dts_net_dblk_size(dts_net_dblk_t *dblk);

#define dts_net_dblk_alloc_from_stack(ppdblk, _data, _size) \
    dts_net_dblk_t dblk##__LINE__; \
    *(ppdblk) = dts_net_dblk_init(&dblk##__LINE__, _data, _size)


dts_net_dblk_t *dts_net_dblk_new1(uint8_t *data, uint32_t size);
dts_net_dblk_t *dts_net_dblk_new2(uint32_t size);

void dts_net_dblk_delete1(dts_net_dblk_t * dblk);
dts_net_dblk_t *dts_net_dblk_delete2(dts_net_dblk_t * dblk);

#endif // DTS_NETWORK_DATA_BLOCK_H_
