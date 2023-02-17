/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include    "Includes.h"


/* Wlan and Service configration.*/
#define		DEFAULT_WLAN_SSID			"Canaan"
#define		DEFAULT_WLAN_PASSWORD		"12345678"
#define		DEFAULT_SERVER_IP_ADDR		"192.168.137.1"
#define		DEFAULT_SERVER_PORT_ADDR	30050

#define     WIFI_TX_STRING              "hello world\n"

static void io_mux_init(void)
{
    /* WIFI IO map and function settings */
    fpioa_set_function(W600_SPI_CS_PIN,  (FUNC_SPI1_SS0+SPI_CS_SELECT));
    fpioa_set_function(W600_SPI_CLK_PIN,  FUNC_SPI1_SCLK);
    fpioa_set_function(W600_SPI_MOSI_PIN, FUNC_SPI1_D0);
    fpioa_set_function(W600_SPI_MISO_PIN, FUNC_SPI1_D1);

    fpioa_set_function(W600_INTERRUPT_PIN, FUNC_GPIOHS0 + W600_INTERRUPT_IO);
    gpiohs_set_drive_mode(W600_INTERRUPT_IO, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(W600_INTERRUPT_IO, GPIO_PE_FALLING);

    fpioa_set_function(W600_RESET_PIN, FUNC_GPIO0 + W600_RESET_IO);
    gpio_set_drive_mode(W600_RESET_IO, GPIO_DM_OUTPUT);
    gpio_set_pin(W600_RESET_IO, GPIO_PV_HIGH);
}

static void	message_parse_operation(uint8_t *pdata, uint16_t length)
{
    for(uint16_t i=0; i<length; i++) {
        printk("%c", *pdata++);
    }

}

void	core1_exec_loop(void)
{
    uint64_t core = current_coreid();
    printk("Core %ld startup====\n", core);
	
    io_mux_init();

    spi_init(SPI_DEVICE, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    spi_set_clk_rate(SPI_DEVICE, 40000000);

    plic_init();
    sysctl_enable_irq();

    gpiohs_set_irq(W600_INTERRUPT_IO, 1, w600_interrupt_operation_triger);
	w600_register_rxdata_callback(message_parse_operation);
	w600_station_init(DEFAULT_WLAN_SSID, DEFAULT_WLAN_PASSWORD, DEFAULT_SERVER_IP_ADDR, DEFAULT_SERVER_PORT_ADDR, W600_RESET_IO);

	while(1)
	{
        static uint32_t count = 0;
		w600_station_operation();

        if(count++ == 10000000) {

            count = 0;
            w600_station_txdata(WIFI_TX_STRING, strlen(WIFI_TX_STRING));
        }

	}
}

