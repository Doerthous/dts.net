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

uint32_t dblk_size(dblk_t *dblk)
{
    uint32_t size = 0;

    while (dblk) {
        size += dblk->size;
        dblk = dblk->next;
    }

    return size;
}


dblk_t *dblk_new1(uint8_t *data, uint32_t size)
{
    dblk_t *dblk;

    dblk = malloc(sizeof(dblk_t));
    dblk = dblk_init(dblk, data, size);
    if (dblk) {
        dblk->node_malloc = 1;
    }

    return dblk;
}

dblk_t *dblk_new2(uint32_t size)
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

void dblk_delete1(dblk_t * dblk)
{
    while (dblk) {
        dblk = dblk_delete2(dblk);
    }
}

dblk_t *dblk_delete2(dblk_t * dblk)
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
