// msg_cache for caching messages in espnow MESH communication
//
// uint32 src_addr, dst_addr-> using last 4 bytes of WIFI_IF_STA ifx MAC as device identifier
// uint16 id  -> message identificator is a 16 bit value
// so msg_uid are 8 bytes ( source_addr + msg_id)
// uint8 max_msg  -> max messages in cache ) 
#include <stdint.h>



typedef struct {
	uint8_t src_addr[6];
	uint16_t id;
} enow_msg_header_t;



void enow_cache_init();
void enow_cache_add(enow_msg_header_t msg);
int enow_cache_find_msg(enow_msg_header_t msg);
