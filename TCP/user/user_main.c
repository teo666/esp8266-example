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
#include "espconn.h"


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
#define SSID "yourownssid"
#define PASSWORD "yourownpassword"

#define REMOTE_PORT 2000;
#define REMOTE_IP {192,168,0,12}

#define DATA "Hello Server!\n"

struct station_config cfg;

struct espconn info;
esp_tcp tcp;

void ICACHE_FLASH_ATTR
connect_cb(void *arg) {
    os_printf("connessione avvenuta con sucesso\n");
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
        os_printf("ssid : %0.32s\npassword : %0.64s\nbssid : " MACSTR "\n", cfg.ssid, cfg.password, MAC2STR(cfg.bssid));
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

    os_printf("Max TCP connection %d\n", espconn_tcp_get_max_con());

    wifi_station_disconnect();
    wifi_station_connect();

}

void ICACHE_FLASH_ATTR
data_sent( void *arg ){
    os_printf("data sent successfully\n");
}

void ICACHE_FLASH_ATTR
tcp_connected( void *arg ){
    os_printf("tcp_connected\n");
    espconn_regist_sentcb( &info, data_sent);
    espconn_send( &info, DATA, os_strlen(DATA));
    //espconn_disconnect( &info );
}


void ICACHE_FLASH_ATTR
tcp_disconnected( void *arg ){
    os_printf("tcp_disconnected\n");
}

void ICACHE_FLASH_ATTR
make_connection()
{
    os_printf("Connecting...\n" );
    
    info.type = ESPCONN_TCP;
    info.state = ESPCONN_NONE;
    info.proto.tcp = &tcp;
    info.proto.tcp->local_port = espconn_port();
    info.proto.tcp->remote_port = REMOTE_PORT;
    uint8 ip[4] = REMOTE_IP;
    os_memcpy( tcp.remote_ip, ip, 4 );
    os_printf("%d.", tcp.remote_ip[0] );
    os_printf("%d.", tcp.remote_ip[1] );
    os_printf("%d.", tcp.remote_ip[2] );
    os_printf("%d\n", tcp.remote_ip[3] );

    espconn_regist_connectcb( &info, tcp_connected );
    espconn_regist_disconcb( &info, tcp_disconnected );
    
    espconn_connect( &info );
}



void ICACHE_FLASH_ATTR
wifi_handle_event_cb(System_Event_t *evt) {
        int8 ret_ = 0;

    os_printf("event %x\n", evt->event);
    switch (evt->event) {
    case EVENT_STAMODE_CONNECTED:
        os_printf("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid, evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            os_printf("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            os_printf("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\n");
            

            make_connection();
    
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            os_printf("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac), evt->event_info.sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            os_printf("station: " MACSTR "leave, AID = %d\n", MAC2STR(evt->event_info.sta_disconnected.mac), evt->event_info.sta_disconnected.aid);
            break;
        default:
            break;
    }
}

void ICACHE_FLASH_ATTR
uart0_rx_intr_handler(void * para){
    os_printf(" sdsdsd ");
}


/**
 * @brief Test spi interfaces.
 *
 */
void ICACHE_FLASH_ATTR
user_init(void)
{
    wifi_station_set_auto_connect(false);
    wifi_set_event_handler_cb(wifi_handle_event_cb);
    system_init_done_cb(system_init_cb);
}

