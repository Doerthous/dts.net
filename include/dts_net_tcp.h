/*
   The MIT License (MIT)

   Copyright (c) 2021 Doerthous

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

#ifndef DTS_NET_TCP_H_
#define DTS_NET_TCP_H_

#include <stdint.h>
#include <dts_net_ip.h>
#include <dts_net_pseudo_header.h>
#include <dts_net_dblk.h>
#include <dts_elib_timer.h>

typedef struct dts_net_tcp_segment
{
    struct dts_net_tcp_segment *next;

    dts_net_pseudo_header_t *psdhdr;

    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq;
    uint32_t ack;
    uint16_t data_offset;
    uint16_t flags;
    uint16_t win;
    uint16_t checksum;
    uint16_t urgent_ptr;
    dts_net_dblk_t *options;
    dts_net_dblk_t *payload;
    dts_net_dblk_t *raw_data;
} dts_net_tcp_segment_t;

typedef struct
{
    void *next; // list

    uint32_t passive:1;
    dts_net_ip_addr_t dest;
    uint16_t dest_port;
    dts_net_ip_t *localhost;
    uint32_t port;

    int state;

    void *rx_segs;
    void *tx_data;
    void *rx_data;
    dts_net_tcp_segment_t *tx_seg;

    struct {
        uint32_t una; // oldest unacknowledged sequence number
        uint32_t nxt; // next sequence number to be sent
        uint32_t iss; // initial send sequence number
    } snd;

    struct {
        // next sequence number expected on an incoming segments, and
        // is the left or lower edge of the receive window
        uint32_t nxt; 
        uint32_t irs; // initial receive sequence number
        uint16_t wnd;
    } rcv;

    dts_elib_timer_t tmr;
} dts_net_tcp_t;

// high level
dts_net_tcp_t *dts_net_tcp_open(dts_net_ip_addr_t *local, int port, 
    dts_net_ip_addr_t *dest, int dest_port);

void dts_net_tcp_close(dts_net_tcp_t *tcp);

size_t dts_net_tcp_recv(dts_net_tcp_t *tcp, uint8_t *data, size_t size);

size_t dts_net_tcp_send(dts_net_tcp_t *tcp, uint8_t *data, size_t size);

enum
{
    DTS_NET_TCP_STATE_CLOSED,
    DTS_NET_TCP_STATE_LISTEN,
    DTS_NET_TCP_STATE_SYN_SENT,
    DTS_NET_TCP_STATE_SYN_RECEIVED,
    DTS_NET_TCP_STATE_ESTABLISHED,
    DTS_NET_TCP_STATE_FIN_WAIT_1,
    DTS_NET_TCP_STATE_FIN_WAIT_2,
    DTS_NET_TCP_STATE_TIME_WAIT,
	DTS_NET_TCP_STATE_CLOSE_WAIT,
	DTS_NET_TCP_STATE_LAST_ACK,
    DTS_NET_TCP_STATE_CLOSING,
};
int dts_net_tcp_status(dts_net_tcp_t *tcp);

void dts_net_tcp_loop(dts_net_tcp_t *tcp);

// ll interfaces
void dts_net_tcp_ip_recv(dts_net_ip_t *ip, dts_net_ip_datagram_t *dg);

#endif // DTS_NET_TCP_H_
