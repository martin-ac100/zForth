// Copyright 2018-2025 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "zforth.h"
#include "zfconf.h"

#define d data[i]
#define BUF_SIZE (512)
#define UART_W( ... ) uart_write_bytes( __VA_ARGS__ )
#define UART_R( ... ) uart_read_bytes( __VA_ARGS__ )


zf_result do_eval( const char *buf)
{
    const char *msg = NULL;

    zf_result rv = zf_eval(buf);

    switch(rv)
    {
        case ZF_OK: msg = "\033[32mok"; break;
        case ZF_ABORT_INTERNAL_ERROR: msg = "internal error"; break;
        case ZF_ABORT_OUTSIDE_MEM: msg = "outside memory"; break;
        case ZF_ABORT_DSTACK_OVERRUN: msg = "dstack overrun"; break;
        case ZF_ABORT_DSTACK_UNDERRUN: msg = "dstack underrun"; break;
        case ZF_ABORT_RSTACK_OVERRUN: msg = "rstack overrun"; break;
        case ZF_ABORT_RSTACK_UNDERRUN: msg = "rstack underrun"; break;
        case ZF_ABORT_NOT_A_WORD: msg = "not a word"; break;
        case ZF_ABORT_COMPILE_ONLY_WORD: msg = "compile-only word"; break;
        case ZF_ABORT_INVALID_SIZE: msg = "invalid size"; break;
        case ZF_ABORT_DIVISION_BY_ZERO: msg = "division by zero"; break;
        default: msg = "unknown error";
    }

    if(msg) {
        printf( "\033[31m");
        printf( "%s\033[0m\n", msg);
    }
    return rv;
}


zf_input_state zf_host_sys(zf_syscall_id id, const char *last_word) {
	int len;
	void *buf;

    switch((int)id) {


        /* The core system callbacks */

        case ZF_SYSCALL_EMIT:
            putchar((char)zf_pop());
            fflush(stdout);
            break;

        case ZF_SYSCALL_PRINT:
            printf(ZF_CELL_FMT " ", zf_pop());
	    fflush(stdout);
            break;

        case ZF_SYSCALL_TELL:
            len = (int)zf_pop();
            buf = (uint8_t *)zf_dump(NULL) + (int)zf_pop();
	    UART_W(UART_NUM_0, buf, len);
            break;


        /* Application specific callbacks */

        default:
            printf("unhandled syscall %d\n", id);
            break;
    }

    return ZF_INPUT_INTERPRET;

}


void zf_host_trace(const char *fmt, va_list va) {}

zf_cell zf_host_parse_num(const char *buf) {
    zf_cell v;
    int n = 0;
    int r = sscanf(buf, "%d%n", &v, &n);
    if(r == 0 || buf[n] != '\0') {
        zf_abort(ZF_ABORT_NOT_A_WORD);
    }
    return v;

}


static void uart_cmdline()
{
static char cmdline[BUF_SIZE];
static char *cursor_pos = cmdline;
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE, 0, 0, NULL, 0);

    // Configure a temporary buffer for the incoming data
    uint8_t data[1024];

    zf_init(0);
    zf_bootstrap();

    while (1) {
        // Read data from the UART
        int len = UART_R(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
	if (len) { // if any data from UART
		for (int i=0; i<len; i++) { // process all the data
			if ( 31 < d && d < 128 ) {
				*cursor_pos = d;
				UART_W(UART_NUM_0, (const char *) (data+i), 1);
				cursor_pos += 1;
				if ( cursor_pos == (cmdline + BUF_SIZE) ) {
					cursor_pos = cmdline;
					UART_W(UART_NUM_0, "\x1b[D",3);
				}
			}
			else if ( d == 0x0D || d == 0x0A ) {
				UART_W(UART_NUM_0, "\x0D\x0A",2);
				*cursor_pos = 0 ;
				if (cursor_pos - cmdline) do_eval(cmdline);
				cursor_pos = cmdline;
			}
			else if ( d == 0x08 ) {
				*cursor_pos = 0 ;
				cursor_pos -= 1 ;
				UART_W(UART_NUM_0, "\x1b[D\x1b[K", 6);
			}

		}
	}
    }

}

void app_main()
{
    xTaskCreate(uart_cmdline, "uart_cmdline_task", 2048, NULL, 10, NULL);
}
