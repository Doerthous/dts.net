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

#include <dts/net/ip.h>
#include <dts/net/checksum.h>
#include <dts/net/dblk.h>
#include <dts/net/mem.h>

#include <string.h>

static inline size_t ip_pack(ip_header_t *hdr, uint8_t *data, size_t size)
{
    uint8_t *ptr = data;

    if (size < hdr->option_size+20) {
        return 0;
    }

    // compute header size (32bit align)
    size = hdr->option_size+20;
    if (size & 0x03) { 
        size = size - (size & 0x03) + 4;
    }
    memset(data, 0, size);
    
    // total_length contains the length of data
    // here we append header size to total_length
    hdr->total_length += size; 

    // 
    hdr->checksum = 0;

    *data++ = (size/4)|(hdr->version<<4);
    *data++ = hdr->type_of_service;
    bw16(hdr->total_length, data); data += 2;
    bw16(hdr->identification, data); data += 2;
    bw16((hdr->flags<<13)|(hdr->fragment_offset), data); data += 2;
    *data++ = hdr->time_to_live;
    *data++ = hdr->protocol;
    bw16(hdr->checksum, data); data += 2;
    memcpy(data, hdr->source_address, 4); data += 4;
    memcpy(data, hdr->destination_address, 4); data += 4;
    memcpy(data, hdr->options, hdr->option_size);

    // compute crc
    hdr->checksum = checksum(ptr, size);
    bw16(hdr->checksum, ptr+10);
    
    return size;
}

static inline int ip_unpack(ip_datagram_t *datagram)
{
    ip_header_t *hdr = &datagram->header;
    uint8_t *data = datagram->raw_data->data;
    uint32_t size = datagram->raw_data->size;

    if (size < 20) {
        return 0;
    }

    hdr->version = ((data[0] & 0xF0) >> 4);
    hdr->ihl = (data[0] & 0x0F);

    if (hdr->ihl * 4 > size) {
        return 0;
    }

    hdr->type_of_service = data[1];
    hdr->total_length = br16(data+2);
    hdr->identification = br16(data+4);
    hdr->identification = br16(data+6);
    hdr->time_to_live = data[8];
    hdr->protocol = data[9];
    hdr->checksum = br16(data+10);
    hdr->source_address = data+12;
    hdr->destination_address = data+16;

    // TODO: do options
    if (size == 20) {
        hdr->options = NULL;
        hdr->option_size = 0;
    }

    datagram->payload->data = data+(hdr->ihl*4);
    datagram->payload->size = size-(hdr->ihl*4);

    return 1;
}

int ip_hl_send(ip_t *ip, ip_datagram_t *datagram)
{
    static int ident;
    uint8_t header_buff[64];
    dblk_t ip_header = DATA_BLOCK(header_buff, 64);
    size_t payload_size = dblk_size(datagram->payload);

    datagram->header.version = ip->addr.version;
    datagram->header.source_address = ip->addr.addr.v4;
    datagram->header.identification = ++ident;

    #define MKFLAGS(DF, MF) (((DF)<<1)|(MF))

    // dblk:
    //   ip-header   ->   payload
    if (payload_size < 1400) {
        datagram->header.total_length = payload_size;
        datagram->header.fragment_offset = 0;

        // pack ip header
        ip_header.size = ip_pack(&datagram->header,
            ip_header.data, ip_header.size);

        dblk_node_concat(&ip_header, datagram->payload);
        datagram->raw_data = &ip_header;
        
        return ip->ll_send(ip->ll, datagram);
    }
    else {
        dblk_t *pl_fgm_curr = dblk_fragment(datagram->payload, 1400);
        dblk_t *pl_fgm_next = dblk_next(pl_fgm_curr);
        ip_datagram_t fragment = *datagram;
        fragment.header.fragment_offset = 0;
        while (pl_fgm_curr) {

            // do header
            fragment.header.total_length = dblk_size(pl_fgm_curr);
            fragment.header.flags = MKFLAGS(0, (pl_fgm_next!=NULL));
            ip_header.size = ip_pack(&fragment.header,
                ip_header.data, ip_header.size);

            // concat header and payload_fragments
            pl_fgm_curr->next = NULL; // TODO: disconnect, because ether module will send all dblk
            dblk_node_concat(&ip_header, pl_fgm_curr);
            fragment.raw_data = &ip_header;

            // send one fragment
            if (!ip->ll_send(ip->ll, &fragment)) {
                dblk_list_delete(pl_fgm_curr);
                return 0;
            }

            // update header
            fragment.header.fragment_offset += fragment.header.total_length/8;
            pl_fgm_curr = pl_fgm_next;
            pl_fgm_next = dblk_next(pl_fgm_next);
        }

        return 1;
    }
}

#include <dts/net/icmp.h>
#include <dts/net/udp.h>
void ip_ll_recv(ip_t *ip, ip_datagram_t *datagram)
{
    dblk_node_new_from_stack(&datagram->payload, NULL, 0);
    if (ip_unpack(datagram)) {
        // do fragment
        switch (datagram->header.protocol) {
            case IP_PROTOCOL_ICMP:
                icmp_ip_recv(ip, datagram);
                break;
            case IP_PROTOCOL_TCP:
                break;
            case IP_PROTOCOL_UDP: {
                udp_ip_recv(ip, datagram);
            } break;
        }
    }
}

#include <dts/net/ether_arp.h>
ip_t *ip_find_netif(ip_addr_t *addr)
{
    if (addr->version == 4) {
        return ether_arp_get_ip_by_addr(addr);
    }

    return NULL;
}

int ip_addr_equals(ip_addr_t *a1, ip_addr_t *a2)
{
    if (a1->version == a2->version) {
        #define au32(u8arr) (*((uint32_t *)(u8arr)))
        return au32(a1->addr.v4) == au32(a2->addr.v4);
    }

    return 0;
}

int ip_addr_same_lan(ip_addr_t *a1, ip_addr_t *a2)
{
    if (a1->version == a2->version) {
        uint32_t a1u32,a2u32,a1mu32,a2mu32;
        a1u32 = au32(a1->addr.v4);
        a2u32 = au32(a2->addr.v4);
        a1mu32 = au32(a1->netmask.v4);
        a2mu32 = au32(a2->netmask.v4);
        return ((a1u32&a1mu32) == (a2u32&a2mu32));
    }

    return 0;
}
