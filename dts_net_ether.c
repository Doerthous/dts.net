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

   Authour: Doerthous <doerthous@gmail.com>
*/

#include <dts/net/ether.h>
#include <dts/net/endian.h>
#include <dts/net/ether_arp.h>
#include <dts/net/arp.h>
#include <dts/net/ip.h>
#include <dts/net/dblk.h>

#include <string.h>

int ether_pack(ether_frame_t *frame)
{
    uint8_t *ptr = frame->data;

    memcpy(ptr, frame->dest_mac_addr, 6); ptr += 6;

    memcpy(ptr, frame->src_mac_addr, 6);  ptr += 6;

    if (frame->type < 0x05DC) {
        if (dblk_size(frame->payload) < 46) {
            ptr = bw16(46, ptr);
        }
    }
    else {
        ptr = bw16(frame->type, ptr);
    }


	dts_net_dblk_t *db = frame->payload;
	while (db) {
		memcpy(ptr, db->data, db->size);
		ptr += db->size;
		if (!db->more) {
			break;
		}
		db = db->next;
	}

    // TODO: if !auto_crc, then fill crc

    frame->data_size = ptr-frame->data;
    return 1;
}

int ether_unpack(ether_frame_t *frame)
{
    uint8_t *ptr = frame->data;

    if (frame->data_size < 64) {
        return 0;
    }

    frame->dest_mac_addr = ptr; ptr += 6;

    frame->src_mac_addr = ptr; ptr += 6;

    frame->type = br16(ptr); ptr += 2;

    frame->payload->size = frame->data_size-12-2-4;
    //memcpy(frame->payload, ptr, frame->payload_size);
	frame->payload->data = ptr;
    ptr += frame->payload->size;

    frame->crc = br32(ptr);

    return 1;
}

int ether_ll_recv(ether_t* ether, ether_frame_t *frame)
{
    dblk_new_from_stack(&frame->payload, NULL, 0);

    frame->data_size = ether->recv(ether->interface,
        frame->data, frame->data_size);
    if (frame->data_size > 0) {
        if (!ether_unpack(frame)) {
            return 0;
        }

        // dispatch
        if (frame->type == ETHER_TYPE_ARP) {
            arp_packet_t arp_pkt = {
                .data = frame->payload->data,
                .data_size = frame->payload->size,
            };
            ether_arp_ether_recv(ether, &arp_pkt);
        }
        if (frame->type == ETHER_TYPE_IP) {
            ip_t *ip = ether_arp_get_ip(ether);
            ip_datagram_t ip_datagram = { .raw_data = frame->payload, };
            ip_ll_recv(ip, &ip_datagram);
        }

        return 1;
    }
    return 0;
}

int ether_hl_send(ether_t* ether, ether_frame_t *frame)
{
    frame->src_mac_addr = ether->mac_address,
    ether_pack(frame);
    return ether->send(ether->interface, frame->data, frame->data_size);
}
