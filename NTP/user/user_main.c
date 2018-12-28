/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

#define HOSTNAME "ESP8266"
#define SSID "CasaSu"
#define PASSWORD "passwordgazzosa"

struct station_config cfg;
os_timer_t sntp_timer;

void ICACHE_FLASH_ATTR
print_mac(uint8 * mac){
    uint8 i = 0;
    for(i = 0; i < 6; i++){
        os_printf("%02X" , mac[i]);
        if(i != 5){
            os_printf(":");
        }
    }
    os_printf("\n");
}


void ICACHE_FLASH_ATTR
callback(void){
    uint32 current_stamp = 0;
    current_stamp = sntp_get_current_timestamp();
    if(current_stamp){
        os_timer_disarm(&sntp_timer);
        os_printf("sntp: %d, %s \n", current_stamp , sntp_get_real_time(current_stamp));
        os_timer_arm(&sntp_timer, 10000, 0);
    } else {
        //os_printf("riarmo\n");
        os_timer_arm(&sntp_timer, 500, 0);
    }

}


void ICACHE_FLASH_ATTR
system_init_cb(void)
{
    uart_init(115200,115200);
    uint8 ret = 0;
    ret = wifi_get_opmode_default();
    os_printf("Operation mode = %d\n" , ret);
    if(ret != STATION_MODE){
        os_printf("Set operation mode to Station\n");
        if(!wifi_set_opmode(STATION_MODE)){
            os_printf("Error while set to Station\n");
            return;
        }        
    }
    os_printf("Connecting...\n");
    if(wifi_station_set_hostname(HOSTNAME)){
        os_printf("Hostname set to " HOSTNAME "\n");
    } else {
        os_printf("Unable to set hostname\n");
    }

    os_printf("Retriving wifi station configuration information\n");
    ret = wifi_station_get_config_default(&cfg);

    if(ret){
        os_printf("ssid : %0.32s\npassword : %0.64s\nbssid : " MACSTR "\n", cfg.ssid, cfg.password, MAC2STR(cfg.bssid));        print_mac(cfg.bssid);
    } else {
        os_printf("error\n");
    }

    os_printf("Setting new wifi parameter\n");
    os_memcpy(&cfg.ssid, SSID, 32);
    os_memcpy(&cfg.password, PASSWORD, 64);
    cfg.bssid_set = 0;
    wifi_station_set_config(&cfg);

    os_printf("Retriving wifi station configuration information\n");
    ret = wifi_station_get_config_default(&cfg);

    if(ret){
        os_printf("ssid : %0.32s\npassword : %0.64s\nbssid : " MACSTR "\n", cfg.ssid, cfg.password, MAC2STR(cfg.bssid));
    } else {
        os_printf("error\n");
    }

    /* autoconnect enable by default*/
    /*wifi_station_disconnect();
    wifi_station_connect();*/
    
    sntp_setservername(0, "0.it.pool.ntp.org");
    sntp_setservername(1, "1.it.pool.ntp.org");
    sntp_setservername(2, "2.it.pool.ntp.org");
    sntp_stop();
    if(sntp_set_timezone(1)){
        sntp_init();
    } else {
        os_printf("error timezone set");
        return;
    }

    
    os_timer_disarm(&sntp_timer);
    os_timer_setfn(&sntp_timer, (os_timer_func_t *) callback, NULL);
    os_timer_arm(&sntp_timer, 100 ,0);

}


/**
 * @brief Test spi interfaces.
 *
 */
void ICACHE_FLASH_ATTR
user_init(void)
{
    
    system_init_done_cb(system_init_cb);
}

