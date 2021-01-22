#include <bsd/socket.h>
#include <arpa/inet.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

#include <string.h>

int udp_server_test(void)
{
	static uint8_t buff[32];
	static int server;
    static struct sockaddr_in ser_addr;

    co_start();

    co_wait_until((server = socket(AF_INET, SOCK_DGRAM, 0)) != NULL);

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr("10.0.0.2");
    ser_addr.sin_port = htons(2000);
    while (bind(server, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) != 0) {
        close(server);
        co_yield();
    }

    while (1) {
        int rc = recvfrom(server, buff, 12, MSG_DONTWAIT, NULL, 0);
        if (rc > 0) {
            ++rc;
        }
        co_yield();
    }

    co_exit();
}
