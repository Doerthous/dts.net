#include <dts/net/sys.h>
#include <dts/embedded/lib/timer.h>

#include <bsd/socket.h>
#include <arpa/inet.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

#include <string.h>

#define SERVER_ADDR "10.0.0.1"
#define SERVER_PORT 12347

int dts_bsd_socket_tcp_client_test(void)
{
    static timer_t tmr;
    struct sockaddr_in ser_addr;
    static int cnt = 3;
	const char *data = "012345";
	static int client;
    co_start();

    timer_init(&tmr, sys_tick_s);
    timer_start(&tmr, 5);

    while (cnt--) {
        co_wait_until(timer_expired(&tmr));
        client = socket(AF_INET, SOCK_STREAM, 0);
        if (client > 0) {
            memset(&ser_addr, 0, sizeof(ser_addr));
            ser_addr.sin_family = AF_INET;
            ser_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
            ser_addr.sin_port = htons(SERVER_PORT);
            if (connect(client, (struct sockaddr*)&ser_addr, sizeof(ser_addr))) {
				timer_restart(&tmr);
                continue;
            }
        }
        co_wait_until(send(client, data, 6, 0) == 6);
        close(client);
		timer_restart(&tmr);
    }

    co_exit();
}
