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

#include <dts/net/dblk.h>
#include <dts/net/mem.h>

// node
static size_t dblk_node_copy_to(dts_net_dblk_t *dblk, void *mem, size_t size)
{
    size_t do_size;
    if (dblk->vsize == 0) {
        do_size = size > dblk->size ? dblk->size : size;
        memcpy(mem, dblk->data, do_size);
    }
    else {
        do_size = size > dblk->vsize ? dblk->vsize : size;
        if (dblk->vsize > dblk->size) {
            size_t remain_size = dblk->vsize-dblk->size;
            memcpy(mem, dblk->data, dblk->size);
            mem = ((uint8_t *)mem) + dblk->size;
            dts_net_dblk_t *vmem = dblk->vmem;
            while (remain_size && vmem) {
                if (remain_size > vmem->size) {
                    memcpy(mem, vmem->data, vmem->size);
                    mem = ((uint8_t *)mem) + vmem->size;
                    remain_size -= vmem->size;
                }
                else {
                    memcpy(mem, vmem->data, remain_size);
                    mem = ((uint8_t *)mem) + remain_size;
                    break;
                }
                vmem = vmem->next;
            }
        }
        else {
            memcpy(mem, dblk->data, dblk->vsize);
        }
    }
    return do_size;
}

dblk_t *dblk_init(dblk_t *dblk, uint8_t *data, uint16_t size)
{
    if (dblk) {
        dblk->next = NULL;
        dblk->data = data;
        dblk->data_malloc = dblk->node_malloc = dblk->more = 0;
        dblk->size = size;
        dblk->vsize = 0;
        dblk->vmem = NULL;
    }
    return dblk;
}

dblk_t *dblk_new(uint8_t *data, size_t size)
{
    dblk_t *dblk;

    dblk = dblk_init(malloc(sizeof(dblk_t)), data, size);
    if (dblk) {
        dblk->node_malloc = 1;
    }

    return dblk;
}

dblk_t *dblk_new_with_data(size_t size)
{
    dblk_t *dblk = NULL;
    uint8_t *data;

    data = malloc(size);
    if (data) {
        dblk = dblk_new(data, size);
        if (dblk) {
            dblk->data_malloc = 1;
        }
        else {
            free(data);
            dblk = NULL;
        }
    }

    return dblk;
}

// list
void dblk_delete_all(dblk_t * dblk)
{
    while (dblk) {
        dblk = dblk_delete(dblk);
    }
}

// block
dblk_t *dblk_delete(dblk_t * dblk)
{
    dblk_t *p;
    int more;

    while (dblk) {
        p = dblk;
        dblk = dblk->next;
        more = p->more;
        if (p->data_malloc) {
            free(p->data);
        }
        if (p->node_malloc) {
            free(p);
        }
        if (!more) {
            break;
        }
    }

    return dblk;
}

size_t dblk_size(const dblk_t *dblk)
{
    size_t size = 0;

    while (dblk) {
        if (dblk->vsize == 0) {
            size += dblk->size;
        }
        else {
            size += dblk->vsize;
        }
        if (!dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return size;
}

dblk_t *dblk_next(dblk_t * dblk)
{
    while (dblk) {
        if (!dblk->more) {
            return dblk->next;
        }
        dblk = dblk->next;
    }
    
    return dblk;
}

size_t dblk_copy_to(dts_net_dblk_t *dblk, void *mem, size_t size)
{
    size_t total = size;
    size_t do_size;

    while (dblk) {
        do_size = dblk_node_copy_to(dblk, mem, size);
        size -= do_size;
        mem = ((uint8_t *)mem) + do_size;
        if (size == 0 || !dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return total-size;
}

size_t dblk_copy_from(dts_net_dblk_t *dblk, void *mem, size_t size)
{
    size_t total = size;
    size_t do_size;

    while (dblk) {
        do_size = size > dblk->size ? dblk->size : size;
        memcpy(dblk->data, mem, do_size);
        size -= do_size;
        mem = ((uint8_t *)mem) + do_size;
        if (size == 0 || !dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return total-size;
}

dblk_t *dblk_fragment(const dblk_t *dblk, size_t fsize)
{
    size_t lgb_size;
    size_t fg_cnt;
    dblk_t *hdr, *f, *e;

    if (!dblk || dblk->size == 0) {
        return NULL;
    }

    lgb_size = dblk_size(dblk);
    fg_cnt = lgb_size/fsize+(lgb_size%fsize>0);

    e = hdr = dblk_new(NULL, 0);
    for (int i = 1; i < fg_cnt; ++i) {
        f = dblk_new(NULL, 0);
        e->next = f;
        e = f;
    }

    e = hdr;
    size_t offset = 0;
    while (e && dblk) {
        e->data = dblk->data + offset;
        e->size = dblk->size - offset;
        e->vsize = fsize;
        if (dblk->size > offset+fsize) {
            offset += fsize;
            e = e->next;
            continue;
        }
        if (dblk->size < offset+fsize) {
            size_t remain_size = offset+fsize - dblk->size;
            f = e->vmem = dblk->next;
            while (f && f->size < remain_size) {
                remain_size -= f->size;
                if (!f->more) {
                    break;
                }
                f = f->next;
            }
            if (!f) {
                e->vsize -= remain_size;
            }
            offset = remain_size;
            dblk = f;
            e = e->next;
            continue;
        }
        offset = 0;
        dblk = dblk->next;
        e = e->next;
    }

    return hdr;
}
