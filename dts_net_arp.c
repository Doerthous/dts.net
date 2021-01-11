#include <dts/net/arp.h>
#include <dts/net/endian.h>
#include <string.h>

uint32_t arp_pack(arp_packet_t *pkt)
{
    uint8_t *ptr;

    if (pkt->data_size < 8+pkt->hln*2+pkt->pln*2) {
        return 0;
    }
    pkt->data_size = 8+pkt->hln*2+pkt->pln*2;

    ptr = pkt->data;

    ptr = bw16(pkt->hrd, ptr);
    ptr = bw16(pkt->pro, ptr);
    *ptr++ = pkt->hln;
    *ptr++ = pkt->pln;
    ptr = bw16(pkt->op, ptr);
    memcpy(ptr, pkt->sha, pkt->hln); ptr += pkt->hln;
    memcpy(ptr, pkt->spa, pkt->pln); ptr += pkt->pln;
    memcpy(ptr, pkt->tha, pkt->hln); ptr += pkt->hln;
    memcpy(ptr, pkt->tpa, pkt->pln); ptr += pkt->pln;

    return pkt->data_size;
}

int arp_unpack(arp_packet_t *pkt)
{
    if (pkt->data_size < 8) {
        return 0;
    }

    pkt->hrd = br16(pkt->data);
    pkt->pro = br16(pkt->data+2);
    pkt->hln = pkt->data[4];
    pkt->pln = pkt->data[5];
    pkt->op = br16(pkt->data+6);

    if (pkt->data_size < 8+pkt->hln*2+pkt->pln*2) {
        return 0;
    }
    
    pkt->sha = pkt->data+8;
    pkt->spa = pkt->sha+pkt->hln;
    pkt->tha = pkt->spa+pkt->pln;
    pkt->tpa = pkt->tha+pkt->hln;

    return 1;
}
