#include "enow.h"
#include "params.h"
#include "mqtt_utils.h"
#include "tokens.h"
static const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static QueueHandle_t enowQueue= NULL;

static enow_msg_header_t device_msg_header;
static void (*p_enow_recv_CB)(char *)= NULL;

config_t config;

void enow_set_recv_CB(void *p_CB)
{
	p_enow_recv_CB = p_CB;
}

static void on_data_recv(void *pvParameter)
{
	enow_msg_t *p_recv_msg = NULL;	
	char *cmdline;
	int sub_topic;
	int N;
	if (enowQueue== NULL) {
		ESP_LOGE("enow","xQueue not ready");
		return;
	}
	while (1) {
		if (xQueueReceive(enowQueue, &p_recv_msg, portMAX_DELAY)) {
			ESP_LOGD("enow","New message in queue...len = %d",p_recv_msg->len);
			if (enow_cache_find_msg(p_recv_msg->header) == 0) { //msg not found in cache -> PROCESS MSG
				if (config.mesh && memcmp(p_recv_msg->header.src_addr,device_msg_header.src_addr,6)) { //if mesh AND src_addr is not equal own MAC
					vTaskDelay(pdMS_TO_TICKS(esp_random() & 0xFF));
					esp_now_send( broadcast_mac, (const unsigned char *)p_recv_msg, p_recv_msg->len);
				}
				enow_cache_add(p_recv_msg->header);
				if ( !strcasecmp(p_recv_msg->payload, "discover") ) {
					vTaskDelay(pdMS_TO_TICKS(esp_random() & 0x1FFF));
					PUBLISHF("MAC %s",mac_str);
				}
				else {
					if (p_enow_recv_CB) {
						p_enow_recv_CB(p_recv_msg->payload);
					}
				}
			}
			vPortFree(p_recv_msg);
		}
	}
}

static void IRAM_ATTR my_recv_cb(const uint8_t *mac, const uint8_t *data, int len)

{
BaseType_t xHigherPriorityTaskWoken;
uint8_t *msg;

	msg = pvPortMalloc(len+2);
	memset(msg+len, 0, 2); //add 0x00, 0x00 to the end of payload to be sure, that payload string is double NULL terminated. This is necessary for commandline parsing as 0x00, 0x00 is the END of cmdline.
	memcpy(msg, data, len);

	if (enowQueue == NULL) {
		vPortFree(msg);
		return;
	}
	if ( xQueueSendToBackFromISR(enowQueue, &msg, &xHigherPriorityTaskWoken ) != pdPASS ) {
		vPortFree(msg);
	}
	if (xHigherPriorityTaskWoken) {
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

static esp_err_t enow_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI("enow","WiFi started");
        break;
    default:
        break;
    }
    return ESP_OK;
}
esp_err_t enow_send( char *p_payload)
{
	uint8_t payload_len = strlen(p_payload) + 1; //add 1 byte for trailing NULL 
	ESP_LOGD("enow","sending payload of size %d",payload_len);
	enow_msg_t *p_msg = (enow_msg_t *)send_buffer;
	p_msg->header = device_msg_header;
	p_msg->len = sizeof(enow_msg_t) + payload_len;
	memcpy(p_msg->payload, p_payload, payload_len);
	p_msg->payload[payload_len]=0;
	device_msg_header.id++;
	return esp_now_send( broadcast_mac, (const uint8_t *)p_msg, p_msg->len);
}


/* WiFi should start before using ESPNOW */
static esp_err_t enow_wifi_init(int channel)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(enow_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());

    /* In order to simplify example, channel is set after WiFi started.
     * This is not necessary in real application if the two devices have
     * been already on the same channel.
     */
    ESP_ERROR_CHECK( esp_wifi_set_channel(channel, 0) );


	enow_cache_init();

	esp_wifi_get_mac(ESP_IF_WIFI_STA, dev_mac);
	enowQueue = xQueueCreate(6, sizeof(enow_msg_t* ));
	if (enowQueue != NULL) { 
		xTaskCreate(&on_data_recv, "on_data_recv", 4096, NULL, 2, NULL);
	}
	else {
		return ESP_FAIL;
	}

    

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(my_recv_cb) );

    /* Set primary master key. */
    //ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = pvPortMalloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = channel;
    peer->ifidx = ESP_IF_WIFI_STA;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    vPortFree(peer);

    /* Initialize sending parameters. */
    memcpy(&device_msg_header.src_addr, &dev_mac,6);

    return ESP_OK;
}


esp_err_t enow_init(int channel)
{
    return enow_wifi_init(channel);
}
