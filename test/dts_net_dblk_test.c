#include <dts/net/dblk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void *dts_net_malloc(size_t size)
{
    return malloc(size);
}
void dts_net_free(void *ptr)
{
    free(ptr);
}

int main()
{
    dblk_t *b1 = dblk_node_new_with_buff(12);
    dblk_t *b2 = dblk_node_new_with_buff(12);
    dblk_t *b3 = dblk_node_new_with_buff(12);
    dblk_node_concat(b1, b2);
    dblk_node_concat(b2, b3);

    dblk_t *it = b1;
    while (it) {
        for (int i = 0; i < it->size; ++i) {
            printf("%02X ", it->data[i]);
        }
        it = dblk_node_next(it);
    }
    printf("\n");

    dblk_copy_from(b1, "123456789098765432101234567890123456", 36);
    char buff[36];
    {
        int c = (int)dblk_copy_to(b1, buff, 36);
        for (int i = 0; i < c; ++i) {
            printf("%02X ", buff[i]);
        }
        printf("\n");
    }

    dblk_t *fragments = dblk_fragment(b1, 7);
    it = fragments;
    while (it) {
        memset(buff, 0, 36);
        int c = (int)dblk_copy_to(it, buff, 36);
        printf("%sn: ", dblk_node_is_vmem_node(it)?"v":" ");
        for (int i = 0; i < c; ++i) {
             printf("%02X ", buff[i]);
        }
        printf("\n");
        it = dblk_next(it);
    }

    if (dblk_list_to_rmem_list(fragments)) {
        it = fragments;
        while (it) {
            memset(buff, 0, 36);
            int c = (int)dblk_copy_to(it, buff, 36);
            printf("%sn: ", dblk_node_is_vmem_node(it)?"v":" ");
            for (int i = 0; i < c; ++i) {
                printf("%02X ", buff[i]);
            }
            printf("\n");
            it = dblk_next(it);
        }
    }
    dblk_list_delete(fragments);


    fragments = dblk_fragment(b1, 5);
    it = fragments;
    while (it) {
        memset(buff, 0, 36);
        int c = (int)dblk_copy_to(it, buff, 36);
        printf("%sn: ", dblk_node_is_vmem_node(it)?"v":" ");
        for (int i = 0; i < c; ++i) {
             printf("%02X ", buff[i]);
        }
        printf("\n");
        it = dblk_next(it);
    }
    dblk_list_delete(fragments);

    dblk_list_delete(b1);


    printf("--- dblk_seek, dblk_write, dblk_read test ---\n");
    uint8_t mem[16];
    size_t sz;
    b1 = dblk_node_new_with_buff(256);
    for (int i = 0; 1; ++i) {
        snprintf(mem, 16, "Hello World [%d]", i%10);
        if (!dblk_write(b1, mem, 16)) {
            break;
        }
    }
    printf("dblk_seek 0\n");
    dblk_seek(b1, 0);
    do {
        if (sz = dblk_read(b1, mem, 16)) {
            printf("size: %d, %s\n", sz, (char *)mem);
        }
    } while (sz);
    printf("dblk_seek 64\n");
    dblk_seek(b1, 64);
    do {
        if (sz = dblk_read(b1, mem, 16)) {
            printf("size: %d, %s\n", sz, (char *)mem);
        }
    } while (sz);

    dblk_list_delete(b1);

    return 0;
}
