#ifndef MEM_H_
#define MEM_H_

#include <dts_net_mem.h>

#define mem_init dts_net_mem_init

#define malloc dts_net_malloc
#define free dts_net_free

#define mem_alloc_udp dts_net_mem_alloc_udp
#define mem_free_udp dts_net_mem_free_udp

#define mem_alloc_nif dts_net_mem_alloc_nif
#define mem_free_nif dts_net_mem_free_nif
#define mem_alloc_arpti dts_net_mem_alloc_arpti
#define mem_free_arpti dts_net_mem_free_arpti

#endif // MEM_H_
