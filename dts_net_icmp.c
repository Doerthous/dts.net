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
        ip_datagram->header.dest = ip_datagram->header.src;
        ip_datagram->header.src = ip->addr;

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
