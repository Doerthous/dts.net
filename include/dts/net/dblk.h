/*
   The MIT License (MIT)

   Copyright (c) 2021 Doerthous

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

#ifndef DBLK_H_
#define DBLK_H_

#include <dts_net_dblk.h>

#define DATA_BLOCK(_data, _size) \
{ \
    .next = (void *)0, \
    .data = (_data), \
    .size = (_size), \
    .data_malloc = 0, \
    .more = 0, \
    .node_malloc = 0, \
    .vsize = 0, \
    .vmem = (void *)0, \
}

#define dblk_t dts_net_dblk_t

#define dblk_node_init dts_net_dblk_node_init
#define dblk_node_new_from_stack dts_net_dblk_node_new_from_stack
#define dblk_node_new dts_net_dblk_node_new
#define dblk_node_new_with_buff dts_net_dblk_node_new_with_buff
#define dblk_node_size dts_net_dblk_node_size
#define dblk_node_next dts_net_dblk_node_next
#define dblk_node_concat dts_net_dblk_node_concat
#define dblk_node_is_vmem_node dts_net_dblk_node_is_vmem_node
#define dblk_node_to_rmem_node dts_net_dblk_node_to_rmem_node

#define dblk_list_delete dts_net_dblk_list_delete
#define dblk_list_has_vmem_node dts_net_dblk_list_has_vmem_node
#define dblk_list_to_rmem_list dst_net_dblk_list_to_rmem_list

#define dblk_delete dts_net_dblk_delete
#define dblk_size dts_net_dblk_size
#define dblk_next dts_net_dblk_next
#define dblk_copy_to dts_net_dblk_copy_to
#define dblk_copy_from dts_net_dblk_copy_from
#define dblk_fragment dts_net_dblk_fragment




#endif // DBLK_H_
