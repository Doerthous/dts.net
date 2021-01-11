#ifndef DTS_NETWORK_MEM_H_
#define DTS_NETWORK_MEM_H_

#include <dts_net_conf.h>
#include <stddef.h>
#include <stdint.h>

extern void dts_net_mem_init(void);

extern void *dts_net_malloc(size_t size);
extern void dts_net_free(void *ptr);

#include <dts_net_udp.h>
extern dts_net_udp_t *dts_net_mem_alloc_udp(void);
extern void dts_net_mem_free_udp(dts_net_udp_t *udp);

#include <dts_net_ether_arp.h>
extern dts_net_nif_t *dts_net_mem_alloc_nif(void);
extern void dts_net_mem_free_nif(dts_net_nif_t *nif);
extern dts_net_ether_arp_ti_t *dts_net_mem_alloc_arpti(void);
extern void dts_net_mem_free_arpti(dts_net_ether_arp_ti_t *nif);

#endif // DTS_NETWORK_MEM_H_
