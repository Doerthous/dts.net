#ifndef DBLK_H_
#define DBLK_H_

#include <dts_net_dblk.h>

#define dblk_t dts_net_dblk_t
#define dblk_init dts_net_dblk_init
#define dblk_size dts_net_dblk_size

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

#define dblk_new dts_net_dblk_new
#define dblk_new_with_data dts_net_dblk_new_with_data
#define dblk_new_from_stack dts_net_dblk_new_from_stack
#define dblk_delete dts_net_dblk_delete
#define dblk_delete_all dts_net_dblk_delete_all

#endif // DBLK_H_
