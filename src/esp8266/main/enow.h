#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "rom/ets_sys.h"
#include "enow_msg_cache.h"


typedef struct {
	uint8_t len;
	enow_msg_header_t header;
	char payload[];
} enow_msg_t;

void enow_set_recv_CB(void *p_CB);
esp_err_t enow_init(int channel);
esp_err_t enow_send(char *p_payload);
uint8_t dev_mac[ESP_NOW_ETH_ALEN];
char send_buffer[256];
