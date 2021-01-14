/*
   The MIT License (MIT)

   Copyright (c) 2020 Doerthous

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

   Author: Doerthous <doerthous@gmail.com>
*/

#ifndef DTS_NET_DBLK_H_
#define DTS_NET_DBLK_H_

#include <stdint.h>
#include <stddef.h>

typedef struct dts_net_dblk
{
    struct dts_net_dblk *next;
    uint8_t *data;
    uint16_t size;
    uint16_t data_malloc:1; // Indicate that the data field comes from malloc.
    uint16_t node_malloc:1; // Indicate that this dblk comes from malloc.

    /**
     * @brief Indicate that there is more dblk on the list.
     *
     *  Suppose we have a dblk list like below:
     *   Node:
     *   n1    ->    n2    ->   n3    ->    n4    ->    n5    ->  n6
     *   more:1      more:1     more:0      more:1      more:1    more:0
     *   -----------------------------      ----------------------------
     *   this is a logical **Block**        this is another logical Block
     *
     *  On this list, we have six node and two logical **Block**
     */
    uint16_t more:1;

    /**
     * @brief Virtual memory mode.
     * 
     *  When vsize != 0, dblk will enter virtual memory mode, **vsize** is the
     *  real size of the memory of the dblk node. **size** is the size of data.
     */
    uint16_t vsize;
    struct dts_net_dblk *vmem;
} dts_net_dblk_t;

// Node
/**
 * @brief Initialize the Node properties.
 * 
 * @param dblk [in]
 * @param data [in]
 * @param size [in]
 * @return dts_net_dblk_t* 
 */
dts_net_dblk_t *dts_net_dblk_node_init(dts_net_dblk_t *dblk, 
    uint8_t *data, uint16_t size);

/**
 * @brief New a Node from stack.
 *
 * @param ppdblk [in] dblk_t **, a pointer passed from outside.
 * @param _data [in] uint8_t *, data buffer.
 * @param _size [in] size_t, size of data buffer.
 */
void dts_net_dblk_node_new_from_stack(dts_net_dblk_t **ppdblk, 
    uint8_t *data, size_t size);
#define DTS_NET_DBLK_CONCAT_(x,y) x##y
#define DTS_NET_DBLK_CONCAT(x,y) DTS_NET_DBLK_CONCAT_(x,y)
#define dts_net_dblk_node_new_from_stack(ppdblk, _data, _size) \
    dts_net_dblk_t DTS_NET_DBLK_CONCAT(dblk,__LINE__); \
    *(ppdblk) = dts_net_dblk_node_init(&DTS_NET_DBLK_CONCAT(dblk,__LINE__), \
        _data, _size)

/**
 * @brief New a Node from stack with buffer.
 *
 * @param ppdblk [in] dblk_t **, a pointer passed from outside.
 * @param _size [in] size_t, size of data buffer.
 */
void dts_net_dblk_node_new_from_stack_with_buff(dts_net_dblk_t **ppdblk,
    size_t size);
#define dts_net_dblk_node_new_from_stack_with_buff(ppdblk, _size) \
    dts_net_dblk_t DTS_NET_DBLK_CONCAT(dblk,__LINE__); \
    uint8_t DTS_NET_DBLK_CONCAT(dblk_buf, __LINE__)[_size]; \
    *(ppdblk) = dts_net_dblk_node_init(&DTS_NET_DBLK_CONCAT(dblk,__LINE__), \
        DTS_NET_DBLK_CONCAT(dblk_buf,__LINE__), _size)

/**
 * @brief This new only new the dblk Node, the real data buffer is passed from
 *          outside, which means that the node_malloc of dblk returned by this
 *          function will be set, and the data_malloc will be clear.
 *
 * @param data [in] real data buffer.
 * @param size [in] data buffer size.
 * @return dts_net_dblk_t* dblk
 */
dts_net_dblk_t *dts_net_dblk_node_new(uint8_t *data, size_t size);

/**
 * @brief This new a dblk Node and a data buffer whose size is indicated by
 *          the **size** arguments, which means that both node_malloc and
 *          data_malloc of dblk returned by this function will be set.
 *
 * @param size [in] data buffer size.
 * @return dts_net_dblk_t* dblk
 * TODO: new_with_buff will be more nicer
 */
dts_net_dblk_t *dts_net_dblk_node_new_with_buff(size_t size);

size_t dts_net_dblk_node_size(const dts_net_dblk_t *dblk);

/**
 * @brief Get next Node.
 * 
 * @param dblk [in] current Node.
 * @return dts_net_dblkt_t* next Node.
 */
int dts_net_dblk_node_next(dts_net_dblk_t *dblk);
#define dts_net_dblk_node_next(dblk) (dblk)->next

/**
 * @brief Put Nodes into the same Block.
 * 
 * @param b1 [in] Node1
 * @param b2 [in] Node2
 */
void dts_net_dblk_node_concat(dts_net_dblk_t * b1, dts_net_dblk_t * b2);
#define dts_net_dblk_node_concat(b1, b2) do \
{ \
    (b1)->next = (b2); \
    (b1)->more = 1; \
} while (0)

/**
 * @brief Check wheter the given Node is a vmem-Node or not.
 * 
 * @param dblk [in] Node need to be checked.
 * @return int !0 if dblk is vmem-Node else 0.
 */
int dts_net_dblk_node_is_vmem_node(dts_net_dblk_t *dblk);
#define dts_net_dblk_node_is_vmem_node(dblk) ((dblk)->vsize > 0)

dts_net_dblk_t *dts_net_dblk_node_to_rmem_node(dts_net_dblk_t *dblk, 
    uint8_t *data, size_t size);

// List 
/**
 * @brief This will delete all Nodes of the list, No care the more field of 
 *          Nodes.
 *
 * @param dblk [in] dblk need to be deleted.
 */
void dts_net_dblk_list_delete(dts_net_dblk_t * dblk);

int dts_net_dblk_list_has_vmem_node(dts_net_dblk_t *dblk);

/**
 * @brief Convert all vmem-Node into rmem-Node.
 * 
 * @param dblk [in]
 * @return int !0 if all vmem-Node is converted into rmem-Node.
 */
int dst_net_dblk_list_to_rmem_list(dts_net_dblk_t *dblk);

// Block
/**
 * @brief Delete the first Block, return the second Block.
 *
 * @param dblk [in]
 * @return dts_net_dblk_t*
 */
dts_net_dblk_t *dts_net_dblk_delete(dts_net_dblk_t * dblk);

/**
 * @brief The size of the first Block.
 *          Loop the size until the more field of a dblk is 0.
 * @param dblk [in] dblk list.
 * @return size_t
 */
size_t dts_net_dblk_size(const dts_net_dblk_t *dblk);

/**
 * @brief Return the second Block.
 *
 * @param dblk [in]
 * @return dts_net_dblk_t*
 */
dts_net_dblk_t *dts_net_dblk_next(dts_net_dblk_t * dblk);

/**
 * @brief Copy the first Block to the given mem.
 *
 * @param dblk [in] dblk list.
 * @param mem [out] memory to which this op copy.
 * @note  Make sure the capability of the memory is bigger enough. Check the
 *        logical block size first, then do this call.
 */
size_t dts_net_dblk_copy_to(dts_net_dblk_t *dblk, void *mem, size_t size);

/**
 * @brief Copy the data of the given memory into the first Block.
 * 
 * @param dblk [in]
 * @param mem [in] 
 * @param size [in] the size of the given memory.
 * @return size_t the size of the data written to the first Block.
 * @note  Make sure there is not vmem-Node in the first Block, else the behavier
 *        is undefined.
 */
size_t dts_net_dblk_copy_from(dts_net_dblk_t *dblk, void *mem, size_t size);

/**
 * @brief Split the first Block into fragments. Each fragment's size is 
 *          specified by fsize, the size of last fragment may less than fsize.
 * 
 * @param dblk [in]
 * @param fsize [in]
 * @return dts_net_dblk_t*
 * @note  Make sure there is not vmem-Node in the first Block, else the behavier
 *        is undefined.
 */
dts_net_dblk_t *dts_net_dblk_fragment(const dts_net_dblk_t *dblk, size_t fsize);

#endif // DTS_NET_DBLK_H_
