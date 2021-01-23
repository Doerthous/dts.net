#include <dts/net/tcp.h>
#include <dts/net/sys.h>
#include <dts/embedded/lib/timer.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

int tcp_server_test(void)
{
	static tcp_t *tcp;
	static timer_t tmr;
	
	tcp_loop(tcp);
	
	co_start();
	
	{
		DTS_NET_IPv4_ADDR_DEF(local, 10,0,0,2);
		tcp = tcp_open(&local, 5000, NULL, 0);
	}
	
	if (tcp) {
		co_wait_until(tcp_status(tcp) == DTS_NET_TCP_STATE_ESTABLISHED);
		
		timer_init(&tmr, sys_tick_s);
		timer_start(&tmr, 1200);
		
		while (1) {
			if (timer_expired(&tmr) || 
                tcp_status(tcp) == DTS_NET_TCP_STATE_CLOSE_WAIT) {
				tcp_close(tcp);
				break;
			}
			else {
				uint8_t buff[16];
				size_t sz;
				if ((sz = tcp_recv(tcp, buff, 16)) > 0) {
                    tcp_send(tcp, buff, sz);
				}
			}
			co_yield();
		}
	}
	
	co_exit();
}
