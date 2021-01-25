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

#include <dts/net/dhcp.h>

#include <bsd/socket.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>

#define DHCP_MESSAGE_FLAG_BROADCAST (1<<0)

#define DHCP_OPT_MAGIC 99, 130, 83, 99 // start

#define DHCP_OPT_REQUEST_IP_ADDR_TAG 50
#define DHCP_OPT_REQUEST_IP_ADDR(ip) \
    DHCP_OPT_REQUEST_IP_ADDR_TAG, 4, (ip)[0], (ip)[1], (ip)[2], (ip)[3]

#define DHCP_OPT_IP_LEASE_TIME_TAG 51

#define DHCP_OPT_DHCP_MSG_TYPE_TAG 53
# define DHCP_MSG_TYPE_DISCOVER 1
# define DHCP_MSG_TYPE_OFFER 2
# define DHCP_MSG_TYPE_REQUEST 3
# define DHCP_MSG_TYPE_DECLINE 4
# define DHCP_MSG_TYPE_ACK 5
# define DHCP_MSG_TYPE_NAK 6
# define DHCP_MSG_TYPE_RELEASE 7
#define DHCP_OPT_DHCP_MSG_TYPE(type) DHCP_OPT_DHCP_MSG_TYPE_TAG, 1, type

#define DHCP_OPT_SERVER_IDENTIFIER_TAG 54
#define DHCP_OPT_SERVER_IDENTIFIER(id) \
    DHCP_OPT_SERVER_IDENTIFIER_TAG, 4, (id)[0], (id)[1], (id)[2], (id)[3]

#define DHCP_OPT_PAD 0
#define DHCP_OPT_END 255

enum
{
    DHCP_C_STATE_INIT_REBOOT,
    DHCP_C_STATE_REBOOTING,
    DHCP_C_STATE_INIT,
    DHCP_C_STATE_SELECTING,
    DHCP_C_STATE_REQUESTING,
    DHCP_C_STATE_BOUND,
    DHCP_C_STATE_RENEWING,
    DHCP_C_STATE_REBINDING,
    DHCP_C_STATE_COUNT,
};


#include <dts/embedded/lib/timer.h>
#include <dts/net/sys.h>
#include <dts/net/mem.h>
typedef struct
{
    int state;
    int rx_sk;
    int tx_sk;

    int udp_setup:1;

    dhcp_message_t msg;

    uint8_t ethaddr[6];

    timer_t T1;
    timer_t T2;

    void (*set_ip)(uint8_t *ip, size_t len);
    
    size_t start_time;
    uint32_t lease;
} dts_net_dhcp_t;
static dts_net_dhcp_t only;

int dhcp_message_pack(dhcp_message_t *msg)
{
    uint8_t *ptr = msg->raw_data;

    if (236 + msg->option_size > msg->raw_data_size) {
        return 0;
    }

    *ptr++ = msg->op;
    *ptr++ = msg->htype;
    *ptr++ = msg->hlen;
    *ptr++ = msg->hops;
    *((uint32_t *)ptr) = htonl(msg->xid); ptr+=4;
    *((uint16_t *)ptr) = htons(msg->secs); ptr+=2;
    *((uint16_t *)ptr) = htons(msg->flags); ptr+=2;
    *((uint32_t *)ptr) = htonl(msg->ciaddr); ptr+=4;
    *((uint32_t *)ptr) = htonl(msg->yiaddr); ptr+=4;
    *((uint32_t *)ptr) = htonl(msg->siaddr); ptr+=4;
    *((uint32_t *)ptr) = htonl(msg->giaddr); ptr+=4;
    memcpy(ptr, msg->chaddr, 16); ptr+=16;
    memcpy(ptr, msg->sname, 64); ptr+=64;
    memcpy(ptr, msg->file, 128); ptr+=128;
    memcpy(ptr, msg->option, msg->option_size); ptr+=msg->option_size;

    msg->raw_data_size = ptr-msg->raw_data;
    
    return 1;
}

int dhcp_message_unpack(dhcp_message_t *msg)
{
    uint8_t *ptr = msg->raw_data;

    if (msg->raw_data_size < 241) {
        return 0;
    }

    msg->op = *ptr++;
    msg->htype = *ptr++;
    msg->hlen = *ptr++;
    msg->hops = *ptr++;
    msg->xid = ntohl(*((uint32_t *)ptr)); ptr+=4;
    msg->secs = ntohl(*((uint16_t *)ptr)); ptr+=2;
    msg->flags = ntohl(*((uint16_t *)ptr)); ptr+=2;
    msg->ciaddr = ntohl(*((uint32_t *)ptr)); ptr+=4;
    msg->yiaddr = ntohl(*((uint32_t *)ptr)); ptr+=4;
    msg->siaddr = ntohl(*((uint32_t *)ptr)); ptr+=4;
    msg->giaddr = ntohl(*((uint32_t *)ptr)); ptr+=4;
    memcpy(msg->chaddr, ptr, 16); ptr+=16;
    memcpy(msg->sname, ptr, 64); ptr+=64;
    memcpy(msg->file, ptr, 128); ptr+=128;
    msg->option = ptr;
    msg->option_size = msg->raw_data_size-(ptr-msg->raw_data);

    return 1;
}

dhcp_message_t *dhcp_message_init(dhcp_message_t *msg)
{
    memset(msg, 0, sizeof(dhcp_message_t));
    return msg;
}

int dhcp_option_message_type(dhcp_message_t *msg)
{
    uint8_t *ptr = msg->option;
    uint8_t magic[] = {DHCP_OPT_MAGIC};
    if (*((uint32_t *)ptr) == *((uint32_t *)magic)) {
        ptr += 4;
        while (ptr-msg->option < msg->option_size) {
            if (*ptr == DHCP_OPT_DHCP_MSG_TYPE_TAG) {
                return ptr[2];
            }
            if (*ptr == DHCP_OPT_PAD) {
                ++ptr;
            }
            if (*ptr == DHCP_OPT_END) {
                break;
            }
            ptr += (2+ptr[1]);
        }
    }
    
    return -1;
}

static int dhcp_option_long(dhcp_message_t *msg, uint32_t *l, int code)
{
    uint8_t *ptr = msg->option;
    uint8_t magic[] = {DHCP_OPT_MAGIC};
    if (*((uint32_t *)ptr) == *((uint32_t *)magic)) {
        ptr += 4;
        while (ptr-msg->option < msg->option_size) {
            if (*ptr == code) {
                *l = ntohl(*((uint32_t *)(ptr+2)));
                return 1;
            }
            if (*ptr == DHCP_OPT_PAD) {
                ++ptr;
            }
            if (*ptr == DHCP_OPT_END) {
                break;
            }
            ptr += (2+ptr[1]);
        }
    }
    
    return 0;
}
int dhcp_option_ip_lease_time(dhcp_message_t *msg, uint32_t *lease)
{
    return dhcp_option_long(msg, lease, DHCP_OPT_IP_LEASE_TIME_TAG);
}
int dhcp_option_server_identifier(dhcp_message_t *msg, uint32_t *id)
{
    return dhcp_option_long(msg, id, DHCP_OPT_SERVER_IDENTIFIER_TAG);
}

//
int dhcp_client_setup(const char *ip, const char *eth, 
    void (*set_ip)(uint8_t *ip, size_t len))
{
    struct sockaddr_in rx_addr;
    int sk;

    sk = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family = AF_INET;
    rx_addr.sin_addr.s_addr = inet_addr(ip);
    rx_addr.sin_port = htons(68);
    if (bind(sk, (struct sockaddr*)&rx_addr, sizeof(rx_addr)) != 0) {
        close(sk);
        return 0;
    }
    only.rx_sk = sk;

    only.tx_sk = socket(AF_INET, SOCK_DGRAM, 0);

    memcpy(only.ethaddr, eth, 6);

    timer_init(&only.T1, sys_tick_s);
    timer_init(&only.T2, sys_tick_s);
    
    only.start_time = sys_tick_s();
    only.set_ip = set_ip;
    only.udp_setup = 1;
    only.state = DHCP_C_STATE_INIT;
    
    return 1;
}

// Actions
static int dhcp_client_send_discover(void)
{
    size_t tx_size;
    struct sockaddr_in tx_addr;
    uint8_t options[] = {
        DHCP_OPT_MAGIC,
        DHCP_OPT_DHCP_MSG_TYPE(DHCP_MSG_TYPE_DISCOVER),
        DHCP_OPT_END
    };

    memset(&tx_addr, 0, sizeof(tx_addr));
    tx_addr.sin_family = AF_INET;
    tx_addr.sin_addr.s_addr = INADDR_ANY;
    tx_addr.sin_port = htons(67);

    dhcp_message_init(&only.msg);
    only.msg.op = BOOTREQUEST;
    only.msg.htype = DHCP_HTYPE_ETHERNET;
    only.msg.hlen = DHCP_HLEN_ETHERNET;
    only.msg.xid = rand();
    only.msg.secs = sys_tick_s()-only.start_time;
    only.msg.flags = DHCP_MESSAGE_FLAG_BROADCAST;
    memcpy(only.msg.chaddr, only.ethaddr, 6);
    only.msg.option = options;
    only.msg.option_size = 8;
    only.msg.raw_data = (uint8_t *)malloc(576);
    if (!only.msg.raw_data) {
        return 0;
    }
    only.msg.raw_data_size = 576;
    dhcp_message_pack(&only.msg);
    tx_size = sendto(only.tx_sk, 
        only.msg.raw_data, only.msg.raw_data_size, 0, 
        (struct sockaddr*)&tx_addr, sizeof(tx_addr));
    free(only.msg.raw_data);
    if (tx_size <= 0) {
        return 0;
    }

    return 1;
}

static int dhcp_client_send_request(void)
{
    struct sockaddr_in tx_addr;
    size_t tx_size;
    uint32_t sid, yip;
    
    if (!dhcp_option_server_identifier(&only.msg, &sid)) {
        return 0;
    }
    sid = htonl(sid);
    yip = htonl(only.msg.yiaddr);

    uint8_t options[] = {
        DHCP_OPT_MAGIC,
        DHCP_OPT_DHCP_MSG_TYPE(DHCP_MSG_TYPE_REQUEST),
        DHCP_OPT_SERVER_IDENTIFIER((uint8_t *)&sid),
        DHCP_OPT_REQUEST_IP_ADDR((uint8_t *)&yip),
        DHCP_OPT_END
    };

    memset(&tx_addr, 0, sizeof(tx_addr));
    tx_addr.sin_family = AF_INET;
    tx_addr.sin_addr.s_addr = INADDR_ANY; // sid
    tx_addr.sin_port = htons(67);
    
    only.msg.op = BOOTREQUEST;
    only.msg.secs = sys_tick_s()-only.start_time;
    only.msg.flags = DHCP_MESSAGE_FLAG_BROADCAST;
    memcpy(only.msg.chaddr, only.ethaddr, 6);
    only.msg.option = options;
    only.msg.option_size = sizeof(options);
    only.msg.raw_data = malloc(1024);
    if (!only.msg.raw_data) {
        return 0;
    }
    only.msg.raw_data_size = 1024;
    dhcp_message_pack(&only.msg);
    tx_size = sendto(only.tx_sk, only.msg.raw_data, only.msg.raw_data_size, 0, 
        (struct sockaddr*)&tx_addr, sizeof(tx_addr));
    free(only.msg.raw_data);
    if (tx_size <= 0) {
        return 0;
    }

    return 1;
}

// Events
void dhcp_client_start(void)
{
    if (only.udp_setup) {
        switch (only.state) {
            case DHCP_C_STATE_INIT: {
                dhcp_client_send_discover();
                timer_start(&only.T1, 3);
                only.state = DHCP_C_STATE_SELECTING;
            } break;
        }
    }
}

static void dhcp_client_timeout(void)
{
    if (only.udp_setup) {
        switch (only.state) {
            case DHCP_C_STATE_INIT: {
                dhcp_client_send_discover();
                timer_start(&only.T1, 5);
            } break;
            case DHCP_C_STATE_REQUESTING: {
                dhcp_client_send_request();
                timer_start(&only.T1, 5);
            } break;
            case DHCP_C_STATE_BOUND: {
                // T1 expired
                timer_start(&only.T1, 1);
                only.state = DHCP_C_STATE_RENEWING;
            } break;
            case DHCP_C_STATE_RENEWING: {
                // retransmit
                if (!timer_expired(&only.T2)) {
                    dhcp_client_send_request();
                    timer_start(&only.T1, 5);
                }

                // T2 expired
                else {
                    timer_start(&only.T1, 1);
                    timer_start(&only.T2, (1-0.875)*only.lease);
                    only.state = DHCP_C_STATE_REBINDING;
                }
            } break;
            case DHCP_C_STATE_REBINDING: {
                // restransmit
                if (!timer_expired(&only.T2)) {
                    dhcp_client_send_request(); // TODO
                    timer_start(&only.T1, 5);
                }

                // Lease expired
                else {
                    timer_stop(&only.T1);
                    timer_stop(&only.T2);
                    only.state = DHCP_C_STATE_INIT;
                }
            } break;
        }
    }
}

static void dhcp_client_received(void)
{
    switch (only.state) {
        case DHCP_C_STATE_SELECTING: {
            if (dhcp_message_unpack(&only.msg)) {
                if (only.msg.op != BOOTREPLY 
                    || dhcp_option_message_type(&only.msg) != DHCP_MSG_TYPE_OFFER) {
                    timer_start(&only.T1, 5);
                    return;
                }

                // extract data
                // setting ip address
                uint32_t ip = htonl(only.msg.yiaddr);
                only.set_ip((uint8_t*)&ip, 4);

                dhcp_client_send_request();
                timer_start(&only.T1, 5);
                only.state = DHCP_C_STATE_REQUESTING;
            }
        } break;
        case DHCP_C_STATE_REQUESTING: 
        case DHCP_C_STATE_RENEWING: 
        case DHCP_C_STATE_REBINDING: {
            if (dhcp_message_unpack(&only.msg)) {
                if (only.msg.op != BOOTREPLY 
                    || dhcp_option_message_type(&only.msg) != DHCP_MSG_TYPE_ACK) {
                    timer_start(&only.T1, 5);
                    return;
                }

                // Record lease, set timers T1, T2
                if (dhcp_option_ip_lease_time(&only.msg, &only.lease)) {
                    timer_start(&only.T1, 0.5*only.lease);
                    timer_start(&only.T2, 0.875*only.lease);
                }

                only.state = DHCP_C_STATE_BOUND;
            }
        } break;
    }
}

//
void dhcp_loop(void)
{
    if (timer_expired(&only.T1)) {
        timer_stop(&only.T1);
        dhcp_client_timeout();
    }

    if (timer_expired(&only.T2)) {
        timer_stop(&only.T2);
        dhcp_client_timeout();
    }

    if (only.udp_setup) {
        only.msg.raw_data = malloc(1024);
        if (only.msg.raw_data) {
            only.msg.raw_data_size = recvfrom(only.rx_sk, 
                only.msg.raw_data, 1024, MSG_DONTWAIT, NULL, 0);
            if (only.msg.raw_data_size > 0) {
                dhcp_client_received();
            }
            free(only.msg.raw_data);
        }
    }
}

//
int dhcp_client_completed(void)
{
    return (only.state == DHCP_C_STATE_BOUND)
        || (only.state == DHCP_C_STATE_RENEWING)
        || (only.state == DHCP_C_STATE_REBINDING);
}
