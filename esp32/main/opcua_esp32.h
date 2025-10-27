#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <esp_flash_encrypt.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include "esp_event.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "lwip/ip_addr.h"
#include "sdkconfig.h"

#include "ethernet_connect.h"
#include "open62541.h"
#include "DHT22.h"
#include "model.h"

#include <stdio.h>
#include "driver/gpio.h"
#include <string.h>
#include <unistd.h> // For read() and STDIN_FILENO