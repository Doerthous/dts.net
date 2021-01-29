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
#define DTS_NET_MEM_BASIC_API
#include <dts/net/mem.h>
#include <string.h>


// Node
static size_t dblk_node_copy_to(dblk_t *dblk, void *mem, size_t size)
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
            dblk_t *vmem = dblk->vmem;
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

dblk_t *dblk_node_init(dblk_t *dblk, uint8_t *data, uint16_t size)
{
    if (dblk) {
        dblk->next = NULL;
        dblk->data = data;
        dblk->data_malloc = dblk->node_malloc = dblk->more = 0;
        dblk->size = size;
        dblk->vsize = 0;
        dblk->vmem = NULL;
        dblk->offset = 0;
    }
    return dblk;
}

dblk_t *dblk_node_new(uint8_t *data, size_t size)
{
    dblk_t *dblk;

    dblk = dblk_node_init(malloc(sizeof(dblk_t)), data, size);
    if (dblk) {
        dblk->node_malloc = 1;
    }

    return dblk;
}

dblk_t *dblk_node_new_with_buff(size_t size)
{
    dblk_t *dblk = NULL;
    uint8_t *data;

    data = malloc(size);
    if (data) {
        dblk = dblk_node_new(data, size);
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

size_t dblk_node_size(const dblk_t *dblk)
{
    if (dblk) {
        if (dblk->vsize) {
            return dblk->vsize;
        }
        else {
            return dblk->size;
        }
    }
    return 0;
}

dblk_t *dblk_node_to_rmem_node(dblk_t *dblk, uint8_t *data, size_t size)
{
    if (dblk_node_is_vmem_node(dblk)) {
        dblk->size = dblk_node_copy_to(dblk, data, size);
        dblk->data = data;
        dblk->vsize = 0;
        dblk->vmem = NULL;
    }
	return dblk;
}

dblk_t *dblk_node_copy(dblk_t *dblk)
{
    dblk_t *cpy;
    cpy = dblk_node_new_with_buff(dblk_node_size(dblk));
    if (cpy) {
        dblk_copy_to(dblk, cpy->data, cpy->size);
    }
	return cpy;
}

// List
void dblk_list_delete(dblk_t * dblk)
{
    while (dblk) {
        dblk = dblk_delete(dblk);
    }
}

int dblk_list_has_vmem_node(dblk_t *dblk)
{
    while (dblk) {
        if (dblk_node_is_vmem_node(dblk)) {
            return 1;
        }
        dblk = dblk_node_next(dblk);
    }
    return 0;
}

int dblk_list_to_rmem_list(dblk_t *dblk)
{
    while (dblk) {
        if (dblk_node_is_vmem_node(dblk)) {
            size_t blksz = dblk_node_size(dblk);
            uint8_t *data = (uint8_t *)malloc(blksz);
            if (!data) {
                return 0;
            }
            dblk->data_malloc = 1;
            dblk_node_to_rmem_node(dblk, data, blksz);
        }
        dblk = dblk->next;
    }

    return 1;
}

// Block
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
        size += dblk_node_size(dblk);
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

size_t dblk_copy_to(dblk_t *dblk, void *mem, size_t size)
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

size_t dblk_copy_from(dblk_t *dblk, void *mem, size_t size)
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

    e = hdr = dblk_node_new(NULL, 0);
    for (int i = 1; i < fg_cnt; ++i) {
        f = dblk_node_new(NULL, 0);
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

void dblk_merge(dblk_t *b, dblk_t *n)
{
    if (n) {
        while (1) {
            if (!b->more) {
                n->next = b->next;
                b->next = n;
                b->more = 1;
                break;
            }
            b = b->next;
        }
    }
}

dblk_t *dblk_copy(dblk_t *dblk)
{
    dblk_t *cpy = dblk_node_new_with_buff(dblk_size(dblk));
	if (cpy) {
		dblk_copy_to(dblk, cpy->data, cpy->size);
	}
	return cpy;
}


int dblk_seek(dblk_t *dblk, size_t offset)
{
    while (dblk) {
        if (dblk->size > offset) {
            dblk->offset = offset;
            offset -= offset;
        }
        else {
            dblk->offset = dblk->size;
            offset -= dblk->size;
        }
        if (!dblk->more) {
            break;
        }
        dblk = dblk->next;
    }
    
    return 1;
}


size_t dblk_write(dblk_t *dblk, void *mem, size_t size)
{
    size_t do_size = 0;
    size_t done_size = 0;

    while (dblk && done_size < size) {
        do_size = dblk->size - dblk->offset;
        if (do_size > 0) {
            do_size = do_size > size ? size : do_size;
            memcpy(dblk->data+dblk->offset, mem, do_size);
            dblk->offset += do_size;
        }
        done_size += do_size;
        if (!dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return done_size;
}

size_t dblk_read(dblk_t *dblk, void *mem, size_t size)
{
    size_t do_size = 0;
    size_t done_size = 0;

    while (dblk && done_size < size) {
        do_size = dblk->size - dblk->offset;
        if (do_size > 0) {
            do_size = do_size > size ? size : do_size;
            memcpy(mem, dblk->data+dblk->offset, do_size);
            dblk->offset += do_size;
        }
        done_size += do_size;
        if (!dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return done_size;
}
