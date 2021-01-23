#include <dts/net/tcp.h>
#include <dts/net/sys.h>
#include <dts/embedded/lib/timer.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

int tcp_client_test(void)
{
    static tcp_t *tcp;
    static timer_t tmr;
    
    tcp_loop(tcp);
    
    co_start();
    
    {
        DTS_NET_IPv4_ADDR_DEF(local, 10,0,0,2);
        DTS_NET_IPv4_ADDR_DEF(remote, 10,0,0,1);
        tcp = tcp_open(&local, 2000, &remote, 12346);
    }
    
    if (tcp) {
        co_wait_until(tcp_status(tcp) == DTS_NET_TCP_STATE_ESTABLISHED);
        
        tcp_send(tcp, (uint8_t*)"Hello!", 6);
        
        timer_init(&tmr, sys_tick_s);
        timer_start(&tmr, 10);
        
        while (1) {
            if (timer_expired(&tmr)) {
                tcp_close(tcp);
                break;
            }
            else {
                uint8_t buff[16];
                size_t sz;
                if ((sz = tcp_recv(tcp, buff, 16)) > 0) {
                    sz++;
                }
            }
            co_yield();
        }
    }
    
    co_exit();
}
