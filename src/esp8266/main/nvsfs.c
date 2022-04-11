#include <stdint.h>
#include "nvsfs.h"
#include "esp_partition.h"
#include "esp_image_format.h"
#include "esp_log.h"

#define nvs_read(nvs_part, addr, buf, size) esp_partition_read(nvs_part, addr, buf, size)
#define nvs_write(nvs_part, addr, buf, size) esp_partition_write(nvs_part, addr, buf, size)

int FF = -1;

const static esp_partition_t *nvs_part;
size_t nvs_start;
size_t nvs_space;

size_t nvs_get_start(const esp_partition_t *nvs_part) {
	int i;
	size_t offset;
	esp_image_header_t img_header;
	esp_image_segment_header_t seg_header;
	
	nvs_read(nvs_part, 0, &img_header, sizeof(esp_image_header_t));
	offset = sizeof(esp_image_header_t);
	for ( i=0; i < img_header.segment_count; i++ ) {
		nvs_read(nvs_part, offset, &seg_header, sizeof(esp_image_segment_header_t));
		offset += sizeof(esp_image_segment_header_t) + seg_header.data_len;
	}
	offset += 4096;
	offset &= 0xFFFFF000; // 4kb-aligned
	return offset;
}

size_t nvs_init() {
	nvs_part = esp_partition_find_first( ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
	ESP_LOGI("nvsfs", "nvs_part = %x",(unsigned int)nvs_part);
	if ( nvs_part == NULL ) {ESP_LOGE("nvsfs","could not find APP partition :-/");}
	nvs_start = nvs_get_start(nvs_part);
	if ( (nvs_start + 4096) > nvs_part->size) {

		nvs_space = 0;
	}
	else {
		nvs_space = nvs_part->size - nvs_start;
	}

	ESP_LOGI("nvsfs", "nvs_part start_offset at %u",nvs_start);
	ESP_LOGI("nvsfs","nvs space = %u bytes",nvs_space);
	return nvs_space;
}

	
esp_err_t nvs_erase() {
	ESP_LOGI("nvsfs","Erasing flash from %u of size %u\n",nvs_start, nvs_space);
	return esp_partition_erase_range(nvs_part, nvs_start, nvs_space);
}

size_t last_rec(const esp_partition_t *nvs_part, size_t *last_len) {
	size_t offset = nvs_start, last_offset = nvs_start, len;
	while (1) {
		if ( nvs_read(nvs_part, offset, (void *)&len, sizeof(len)) != ESP_OK ) len = 0;
		if ( (int)len <= 0 ) break;
		ESP_LOGI("nvsfs, last_rec","offset=%d, len=%d",offset,len);
		*last_len = len;
		last_offset=offset;
		offset += len + sizeof(len);
	}
	ESP_LOGI("nvsfs","rec_addr = %d, rec_len = %d\n", last_offset, *last_len);
	return last_offset;
}

size_t nvs_load( void *buf) {
	size_t addr, len;
		addr = last_rec(nvs_part,&len);
		ESP_LOGI("nvsfs","loading rec_addr = %d, rec_len = %d buf_addr=%x", addr, len, (unsigned int) buf);
		if (len > 0) {
			addr += sizeof(len);
			nvs_read(nvs_part, addr, buf, len);
		}
	return len;
}



size_t nvs_save( void *buf, size_t len) {
	size_t addr;
	size_t last_len, total_len;

	ESP_LOGI("nvsfs", "saving dict of size %d from addr %x",len, (unsigned int) buf);
	total_len = len + sizeof(len);
	addr = last_rec(nvs_part,&last_len);

	ESP_LOGI("nvsfs","rec_addr = %d, rec_len = %d\n", addr, len);
	if ( last_len > 0 ) addr += last_len + sizeof(len);

	if ( (addr + total_len) < nvs_part->size ) {
		nvs_write(nvs_part, addr, (void *)&len, sizeof(len));
		addr += sizeof(len);
		return nvs_write(nvs_part, addr, buf, len);
	}
	else {
		if ( len <= nvs_part->size ) {
			nvs_erase(nvs_part);
			nvs_write(nvs_part, nvs_start, &len, sizeof(len));
			return nvs_write(nvs_part, nvs_start + sizeof(len), buf, len-sizeof(len));
		}
		else {
			ESP_LOGE("nvsfs","Not enough space - partition has %uB, image %uB\n",nvs_space,len);
			return -1;
		}
	}
}


