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
     *
     *   n1    ->    n2    ->   n3    ->    n4    ->    n5    ->  n6
     *   more:1      more:1     more:0      more:1      more:1    more:0
     *   -----------------------------      ----------------------------
     *   this is a **logical block**        this is another logical block
     *
     *  On this list, we have six node and **two logical block**
     */
    uint16_t more:1;

    // virtual memory mode

    /**
     * @brief Virtual memory mode.
     * 
     *  When vsize != 0, dblk will enter virtual memory mode, **vsize** is the
     *  real size of the memory of the dblk node. **size** is the size of data.
     */
    uint16_t vsize;
    struct dts_net_dblk *vmem;
} dts_net_dblk_t;

#define dts_net_dblk_concat(b1, b2) do \
{ \
    (b1)->next = (b2); \
    (b1)->more = 1; \
} while (0)

dts_net_dblk_t *dts_net_dblk_init(dts_net_dblk_t *dblk, uint8_t *data, uint16_t size);

/**
 * @brief New a dblk from stack.
 *
 * @param ppdblk [in] dblk_t **, a pointer passed from outside.
 * @param _data [in] uint8_t *, data buffer.
 * @param _size [in] size_t, size of data buffer.
 */
#define dts_net_dblk_new_from_stack(ppdblk, _data, _size) \
    dts_net_dblk_t dblk##__LINE__; \
    *(ppdblk) = dts_net_dblk_init(&dblk##__LINE__, _data, _size)

/**
 * @brief This new only new the dblk node, the real data buffer is passed from
 *          outside, which means that the node_malloc of dblk returned by this
 *          function will be set, and the data_malloc will be clear.
 *
 * @param data [in] real data buffer.
 * @param size [in] data buffer size.
 * @return dts_net_dblk_t* dblk
 */
dts_net_dblk_t *dts_net_dblk_new(uint8_t *data, size_t size);

/**
 * @brief This new new a dblk node and a data buffer whose size is indicated by
 *          the **size** arguments, which means that both node_malloc and
 *          data_malloc of dblk returned by this function will be set.
 *
 * @param size [in] data buffer size.
 * @return dts_net_dblk_t* dblk
 * TODO: new_with_buff will be more nicer
 */
dts_net_dblk_t *dts_net_dblk_new_with_data(size_t size);

/**
 * @brief This will delete all node of the list, No care the more field of node.
 *
 * @param dblk [in] dblk need to be deleted.
 */
void dts_net_dblk_delete_all(dts_net_dblk_t * dblk);

/**
 * @brief This will delete a logical block.
 *
 * @param dblk
 * @return dts_net_dblk_t*
 */
dts_net_dblk_t *dts_net_dblk_delete(dts_net_dblk_t * dblk);

/**
 * @brief The size of all dblk of the first logical dblk on the list.
 *          Loop the size until the more field of a dblk is 0.
 * @param dblk [in] dblk list.
 * @return size_t
 */
size_t dts_net_dblk_size(const dts_net_dblk_t *dblk);

/**
 * @brief Return the next logical block.
 *
 * @param dblk
 * @return dts_net_dblk_t*
 */
dts_net_dblk_t *dts_net_dblk_next(dts_net_dblk_t * dblk);

/**
 * @brief Copy the first logical block to the given mem.
 *
 * @param dblk [in] dblk list.
 * @param mem [out] memory to which this op copy.
 * @note  Make sure the capability of the memory is bigger enough. Check the
 *        logical block size first, then do this call.
 */
size_t dts_net_dblk_copy_to(dts_net_dblk_t *dblk, void *mem, size_t size);

size_t dts_net_dblk_copy_from(dts_net_dblk_t *dblk, void *mem, size_t size);

dts_net_dblk_t *dts_net_dblk_fragment(const dts_net_dblk_t *dblk, size_t fsize);

#endif // DTS_NET_DBLK_H_
