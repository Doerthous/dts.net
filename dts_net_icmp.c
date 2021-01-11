#include <dts/net/icmp.h>
#include <dts/net/ip.h>
#include <dts/net/checksum.h>
#include <string.h>
int icmp_pack(icmp_t *datagram)
{
    uint8_t *ptr = datagram->data;

    *ptr++ = datagram->type;
    *ptr++ = datagram->code = 0;
    ptr = bw16(0, ptr);

    switch (datagram->type) {
        case ICMP_TYPE_ECHO_REPLY:
        case ICMP_TYPE_ECHO: {
            if (datagram->data_size < 8) {
                return 0;
            }
            ptr = bw16(datagram->message.echo_or_echo_reply.identifier, ptr);
            ptr = bw16(datagram->message.echo_or_echo_reply.sequence_number, ptr);
            memcpy(ptr, 
                datagram->message.echo_or_echo_reply.data, 
                datagram->message.echo_or_echo_reply.data_size);
            datagram->data_size = 8+datagram->message.echo_or_echo_reply.data_size;
        } break;
        case ICMP_TYPE_UNREACHABLE: {
            ptr = bw32(0, ptr);
            memcpy(ptr, 
                datagram->message.unreachable.data, 
                datagram->message.unreachable.data_size);
            datagram->data_size = 8+datagram->message.unreachable.data_size;
        } break;
    }

	datagram->checksum = checksum(datagram->data, datagram->data_size);
    bw16(datagram->checksum, datagram->data+2);

    return 1;
}
int icmp_unpack(icmp_t *datagram)
{
    if (datagram->data_size < 4) {
        return 0;
    }

    datagram->type = datagram->data[0];
    datagram->code = datagram->data[1];
    datagram->checksum = br16(datagram->data+2);
    
    switch (datagram->type) {
        case ICMP_TYPE_ECHO_REPLY:
        case ICMP_TYPE_ECHO: {
            if (datagram->data_size < 8) {
                return 0;
            }
            datagram->message.echo_or_echo_reply.identifier = br16(datagram->data+4);
            datagram->message.echo_or_echo_reply.sequence_number = br16(datagram->data+6);
            datagram->message.echo_or_echo_reply.data = datagram->data+8;
            datagram->message.echo_or_echo_reply.data_size = datagram->data_size-8;
            return 1;
        };
        case ICMP_TYPE_UNREACHABLE: {
            if (datagram->data_size < 8) {
                return 0;
            }
            datagram->message.unreachable.data = datagram->data+8;
            datagram->message.unreachable.data_size = datagram->data_size-8;
            return 1;
        };
    }
    return 0;
}


void icmp_ip_recv(ip_t *ip, ip_datagram_t *ip_datagram)
{
    icmp_t datagram;
    datagram.data = ip_datagram->payload->data;
    datagram.data_size = ip_datagram->payload->size;
    if (icmp_unpack(&datagram)) {
        ip_datagram->header.destination_address = ip_datagram->header.source_address;
        ip_datagram->header.source_address = ip->addr.addr.v4;

        switch (datagram.type) {
            case ICMP_TYPE_ECHO: {                
                datagram.type = ICMP_TYPE_ECHO_REPLY;
                datagram.data = ip_datagram->payload->data;
                datagram.data_size = ip_datagram->payload->size;
                icmp_pack(&datagram);                
                ip_hl_send(ip, ip_datagram);
			} break;
        }
    }
}
