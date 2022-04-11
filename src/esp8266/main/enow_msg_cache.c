#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "enow_msg_cache.h"

#define max_msg 16
static const char* TAG="enow_msg_cache";
static uint8_t cache_pos;
static enow_msg_header_t cache[max_msg];

void enow_cache_init() {
	cache_pos=0;
	memset(&cache,0,max_msg*sizeof(enow_msg_header_t));
	ESP_LOGI(TAG,"Cache for %d messages has been initialized.\n",max_msg);
}

void enow_cache_add(enow_msg_header_t msg) {
	ESP_LOGD(TAG,"Adding msg from dev %d, msg_id %d to cache at pos %d.", (uint32_t)*msg.src_addr, msg.id, cache_pos);
	cache[cache_pos] = msg;
	ESP_LOGD(TAG,"Cache item at pos %d is %d, %d",cache_pos,(uint32_t)*cache[cache_pos].src_addr, cache[cache_pos].id);
	cache_pos++;
	if (cache_pos == max_msg) {
		cache_pos = 0;
	}
	
}

int enow_cache_find_msg(enow_msg_header_t msg) {
	uint8_t i,p;
	
	ESP_LOGD(TAG,"Looking for msg %d, %d.", (uint32_t)*msg.src_addr, msg.id);

	for (i = 0, p = cache_pos; i < max_msg; i++) {
		if (p == 0) {
			p = max_msg;
		}
		p--;

		ESP_LOGD(TAG,"Reading record %d of cache: %d, %d",p, (uint32_t)*cache[p].src_addr, cache[p].id);

		if (memcmp(&cache[p],&msg,sizeof(msg)) == 0) {
			return 1;
		}
		if (cache[p].src_addr == 0) {
			return 0;
		}

		
	}
	return 0;
}

