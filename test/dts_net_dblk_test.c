#include <dts/net/dblk.h>
#include <stdlib.h>
#include <stdio.h>

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
    dblk_t *b1 = dblk_new_with_data(12);
    dblk_t *b2 = dblk_new_with_data(12);
    dblk_t *b3 = dblk_new_with_data(12);
    dblk_concat(b1, b2);
    dblk_concat(b2, b3);

    dblk_t *it = b1;
    while (it) {
        for (int i = 0; i < it->size; ++i) {
            printf("%02X ", it->data[i]);
        }
        it = it->next;
    }
    printf("\n");

    dblk_copy_from(b1, "123456789098765432101234567890123456", 36);
    it = b1;
    while (it) {
        for (int i = 0; i < it->size; ++i) {
            printf("%02X ", it->data[i]);
        }
        it = it->next;
    }
    printf("\n");

    char buff[36];
    int c = dblk_copy_to(b1, buff, 35);
    for (int i = 0; i < c; ++i) {
         printf("%02X ", buff[i]);
    }
    printf("\n");

    dblk_t *fragments = dblk_fragment(b1, 7);
    while (fragments) {
        memset(buff, 0, 36);
        int c = dblk_copy_to(fragments, buff, 36);
        for (int i = 0; i < c; ++i) {
             printf("%02X ", buff[i]);
        }
        printf("\n");
        fragments = dblk_delete(fragments);
    }
}
