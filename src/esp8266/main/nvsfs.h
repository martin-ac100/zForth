#include <stdint.h>
#include "esp_partition.h"
#include "esp_log.h"


size_t nvs_init();
esp_err_t nvs_erase();
size_t nvs_load( void *buf);
size_t nvs_save( void *buf, size_t len);

