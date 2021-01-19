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

#include <dts/net/tcp.h>
#include <dts/net/ip.h>
#include <dts/net/dblk.h>
#include <dts/net/mem.h>
#include <dts/net/endian.h>
#include <dts/net/sys.h>
#include <dts/net/pseudo_header.h>
#include <dts/net/checksum.h>
#include <dts/embedded/lib/timer.h>
#include <dts/datastruct/list.h>
#include <stdlib.h>

#define SEG_FLAG_URG (1<<5)
#define SEG_FLAG_ACK (1<<4)
#define SEG_FLAG_PSH (1<<3)
#define SEG_FLAG_RST (1<<2)
#define SEG_FLAG_SYN (1<<1)
#define SEG_FLAG_FIN (1<<0)

#define SEG_CTRL_A(pseg) \
    ((pseg)->flags&SEG_FLAG_ACK)
#define SEG_CTRL_S(pseg) \
    ((pseg)->flags&SEG_FLAG_SYN)
#define SEG_CTRL_R(pseg) \
    ((pseg)->flags&SEG_FLAG_RST)
#define SEG_CTRL_U(pseg) \
    ((pseg)->flags&SEG_FLAG_URG)
#define SEG_CTRL_P(pseg) \
    ((pseg)->flags&SEG_FLAG_PSH)
#define SEG_CTRL_F(pseg) \
    ((pseg)->flags&SEG_FLAG_FIN)

#define SEG_CTRL_AS(pseg) \
    (((pseg)->flags&SEG_FLAG_SYN) && SEG_CTRL_A(pseg))

#define SEG_CTRL_AF(pseg) \
    (((pseg)->flags&SEG_FLAG_FIN) && SEG_CTRL_A(pseg))

//
static int tcp_pack(tcp_segment_t *seg)
{
    pseudo_header_t * psdhdr = seg->psdhdr;
    dblk_t *header = seg->raw_data;
    dblk_t *options = seg->options;
    dblk_t *payload = seg->payload;
    if (dblk_size(seg->raw_data) > (20 + dblk_size(options))) {
        bw16(seg->src_port, header->data);
        bw16(seg->dest_port, header->data+2);
        bw32(seg->seq, header->data+4);
        bw32(seg->ack, header->data+8);
        bw16((seg->data_offset<<12)|(seg->flags), header->data+12);
        bw16(seg->win, header->data+14);
        bw16(0, header->data+16); // clear checksum
        bw16(seg->urgent_ptr, header->data+18);
        //memcpy(header->data+20, seg->options, seg->option_size);
        header->size = 20;

        // do pseudo header
        psdhdr->length = header->size + dblk_size(options) + dblk_size(payload);
        pseudo_header_pack(psdhdr);
    
        // do checksume
        uint64_t sum = 0;
        sum = checksum_accmulate(sum, psdhdr->data, psdhdr->data_size);
        sum = checksum_accmulate(sum, header->data, header->size);
        if (options) {
            sum = checksum_accmulate(sum, options->data, options->size);
        }
        if (payload) {
            sum = checksum_accmulate(sum, payload->data, payload->size);
        }
        seg->checksum = checksum_complete(sum);
        bw16(seg->checksum, header->data+16); // checksum

        dblk_merge(header, options);
        dblk_merge(header, payload);
        return 1;
    }
    return 0;
}
static int tcp_unpack(tcp_segment_t *seg)
{
    pseudo_header_t * psdhdr = seg->psdhdr;
    dblk_t *header = seg->raw_data;
    dblk_t *options = seg->options;
    dblk_t *payload = seg->payload;

    if (dblk_size(seg->raw_data) >= 20) {
        seg->src_port = br16(header->data);
        seg->dest_port = br16(header->data+2);
        seg->seq = br32(header->data+4);
        seg->ack = br32(header->data+8);
        seg->data_offset = ((br16(header->data+12) & 0xF000)>>12);
        seg->flags = (br16(header->data+12) & 0x0FFF);
        seg->win = br16(header->data+14);
        seg->checksum = br16(header->data+16);
        seg->urgent_ptr = br16(header->data+18);
        dblk_node_init(options, header->data+20, seg->data_offset*4-20);
        int doff = seg->data_offset*4;
        dblk_node_init(payload, header->data+doff, header->size-doff);

        psdhdr->length = dblk_size(seg->raw_data);
        bw16(0, seg->raw_data->data+16); // clear checksum

        pseudo_header_pack(psdhdr);

        uint64_t sum = 0;
        sum = checksum_accmulate(sum, psdhdr->data, psdhdr->data_size);
        sum = checksum_accmulate(sum, seg->raw_data->data, seg->raw_data->size);
        return seg->checksum == checksum_complete(sum);
    }
    return 0;
}
tcp_segment_t *tcp_segment_init(tcp_segment_t *seg)
{
    if (seg) {
        memset(seg, 0, sizeof(tcp_segment_t));
    }
    return seg;
}
static tcp_segment_t *mem_alloc_tcp_segment(void)
{
    return tcp_segment_init((tcp_segment_t *)malloc(sizeof(tcp_segment_t)));
}
static void mem_free_tcp_segment(tcp_segment_t *seg)
{
    free(seg);
}


// op:
#define SN_LS 0 // <
#define SN_LE 1 // <=
static int sncmp(uint32_t min, int op1, uint32_t x, int op2, uint32_t max)
{
    // min <= x <= max, min = max, x = max
    if (min == max) {
        if ((x == max) && (op1 == op2) && (op1 == SN_LE)) {
            return 1;
        }
    }
    else if (min < max) {
        // min < x < max
        if (op1 == SN_LS && op2 == SN_LS) {
            return (min < x) && (x < max);
        }
        // min <= x < max
        if (op1 == SN_LE && op2 == SN_LS) {
            return (min <= x) && (x < max);
        }
        // min < x <= max
        if (op1 == SN_LS && op2 == SN_LE) {
            return (min < x) && (x <= max);
        }
        // min <= x <= max
        if (op1 == SN_LE && op2 == SN_LE) {
            return (min <= x) && (x <= max);
        }
    }
    else {
        // min < x < max
        if (op1 == SN_LS && op2 == SN_LS) {
            return !((max <= x) && (x <= min));
        }
        // min <= x < max
        if (op1 == SN_LE && op2 == SN_LS) {
            return (max <= x) && (x < min);
        }
        // min < x <= max
        if (op1 == SN_LS && op2 == SN_LE) {
            return (max < x) && (x <= min);
        }
        // min <= x <= max
        if (op1 == SN_LE && op2 == SN_LE) {
            return (max < x) && (x < min);
        }
    }
    return 0; // this will no happens.
}

enum
{
    TCP_STATE_CLOSED,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_TIME_WAIT,
	TCP_STATE_CLOSE_WAIT,
	TCP_STATE_LAST_ACK,
    TCP_STATE_CLOSING,
    TCP_STATE_COUNT,
};

//

uint32_t tcp_generate_isn(void)
{
    return sys_tick_s();
}

void tcp_loop(tcp_t *tcp)
{
    if (timer_expired(&tcp->tmr)) {
        timer_stop(&tcp->tmr);
		void tcp_timeout(void *);
        tcp_timeout(tcp);
    }
}

// interface for user
static int tcp_free_port(void)
{
    return 0x1234;
}

//
static list_t tcp_list;
void tcp_ip_recv(ip_t *ip, ip_datagram_t *dg) 
{
    //unpack segment
    tcp_segment_t seg;
    seg.raw_data = dg->payload;
    pseudo_header_new_from_stack(&seg.psdhdr, 
        &dg->header.src, &dg->header.dest, 
        dg->header.protocol, 0);
    dblk_node_new_from_stack(&seg.payload, NULL, 0);
    dblk_node_new_from_stack(&seg.options, NULL, 0);
    if (tcp_unpack(&seg)) {
        // find which tcp
        list_foreach(tcp_t, tcp, &tcp_list, {
            if ((*tcp)->localhost == ip && (*tcp)->port == seg.dest_port) {
                list_enqueue((list_t *)&((*tcp)->rx_segs), &seg);
                void tcp_received(tcp_t *);
                tcp_received((*tcp));
                break;
            }
        });
    }
    return;
}
static int tcp_ip_send(tcp_t *tcp, tcp_segment_t *seg)
{
    ip_datagram_t dg;

    dg.header.options = NULL;
    dg.header.option_size = 0;
    dg.header.type_of_service = 0;
    dg.header.protocol = IP_PROTOCOL_TCP;
    dg.header.time_to_live = 0b00111100;
    dg.header.dest = tcp->dest;
    dg.payload = dblk_copy(seg->raw_data);
    if (!ip_hl_send(tcp->localhost, &dg)) {
        dblk_delete(dg.payload);
        return 0;
    }
    return 1;
}


//
static int tcp_send_ctrl_seg(tcp_t *tcp, uint32_t seq, uint32_t ack, uint16_t flags)
{
    tcp_segment_t seg;
    tcp_segment_init(&seg);
    seg.src_port = tcp->port;
    seg.dest_port = tcp->dest_port;
    seg.seq = seq;
    seg.ack = ack;
    seg.data_offset = 5;
    seg.flags = flags;
    seg.win = tcp->rcv.wnd;
    dblk_node_new_from_stack_with_buff(&seg.raw_data, 32);
    pseudo_header_new_from_stack(&seg.psdhdr, 
        &tcp->localhost->addr, &tcp->dest, 
        IP_PROTOCOL_TCP, 0);
    tcp_pack(&seg);
    return tcp_ip_send(tcp, &seg);
}
static void tcp_retransmit(tcp_t *tcp)
{
    tcp_segment_t *seg;
    if (tcp->tx_seg) { // if tcp has data to send
        seg = tcp->tx_seg;
    }
    else if (tcp->tx_fin) { // else send fin
        seg = tcp->tx_fin;
    }

    if (seg) {
        tcp_ip_send(tcp, seg); // send data
    }
    //sys_timer_start(&tcp->ctmr, 5);
    timer_start(&tcp->tmr, 5);
}
static void tcp_tx_seg_complete(tcp_t *tcp, tcp_segment_t **seg)
{
    if (*seg) {
        dblk_list_delete((*seg)->raw_data);
        mem_free_tcp_segment((*seg));
        (*seg) = NULL;
        timer_stop(&tcp->tmr);
    }
}


static void tcp_construct_data(tcp_t *tcp)
{
    if (tcp->tx_seg == NULL) { 
        if (list_length((list_t*)&tcp->tx_data)>0) {
            dblk_t *tx = list_dequeue((list_t*)&tcp->tx_data);
            tcp->tx_seg = mem_alloc_tcp_segment();
            if (tcp->tx_seg) {
                tcp->tx_seg->src_port = tcp->port;
                tcp->tx_seg->dest_port = tcp->dest_port;
                tcp->tx_seg->seq = tcp->snd.una;
                tcp->tx_seg->ack = tcp->rcv.nxt;
                tcp->tx_seg->data_offset = 5; // 20B
                tcp->tx_seg->flags |= SEG_FLAG_ACK;
                tcp->tx_seg->win = tcp->rcv.wnd;
                tcp->tx_seg->raw_data = dblk_node_new_with_buff(32);
                tcp->tx_seg->payload = tx;
                pseudo_header_new_from_stack(&tcp->tx_seg->psdhdr, 
                    &tcp->localhost->addr, &tcp->dest, 
                    IP_PROTOCOL_TCP, 0);
                tcp_pack(tcp->tx_seg);
                tcp_ip_send(tcp, tcp->tx_seg);
                timer_start(&tcp->tmr, 3);

                tcp->snd.nxt += dblk_size(tx);
            }
        }
    }
}
// when receive ack of tx data, call this to update tcb and send new data.
static int tcp_handle_ack_of_data(tcp_t *tcp, tcp_segment_t *ack)
{
    if (tcp && tcp->tx_seg) {
        if (sncmp(tcp->snd.una, SN_LS, ack->ack, SN_LE, tcp->snd.nxt)) {
            if (ack->ack == tcp->snd.nxt) {
                tcp->snd.una = ack->ack;
                // only when entire segment has acked 
                tcp_tx_seg_complete(tcp, &tcp->tx_seg);
                tcp_construct_data(tcp);
                return 1;
            }
        }
    }
    return 0;
}
static int tcp_handle_data(tcp_t *tcp, tcp_segment_t *dat)
{
    if (dat->seq == tcp->rcv.nxt) {
        int seg_len = dblk_size(dat->payload);
        if (seg_len > 0) {
            seg_len = seg_len > tcp->rcv.wnd ? tcp->rcv.wnd : seg_len;
            dblk_t *rx = dblk_node_new_with_buff(seg_len);
            if (rx) {
                list_enqueue((list_t*)&tcp->rx_data, rx);
                tcp->rcv.nxt += seg_len;
                // if there is a seg in the transmitting queue
                // update it's ack
                if (tcp->tx_seg) {
                    tcp->tx_seg->ack = tcp->rcv.nxt;
                }
                else { // construct an ack to send.
                    tcp_send_ctrl_seg(tcp, tcp->snd.una, tcp->rcv.nxt, 
                        SEG_FLAG_ACK);
                }
                return 1;
            }
        }
    }
    return 0;
}

static void tcp_contruct_fin_ack(tcp_t *tcp)
{
    // if data transmission complete
    if ((dblk_size(tcp->tx_data) == 0) && (tcp->tx_seg == NULL)) {
        if (tcp->tx_fin == NULL) {
            tcp->tx_fin = mem_alloc_tcp_segment();
            if (tcp->tx_fin) {
                tcp->tx_fin->src_port = tcp->port;
                tcp->tx_fin->dest_port = tcp->dest_port;
                tcp->tx_fin->seq = tcp->snd.una;
                tcp->tx_fin->ack = tcp->rcv.nxt;
                tcp->tx_fin->data_offset = 5; // 20B
                tcp->tx_fin->flags |= (SEG_FLAG_FIN|SEG_FLAG_ACK);
                tcp->tx_fin->win = tcp->rcv.wnd;
                tcp->tx_fin->raw_data = dblk_node_new_with_buff(32);
                pseudo_header_new_from_stack(&tcp->tx_fin->psdhdr, 
                    &tcp->localhost->addr, &tcp->dest, 
                    IP_PROTOCOL_TCP, 0);
                tcp_pack(tcp->tx_fin);
                //sys_timer_start(&tcp->ctmr, 1);
                timer_start(&tcp->tmr, 1);

                tcp->snd.nxt += 1;
            }
        }
    }
}
// check whether the ack is correct or not.
// receive <ACK> of <FIN,ACK>
static int tcp_handle_ack_of_fin_ack(tcp_t *tcp, tcp_segment_t *ack)
{
    if (tcp && tcp->tx_fin) {
        if (tcp->tx_fin->seq+1 == ack->ack) {
            tcp_tx_seg_complete(tcp, &tcp->tx_fin);
            return 1;
        }
    }
    return 0;
}

// receive <FIN,ACK>
static int tcp_handle_fin_ack(tcp_t *tcp, tcp_segment_t *seg)
{
    if (sncmp(tcp->snd.una, SN_LS, seg->ack, SN_LE, tcp->snd.nxt)) {
        // update tcb vars
        tcp->snd.una = seg->ack;
        tcp->rcv.nxt = seg->seq+1;

        // ack back
        return tcp_send_ctrl_seg(tcp, tcp->snd.una, tcp->rcv.nxt, SEG_FLAG_ACK);
    }
	return 0;
}

// EVENT OPEN
void open_when_closed(tcp_t *tcp)
{
    if (tcp->passive) {
        tcp->state = TCP_STATE_LISTEN;
    }
    else {
        /// init tcb vars
        tcp->snd.una = tcp->snd.iss = tcp_generate_isn();
        tcp->snd.nxt = tcp->snd.iss+1;

        // send syn
        tcp->tx_seg = mem_alloc_tcp_segment();
        tcp->tx_seg->src_port = tcp->port;
        tcp->tx_seg->dest_port = tcp->dest_port;
        tcp->tx_seg->seq = tcp->snd.una;
        tcp->tx_seg->data_offset = 5; // 20B
        tcp->tx_seg->flags |= SEG_FLAG_SYN; // tcp->tx_seg->flags.syn = 1;
        tcp->tx_seg->win = tcp->rcv.wnd; // tcp->tx_seg->seq+64;
        tcp->tx_seg->raw_data = dblk_node_new_with_buff(32);
        pseudo_header_new_from_stack(&tcp->tx_seg->psdhdr, 
            &tcp->localhost->addr, &tcp->dest, IP_PROTOCOL_TCP, 0);
        tcp_pack(tcp->tx_seg);
        tcp_ip_send(tcp, tcp->tx_seg);
        tcp->state = TCP_STATE_SYN_SENT;
        //sys_timer_start(&tcp->ctmr, 3);
        timer_start(&tcp->tmr, 3);
    }
}
static void (*open_event_process[TCP_STATE_COUNT])(tcp_t *t) = 
{
    open_when_closed,
};
tcp_t *tcp_open(ip_addr_t *local, int port, ip_addr_t *dest, int dest_port)
{
    tcp_t *tcp = NULL;
    ip_t *ip;

    ip = ip_find_netif(local); 
    list_foreach(tcp_t, t, &tcp_list, {
        if ((*t)->localhost == ip && (*t)->port == port) {
            return NULL;
        }
    });

    if (ip) {
        tcp = (tcp_t *)malloc(sizeof(tcp_t));
        if (tcp) {
            memset(tcp, 0, sizeof(tcp_t));
            tcp->localhost = ip;
            tcp->port = tcp_free_port();
            tcp->dest = *dest;
            tcp->dest_port = dest_port;
            tcp->rcv.wnd = 1500;
            tcp->state = TCP_STATE_CLOSED;
            // void tcp_timeout(void *x);
            // tcp->ctmr.callback = tcp_timeout;
            // tcp->ctmr.data = tcp;
            timer_init(&tcp->tmr, sys_tick_s);
            list_enqueue(&tcp_list, tcp);
            open_event_process[tcp->state](tcp);
        }
    }

    return tcp;
}

// EVENT TIMEOUT
void timeout_when_syn_sent(tcp_t *tcp) 
{
    tcp_retransmit(tcp);
}
void timeout_when_fin_wait_1(tcp_t *tcp)
{
    tcp_retransmit(tcp);
}
void timeout_when_time_wait(tcp_t *tcp)
{
    tcp->state = TCP_STATE_CLOSED;
}
void timeout_when_last_ack(tcp_t *tcp)
{
    tcp_retransmit(tcp);
}
static void (*timeout_event_process[TCP_STATE_COUNT])(tcp_t *t) = 
{
    [TCP_STATE_SYN_SENT] = timeout_when_syn_sent,
    [TCP_STATE_FIN_WAIT_1] = timeout_when_fin_wait_1,
    [TCP_STATE_TIME_WAIT] = timeout_when_time_wait,
    [TCP_STATE_LAST_ACK] = timeout_when_last_ack,
    [TCP_STATE_ESTABLISHED] = tcp_retransmit,
};
void tcp_timeout(void *tcp)
{
    if (timeout_event_process[((tcp_t *)(tcp))->state]) {
        timeout_event_process[((tcp_t *)(tcp))->state](tcp);
    }
}

// EVENT SEGMENT ARRIVES
void received_when_syn_sent(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        if (SEG_CTRL_A(seg)) { 
            // If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset (unless
            // the RST bit is set, if so drop the segment and return)
            if (!sncmp(tcp->snd.iss, SN_LS, seg->ack, SN_LE, tcp->snd.nxt)) {
                if (!SEG_CTRL_R(seg)) {
                    // send a reset
                    tcp_send_ctrl_seg(tcp, seg->ack, seg->seq, SEG_FLAG_RST);
                    return;
                }
            }
            // If SND.UNA =< SEG.ACK =< SND.NXT then the ACK is acceptable.
            if (sncmp(tcp->snd.una, SN_LE, seg->ack, SN_LE, tcp->snd.nxt)) {
                if (SEG_CTRL_R(seg)) {
                    tcp->state = TCP_STATE_CLOSED;
                    return;
                }
                if (SEG_CTRL_S(seg)) {
                    // update tcb vars
                    tcp->rcv.nxt = seg->seq+1;
                    tcp->rcv.irs = seg->seq;
                    tcp->snd.una = seg->ack;
                    // TODO: if iss == 0xFFFFFFFF !!!
                    if (tcp->snd.una > tcp->snd.iss) { 
                        if (tcp_send_ctrl_seg(tcp, // ack back
                            tcp->snd.una, tcp->rcv.nxt, SEG_FLAG_ACK)) {
                            // to TCP_STATE_ESTABLISHED
                            tcp->state = TCP_STATE_ESTABLISHED;
                            // construct from open_when_closed
                            tcp_tx_seg_complete(tcp, &tcp->tx_seg);
                        }
                    }
                }
            }
        }
        
    }
}
void received_when_established(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (!seg) {
        return;
    }
	size_t seg_len = dblk_size(seg->payload);

    /* 1. check sequence number
    */
    /*
    If the RCV.WND is zero, no segments will be acceptable, but
    special allowance should be made to accept valid ACKs, URGs and
    RSTs.
    */
    if (tcp->rcv.wnd == 0) {
        if (dblk_size(seg->payload) == 0) {
            if (seg->seq == tcp->rcv.nxt) {
                if (SEG_CTRL_A(seg)) {
                    // TODO
                }
                if (SEG_CTRL_R(seg)) {
                    // TODO
                }
                if (SEG_CTRL_U(seg)) {
                    // TODO
                }
            }
        }
        else {
            goto seg_unacceptable;
        }
        return;
    }

    if (seg_len == 0) {
        // RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
        if (!sncmp(tcp->rcv.nxt, SN_LE, seg->seq, 
            SN_LS, tcp->rcv.nxt+tcp->rcv.wnd)) {
            goto seg_unacceptable;
        }
    }
    else {
        // RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
        if (!sncmp(tcp->rcv.nxt, SN_LE, seg->seq, 
            SN_LS, tcp->rcv.nxt+tcp->rcv.wnd)) {
            if (!sncmp(tcp->rcv.nxt, SN_LE, seg->seq+seg_len-1, 
                SN_LS, tcp->rcv.nxt+tcp->rcv.wnd)) {
                goto seg_unacceptable;
            }
        }
    }

    /* 2. check the RST bit
    If the RST bit is set then, any outstanding RECEIVEs and SEND
    should receive "reset" responses.  All segment queues should be
    flushed.  Users should also receive an unsolicited general
    "connection reset" signal.  Enter the CLOSED state, delete the
    TCB, and return.
    */
    if (SEG_CTRL_R(seg)) {
        // TODO

        return;
    }

    /* 4. check the SYN bit
    If the SYN is in the window it is an error, send a reset, any
    outstanding RECEIVEs and SEND should receive "reset" responses,
    all segment queues should be flushed, the user should also
    receive an unsolicited general "connection reset" signal, enter
    the CLOSED state, delete the TCB, and return.
    */
    if (SEG_CTRL_S(seg)) {

    }

    /* 5. check the ACK field
    */
    if (SEG_CTRL_A(seg)) {
        if (tcp->tx_seg) {
            tcp_handle_ack_of_data(tcp, seg); // normal data transmission ack
        }
    }
    else {
        goto drop_seg;
    }

    /* 7. process the segment text
    */
    tcp_handle_data(tcp, seg);

    /* 8. check the FIN bit
    If the FIN bit is set, signal the user "connection closing" and
    return any pending RECEIVEs with same message, advance RCV.NXT
    over the FIN, and send an acknowledgment for the FIN.  Note that
    FIN implies PUSH for any segment text not yet delivered to the
    user.

    Enter the CLOSE-WAIT state.
    */
    if (SEG_CTRL_F(seg)) {
        tcp_handle_fin_ack(tcp, seg);
        tcp->state = TCP_STATE_CLOSE_WAIT;
    }

drop_seg:
    return;

seg_unacceptable:
    /*
    If an incoming segment is not acceptable, an acknowledgment
    should be sent in reply (unless the RST bit is set, if so drop
    the segment and return):

        <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
    
    After sending the acknowledgment, drop the unacceptable segment
    and return.
    */
    if (!SEG_CTRL_R(seg)) {
        tcp_send_ctrl_seg(tcp, 
            tcp->snd.nxt, tcp->rcv.nxt, SEG_FLAG_ACK);
    }
}
void received_when_fin_wait_1(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        // TODO:
        /*
        if (SEG_CTRL_AF(seg)) {
            // ack
            tcp->state = TCP_STATE_CLOSING;
        }
        */
        if (SEG_CTRL_A(seg)) {
            if (tcp->tx_seg) {
                tcp_handle_ack_of_data(tcp, seg); // normal data transmission ack
            }
            else {
                if (tcp_handle_ack_of_fin_ack(tcp, seg)) {
                    tcp->state = TCP_STATE_FIN_WAIT_2;
                }
            }
        }
    }
    if (tcp->state == TCP_STATE_FIN_WAIT_1) {
        tcp_contruct_fin_ack(tcp);
    }
}
void received_when_fin_wait_2(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        if (SEG_CTRL_AF(seg)) {
            // if ok
            tcp->state = TCP_STATE_TIME_WAIT;
            // start timer 2min
            //sys_timer_start(&tcp->ctmr, 120);
            timer_start(&tcp->tmr, 120);
            tcp_handle_fin_ack(tcp, seg);
        }
    }
}
void received_when_time_wait(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        if (SEG_CTRL_AF(seg)) {
            tcp_handle_fin_ack(tcp, seg);
        }
    }
}
void received_when_close_wait(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        if (SEG_CTRL_AF(seg)) {
            tcp_handle_fin_ack(tcp, seg);
        }
    }
}
void received_when_last_ack(tcp_t *tcp)
{
    tcp_segment_t *seg = list_dequeue((list_t *)&tcp->rx_segs);
    if (seg) {
        if (SEG_CTRL_A(seg)) {
            if (tcp->tx_seg) {
                tcp_handle_ack_of_data(tcp, seg); // normal data transmission ack
            }
            else {
                if (tcp_handle_ack_of_fin_ack(tcp, seg)) {
                    tcp->state = TCP_STATE_CLOSED;
                }
            }
        }
    }
    if (tcp->state == TCP_STATE_LAST_ACK) {
        tcp_contruct_fin_ack(tcp);
    }
}
static void (*received_event_process[TCP_STATE_COUNT])(tcp_t *t) = 
{
    [TCP_STATE_SYN_SENT] = received_when_syn_sent,
    [TCP_STATE_ESTABLISHED] = received_when_established,
    [TCP_STATE_FIN_WAIT_1] = received_when_fin_wait_1,
    [TCP_STATE_FIN_WAIT_2] = received_when_fin_wait_2,
    [TCP_STATE_TIME_WAIT] = received_when_time_wait,
    [TCP_STATE_CLOSE_WAIT] = received_when_close_wait,
    [TCP_STATE_LAST_ACK] = received_when_last_ack,
};
void tcp_received(tcp_t *tcp)
{
    if (received_event_process[tcp->state]) {
        received_event_process[tcp->state](tcp);
    }
}

// EVENT CLOSE
void close_when_syn_sent(tcp_t *tcp)
{
    tcp->state = TCP_STATE_CLOSED;
    // TODO: lock?
    tcp_tx_seg_complete(tcp, &tcp->tx_seg);
}
void close_when_established(tcp_t *tcp) 
{                
    tcp_contruct_fin_ack(tcp);
    tcp->state = TCP_STATE_FIN_WAIT_1;
}
void close_when_close_wait(tcp_t *tcp) 
{
    tcp_contruct_fin_ack(tcp);         
    tcp->state = TCP_STATE_LAST_ACK;
}
static void (*close_event_process[TCP_STATE_COUNT])(tcp_t *t) = 
{
    [TCP_STATE_ESTABLISHED] = close_when_established,
    [TCP_STATE_CLOSE_WAIT] = close_when_close_wait,
};
void tcp_close(tcp_t *tcp)
{
    list_foreach(tcp_t, t, &tcp_list, {
        if ((*t) == tcp) {
            if (close_event_process[((tcp_t *)(tcp))->state]) {
                close_event_process[((tcp_t *)(tcp))->state](tcp);
            }
            if (tcp->state == TCP_STATE_CLOSED) {
                list_foreach_remove(t);
                free(tcp);
                break;
            }
        }
    });
}

// EVENT RECEIVE
size_t recv_when_established(tcp_t *tcp, uint8_t *data, size_t size) 
{
    while (list_length((list_t*)&tcp->rx_data)) {
        dblk_t *rx = (dblk_t*)list_dequeue((list_t*)&tcp->rx_data);
        size_t rx_sz = dblk_size(rx);
        if (rx_sz <= size) {
            dblk_copy_to(rx, data, rx_sz);
            return rx_sz;
        }
        else {
            dblk_copy_to(rx, data, size); 
            // TODO: buff insufficient!!!
            return size;
        }
    }
    return 0;
}
static size_t (*recv_event_process[TCP_STATE_COUNT])(tcp_t *, uint8_t *, size_t) = 
{
    [TCP_STATE_ESTABLISHED] = recv_when_established,
};
size_t tcp_recv(tcp_t *tcp, uint8_t *data, size_t size)
{
    if (recv_event_process[((tcp_t *)(tcp))->state]) {
        return recv_event_process[((tcp_t *)(tcp))->state](tcp, data, size);
    }
    return 0;
}

// EVENT SEND
size_t send_when_established(tcp_t *tcp, uint8_t *data, size_t size) 
{
    dblk_t *tx = dblk_node_new_with_buff(size);
    if (tx) {
        dblk_copy_from(tx, data, size);
        list_enqueue((list_t*)&tcp->tx_data, tx);
    }
    else {
        size = 0;
    }
    tcp_construct_data(tcp);
    return size;
}
static size_t (*send_event_process[TCP_STATE_COUNT])(tcp_t *, uint8_t *, size_t) = 
{
    [TCP_STATE_ESTABLISHED] = send_when_established,
};
size_t tcp_send(tcp_t *tcp, uint8_t *data, size_t size)
{
    if (send_event_process[((tcp_t *)(tcp))->state]) {
        return send_event_process[((tcp_t *)(tcp))->state](tcp, data, size);
    }
    return 0;
}


int tcp_status(tcp_t *tcp)
{
    if (tcp) {
        return tcp->state;
    }
    return TCP_STATE_CLOSED;
}
