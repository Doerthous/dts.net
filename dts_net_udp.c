#include <dts/net/udp.h>
#include <dts/net/checksum.h>
#include <dts/net/pseudo_header.h>
#include <dts/net/endian.h>
#include <dts/net/dblk.h>

#include <string.h>

uint32_t udp_pack(udp_datagram_t *dg)
{
    uint8_t *ptr = dg->raw_data;

    dg->length = 8+dg->data_size;

    ptr = bw16(dg->src_port, ptr);
    ptr = bw16(dg->dest_port, ptr);
    ptr = bw16(dg->length, ptr);
    ptr = bw16(0, ptr);
    memcpy(ptr, dg->data, dg->data_size); ptr+=dg->data_size;
    dg->raw_data_size = (ptr-dg->raw_data);

    uint64_t sum = 0;
    pseudo_header_set_length(dg->pseudo_header, dg->length);
    sum = checksum_accmulate(sum, 
        dg->pseudo_header, dg->pseudo_header_size);
    sum = checksum_accmulate(sum, 
        dg->raw_data, dg->raw_data_size);
    dg->checksum = checksum_complete(sum);

    bw16(dg->checksum, dg->raw_data+6);

    return dg->raw_data_size;
}


int udp_unpack(udp_datagram_t *dg)
{
    if (dg->raw_data_size < 8) {
        return 0;
    }

    dg->src_port = br16(dg->raw_data);
    dg->dest_port = br16(dg->raw_data+2);
    dg->length = br16(dg->raw_data+4);
    dg->checksum = br16(dg->raw_data+6);
    dg->data = dg->raw_data+8;
    dg->data_size = dg->length-8;

    bw16(0, dg->raw_data+6); // clear checksum

    uint64_t sum = 0;
    pseudo_header_set_length(dg->pseudo_header, dg->length);
    sum = checksum_accmulate(sum, 
        dg->pseudo_header, dg->pseudo_header_size);
    sum = checksum_accmulate(sum, 
        dg->raw_data, dg->raw_data_size);

    return dg->checksum == checksum_complete(sum);
}

#include <dts/net/dblk.h>
static uint32_t udp_pack_ext(udp_datagram_t *dg)
{
    uint8_t *header = dg->raw_data_ext->data;
    uint16_t header_size = dg->raw_data_ext->size;
    uint8_t *payload = dg->raw_data_ext->next->data;
    uint16_t payload_size = dg->raw_data_ext->next->size;

    dg->length = header_size+payload_size;

    header = bw16(dg->src_port, header);
    header = bw16(dg->dest_port, header);
    header = bw16(dg->length, header);
    header = bw16(0, header);

    uint64_t sum = 0;
    pseudo_header_set_length(dg->pseudo_header, dg->length);
    sum = checksum_accmulate(sum, 
        dg->pseudo_header, dg->pseudo_header_size);
    sum = checksum_accmulate(sum, header, header_size);
    sum = checksum_accmulate(sum, payload, payload_size);
    dg->checksum = checksum_complete(sum);

    bw16(dg->checksum, header+6);

    return dg->length;
}


#include <dts/net/ip.h>
#include <dts/datastruct/list.h>
typedef struct { ip_t *ip; int port; } _addr_t;
static list_t udp_list;
int list_match(void *n, void *d)
{
    udp_t *u = (udp_t *)n;
    _addr_t *v = (_addr_t *)d;
    return (u->low_level == v->ip) && (u->port == v->port);
}
static udp_t *find_udp_by_addr(ip_t *ip, int port)
{
	_addr_t val;
	val.ip = ip;
	val.port = port;
	return (udp_t *)list_find(&udp_list, list_match, (void *)&val);
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
    datagram.raw_data = ip_datagram->payload->data;
    datagram.raw_data_size = ip_datagram->payload->size;

    int ret;
    { 
        uint8_t phdr[12];
        pseudo_header_construct(&ip_datagram->header, phdr, 12);
        datagram.pseudo_header = phdr;
        datagram.pseudo_header_size = 12;
        ret = udp_unpack(&datagram);
    }
    if (ret) {
        udp_t *udp = find_udp_by_addr(ip, datagram.dest_port);
        if (udp && udp->callback) {
            udp->callback(udp->cb_data, datagram.data, datagram.data_size);
        }
    }

    return ret;
}


static int _sendto(ip_t *ip, ip_addr_t *dist, 
    uint16_t port, uint8_t *data, uint16_t size)
{
    ip_datagram_t ip_datagram;
    udp_datagram_t datagram;

    ip_datagram.header.options = NULL;
    ip_datagram.header.option_size = 0;
    IP_MKTOS(&ip_datagram.header, 0, 0, 0, 0);
    ip_datagram.header.protocol = IP_PROTOCOL_UDP;
    ip_datagram.header.time_to_live = 128;
    ip_datagram.header.source_address = ip->addr.addr.v4;
    ip_datagram.header.destination_address = dist->addr.v4;
    
    uint8_t udp_header[8];
	dblk_alloc_from_stack(&ip_datagram.payload, udp_header, 8);
    dblk_t udp_payload_dblk = DATA_BLOCK(data, size);
    ip_datagram.payload->next = &udp_payload_dblk;
    ip_datagram.payload->more = 1;

    #define udp_random_port() 12435
    datagram.src_port = udp_random_port();
    datagram.dest_port = port;
    datagram.data = data;
    datagram.data_size = size;
    datagram.raw_data_ext = ip_datagram.payload;

    {
        uint8_t pseudo_header[12];
        pseudo_header_construct(&ip_datagram.header, 
            pseudo_header, 12);
        datagram.pseudo_header = pseudo_header;
        datagram.pseudo_header_size = 12;
        udp_pack_ext(&datagram);
    }

    return ip_hl_send(ip, &ip_datagram);
}

int udp_sendto(ip_addr_t *dist, uint16_t port, uint8_t *data, uint16_t size)
{
    if (dist->version != 4) {
        return 0;
    }
    
    ip_t *ip = ip_find_netif(dist);
    if (ip) {
        return _sendto(ip, dist, port, data, size);
    }

    return 0;
}
