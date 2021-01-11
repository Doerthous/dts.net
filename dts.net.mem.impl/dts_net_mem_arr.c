#include <dts/net/mem.h>

void mem_init(void)
{

}

#include <dts/net/udp.h>
static struct 
{
    udp_t udp;
    int used;
} udps[DTS_NET_SERVER_UDP_MAX_COUNT];
udp_t *dts_net_mem_alloc_udp()
{
    for (int i = 0; i < DTS_NET_SERVER_UDP_MAX_COUNT; ++i) {
        if (!udps[i].used) {
            udps[i].used = 1;
            return &udps[i].udp;
        }
    }
	
	return NULL;
}
void dts_net_mem_free_udp(udp_t *udp)
{
    for (int i = 0; i < DTS_NET_SERVER_UDP_MAX_COUNT; ++i) {
        if (&udps[i].udp == udp) {
            udps[i].used = 0;
        }
    }
}


#include <dts/net/ether_arp.h>
static struct { nif_t nif; int used; } nifs[DTS_NET_NET_IF_MAX_COUNT];
nif_t *dts_net_mem_alloc_nif()
{
    for (int i = 0; i < DTS_NET_NET_IF_MAX_COUNT; ++i) {
        if (!nifs[i].used) {
            nifs[i].used = 1;
            return &nifs[i].nif;
        }
    }
	
	return NULL;
}
void dts_net_mem_free_nif(nif_t *nif)
{
    for (int i = 0; i < DTS_NET_NET_IF_MAX_COUNT; ++i) {
        if (&nifs[i].nif == nif) {
            nifs[i].used = 0;
        }
    }
}

static struct 
{
    ether_arp_ti_t arpti;
    int used;
} arptis[DTS_NET_ARP_TABLE_MAX_SIZE];
ether_arp_ti_t *mem_alloc_arpti()
{
    for (int i = 0; i < DTS_NET_ARP_TABLE_MAX_SIZE; ++i) {
        if (!arptis[i].used) {
            arptis[i].used = 1;
            return &arptis[i].arpti;
        }
    }
	
	return NULL;
}
void mem_free_arpti(ether_arp_ti_t *arpti)
{
    for (int i = 0; i < DTS_NET_ARP_TABLE_MAX_SIZE; ++i) {
        if (&arptis[i].arpti == arpti) {
            arptis[i].used = 0;
        }
    }
}
