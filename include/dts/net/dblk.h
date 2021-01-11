#ifndef DATA_BLOCK_H_
#define DATA_BLOCK_H_

#include <dts_net_dblk.h>

#define dblk_t dts_net_dblk_t
#define dblk_init dts_net_dblk_init
#define dblk_size dts_net_dblk_size
#define dblk_alloc_from_stack dts_net_dblk_alloc_from_stack

#define DATA_BLOCK(_data, _size) \
{ \
    .next = (void *)0, \
    .data = (_data), \
    .size = (_size), \
    .data_malloc = 0, \
    .more = 0, \
    .node_malloc = 0, \
}

static inline dblk_t DATA_BLOCK1(uint8_t *_data, uint16_t _size)
{
    dblk_t dblk;
    dblk.next = (void *)0;
    dblk.data = (_data);
    dblk.size = (_size);
    dblk.data_malloc = 0;
    dblk.more = 0;
    dblk.node_malloc = 0;
    return dblk;
}

#define dblk_new1 dts_net_dblk_new1
#define dblk_new2 dts_net_dblk_new2
#define dblk_delete1 dts_net_dblk_delete1
#define dblk_delete2 dts_net_dblk_delete2

#endif // DATA_BLOCK_H_
