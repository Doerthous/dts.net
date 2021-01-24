#include <dts/net/sys.h>
#include <dts/embedded/lib/timer.h>

#include <bsd/socket.h>
#include <arpa/inet.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

#include <string.h>

int udp_client_test(void)
{
    static timer_t tmr;
    struct sockaddr_in ser_addr;
    //static uint8_t bp[2048];

    co_start();

    timer_init(&tmr, sys_tick_s);
    timer_start(&tmr, 2);
    
    while (1) {
        if (timer_expired(&tmr)) {
            timer_restart(&tmr);
            int client = socket(AF_INET, SOCK_DGRAM, 0);
            if (client > 0) {
                memset(&ser_addr, 0, sizeof(ser_addr));
                ser_addr.sin_family = AF_INET;
                ser_addr.sin_addr.s_addr = inet_addr("10.0.0.1");
                ser_addr.sin_port = htons(50002);
            }
            const char *data = "012345";
            sendto(client, data, 6, 0, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
            //sendto(client, bp, 2048, 0, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
            close(client);
        }

        co_yield();
    }

    co_exit();
}
