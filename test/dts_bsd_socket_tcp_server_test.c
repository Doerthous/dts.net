#include <dts/net/sys.h>
#include <dts/embedded/lib/timer.h>

#include <bsd/socket.h>
#include <arpa/inet.h>

#define USING_DTS_COROUTINE
#include <dts_coroutine.h>

#include <string.h>

#define SERVER_ADDR "10.0.0.2"
#define SERVER_PORT 2345

int dts_bsd_socket_tcp_server_test(void)
{
    struct sockaddr_in ser_addr;
    static int cnt = 3;
    static int server;
    static int client;
    co_start();

    while (cnt--) {
        server = socket(AF_INET, SOCK_STREAM, 0);
        if (server > 0) {
            memset(&ser_addr, 0, sizeof(ser_addr));
            ser_addr.sin_family = AF_INET;
            ser_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
            ser_addr.sin_port = htons(SERVER_PORT);
            if (bind(server, (struct sockaddr*)&ser_addr, sizeof(ser_addr))) {
                continue;
            }
        }
        
        listen(server, 1);

        while (1) {
            client = -1;
            while (client == -1) {
                co_yield();
                client = accept(server, NULL, 0);
            }

            while (1) {
                uint8_t buff[16];
                ssize_t sz;
                sz = recv(client, buff, 8, 0);
                if (sz > 0) {
                    send(client, buff, sz, 0);
                }
                if (sz == -1) {
                    close(client);
                    break;
                }
                co_yield();
            }
        }
    }

    co_exit();
}
