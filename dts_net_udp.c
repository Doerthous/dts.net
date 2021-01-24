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

#include <dts/net/udp.h>
#include <dts/net/checksum.h>
#include <dts/net/pseudo_header.h>
#include <dts/net/endian.h>
#include <dts/net/dblk.h>

#include <string.h>


int udp_unpack(udp_datagram_t *dg)
{
    if (dg->raw_data->size < 8) {
        return 0;
    }

    dg->src_port = br16(dg->raw_data->data);
    dg->dest_port = br16(dg->raw_data->data+2);
    dg->psdhdr->length = dg->length = br16(dg->raw_data->data+4);
    dg->checksum = br16(dg->raw_data->data+6);
    dg->payload->data = dg->raw_data->data+8;
    dg->payload->size = dg->length-8;

    bw16(0, dg->raw_data->data+6); // clear checksum

    pseudo_header_pack(dg->psdhdr);

    uint64_t sum = 0;
    sum = checksum_accmulate(sum, dg->psdhdr->data, dg->psdhdr->data_size);
    sum = checksum_accmulate(sum, dg->raw_data->data, dg->raw_data->size);
    return dg->checksum == checksum_complete(sum);
}

size_t udp_pack(udp_datagram_t *dg)
{
    dblk_t *header = dg->raw_data;
    //uint8_t *payload = dg->raw_data->next->data;
   // uint16_t payload_size = dg->raw_data->next->size;

    dg->psdhdr->length = dg->length = header->size+dg->payload->size;

    // do header
    bw16(dg->src_port, header->data);
    bw16(dg->dest_port, header->data+2);
    bw16(dg->length, header->data+4);
    bw16(0, header->data+6); // clear checksum

    // do pseudo header
    pseudo_header_pack(dg->psdhdr);

    // do checksume
    uint64_t sum = 0;
    sum = checksum_accmulate(sum, dg->psdhdr->data, dg->psdhdr->data_size);
    sum = checksum_accmulate(sum, header->data, header->size);
    sum = checksum_accmulate(sum, dg->payload->data, dg->payload->size);
    dg->checksum = checksum_complete(sum);
    bw16(dg->checksum, header->data+6); // checksum

    dblk_node_concat(header, dg->payload);

    return dg->length;
}


#include <dts/net/ip.h>
#include <dts/datastruct/list.h>
static list_t udp_list;
static udp_t *find_udp_by_addr(ip_t *ip, int port)
{
    list_foreach(udp_t, udp, &udp_list, {
        if (((*udp)->low_level == ip) && (*udp)->port == port) {
            return (*udp);
        }
    });

    return NULL;
}

#include <dts/net/mem.h>
udp_t *udp_bind(ip_t *ip, int port, udp_recv_t cbk,void *cbkd)
{
    udp_t *udp = NULL;

    // check whether address is in used.
    if (!find_udp_by_addr(ip, port)) {
        udp_t *udp = mem_alloc_udp();
        if (udp) {
            udp->low_level = ip;
            udp->port = port;
            udp->callback = cbk;
            udp->cb_data = cbkd;
            list_add(&udp_list, udp);
            return udp;
        }
    }
    
	return udp;
}

int udp_ip_recv(ip_t *ip, ip_datagram_t *ip_datagram)
{
    udp_datagram_t datagram;
    datagram.raw_data = ip_datagram->payload;

	dblk_node_new_from_stack(&datagram.payload, NULL, 0);
	
    int ret;
    { 
        pseudo_header_new_from_stack(&datagram.psdhdr, 
            &ip_datagram->header.src, &ip_datagram->header.dest, 
            ip_datagram->header.protocol, 0);
        ret = udp_unpack(&datagram);
    }
    if (ret) {
        udp_t *udp = find_udp_by_addr(ip, datagram.dest_port);
        if (udp && udp->callback) {
            udp->callback(udp->cb_data, 
                datagram.payload->data, datagram.payload->size);
        }
    }

    return ret;
}


static int _sendto(ip_t *ip, ip_addr_t *dest, 
    uint16_t port, uint8_t *data, uint16_t size)
{
    ip_datagram_t ip_datagram;

    ip_datagram.header.options = NULL;
    ip_datagram.header.option_size = 0;
    IP_MKTOS(&ip_datagram.header, 0, 0, 0, 0);
    ip_datagram.header.protocol = IP_PROTOCOL_UDP;
    ip_datagram.header.time_to_live = 128;
    ip_datagram.header.src = ip->addr;
    ip_datagram.header.dest = *dest;
    

    udp_datagram_t udp_datagram;
    #define udp_random_port() 12435
    udp_datagram.src_port = udp_random_port();
    udp_datagram.dest_port = port;
	dblk_node_new_from_stack_with_buff(&udp_datagram.raw_data, 8);
	dblk_node_new_from_stack(&udp_datagram.payload, data, size);
    {
        pseudo_header_new_from_stack(&udp_datagram.psdhdr, 
            &ip->addr, dest, IP_PROTOCOL_UDP, 0);
        udp_pack(&udp_datagram);
    }

    ip_datagram.payload = udp_datagram.raw_data;

    return ip_hl_send(ip, &ip_datagram);
}

int udp_sendto(ip_addr_t *dest, uint16_t port, uint8_t *data, uint16_t size)
{
    if (dest->version != 4) {
        return 0;
    }
    
    ip_t *ip = ip_find_netif(dest);
    if (ip) {
        return _sendto(ip, dest, port, data, size);
    }

    return 0;
}
