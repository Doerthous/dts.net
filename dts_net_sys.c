#include <dts/net/sys.h>
#include <stddef.h>

static size_t tick;
static size_t second;
void dts_net_sys_heartbeat(void)
{
    ++tick;
    size_t s = tick/DTS_NET_SYS_HEARTBEAT_FREQ;
    if (s != second) {
        second = s;
    }
}

size_t sys_tick_s(void)
{
    return second;
}
