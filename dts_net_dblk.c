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

dblk_t *dblk_init(dblk_t *dblk, uint8_t *data, uint16_t size)
{
    if (dblk) {
        dblk->next = (void *)0;
		dblk->data = data;
        dblk->data_malloc = dblk->node_malloc = dblk->more = 0;
        dblk->size = size;
    }
    return dblk;
}

size_t dblk_size(dblk_t *dblk)
{
    size_t size = 0;

    while (dblk) {
        size += dblk->size;
        if (!dblk->more) {
            break;
        }
        dblk = dblk->next;
    }

    return size;
}


dblk_t *dblk_new(uint8_t *data, size_t size)
{
    dblk_t *dblk;

    dblk = malloc(sizeof(dblk_t));
    dblk = dblk_init(dblk, data, size);
    if (dblk) {
        dblk->node_malloc = 1;
    }

    return dblk;
}

dblk_t *dblk_new_with_data(size_t size)
{
    dblk_t *dblk;

    dblk = malloc(sizeof(dblk_t));
    dblk = dblk_init(dblk, 0, 0);
    if (dblk) {
        dblk->node_malloc = 1;
        dblk->data = malloc(size);
        if (dblk->data) {
            dblk->data_malloc = 1;
            dblk->size = size;
        }
        else {
            free(dblk);
            dblk = NULL;
        }
    }

    return dblk;
}

void dblk_delete_all(dblk_t * dblk)
{
    while (dblk) {
        dblk = dblk_delete(dblk);
    }
}

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
