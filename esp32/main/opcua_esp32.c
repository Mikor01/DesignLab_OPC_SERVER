#include "opcua_esp32.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY 10

#define TAG "OPCUA_ESP32"
#define SNTP_TAG "SNTP"
#define MEMORY_TAG "MEMORY"
#define ENABLE_MDNS 1

#define UART_TAG "UART_BRIDGE"

// UART Bridge Configuration
#define UART_PC_NUM         (UART_NUM_0)
const int UART_DEVICE_NUM = UART_NUM_2; 
#define UART_BUF_SIZE       (1024)

// Pinout for UART2 (to external device)
#define TXD2_PIN            (GPIO_NUM_17)
#define RXD2_PIN            (GPIO_NUM_16)

static bool obtain_time(void);
static void initialize_sntp(void);

UA_ServerConfig *config;
static UA_Boolean sntp_initialized = false;
static UA_Boolean running = true;
static UA_Boolean isServerCreated = false;
RTC_DATA_ATTR static int boot_count = 0;
static struct tm timeinfo;
static time_t now = 0;

static UA_StatusCode
UA_ServerConfig_setUriName(UA_ServerConfig *uaServerConfig, const char *uri, const char *name)
{
    // delete pre-initialized values
    UA_String_clear(&uaServerConfig->applicationDescription.applicationUri);
    UA_LocalizedText_clear(&uaServerConfig->applicationDescription.applicationName);

    uaServerConfig->applicationDescription.applicationUri = UA_String_fromChars(uri);
    uaServerConfig->applicationDescription.applicationName.locale = UA_STRING_NULL;
    uaServerConfig->applicationDescription.applicationName.text = UA_String_fromChars(name);

    for (size_t i = 0; i < uaServerConfig->endpointsSize; i++)
    {
        UA_String_clear(&uaServerConfig->endpoints[i].server.applicationUri);
        UA_LocalizedText_clear(
            &uaServerConfig->endpoints[i].server.applicationName);

        UA_String_copy(&uaServerConfig->applicationDescription.applicationUri,
                       &uaServerConfig->endpoints[i].server.applicationUri);

        UA_LocalizedText_copy(&uaServerConfig->applicationDescription.applicationName,
                              &uaServerConfig->endpoints[i].server.applicationName);
    }

    return UA_STATUSCODE_GOOD;
}

static void opcua_task(void *arg)
{
    // BufferSize's got to be decreased due to latest refactorings in open62541 v1.2rc.
    UA_Int32 sendBufferSize = 16384;
    UA_Int32 recvBufferSize = 16384;

    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    ESP_LOGI(TAG, "Fire up OPC UA Server.");
    UA_Server *server = UA_Server_new();
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_ServerConfig_setMinimalCustomBuffer(config, 4840, 0, sendBufferSize, recvBufferSize);

    const char *appUri = "open62541.esp32.server";
    UA_String hostName = UA_STRING("opcua-esp32");
    
    UA_ServerConfig_setUriName(config, appUri, "OPC_UA_Server_ESP32");
    UA_ServerConfig_setCustomHostname(config, hostName);

    /* Add Information Model Objects Here */
    addCurrentTemperatureDataSourceVariable(server);
    addRelay0ControlNode(server);
    addRelay1ControlNode(server);
    

    addIN1ControlNode(server);
    addIN2ControlNode(server);
    addUARTStatusNode(server);

    ESP_LOGI(TAG, "Heap Left : %d", xPortGetFreeHeapSize());
    UA_StatusCode retval = UA_Server_run_startup(server);
    if (retval == UA_STATUSCODE_GOOD)
    {
        while (running)
        {
            UA_Server_run_iterate(server, false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            ESP_ERROR_CHECK(esp_task_wdt_reset());
            taskYIELD();
        }
        UA_Server_run_shutdown(server);
    }
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(SNTP_TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(SNTP_TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.google.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    sntp_initialized = true;
}

static bool obtain_time(void)
{
    initialize_sntp();
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    memset(&timeinfo, 0, sizeof(struct tm));
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry <= retry_count)
    {
        ESP_LOGI(SNTP_TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(esp_task_wdt_reset());
    }
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
    return timeinfo.tm_year > (2016 - 1900);
}

static void opc_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (sntp_initialized != true)
    {
        if (timeinfo.tm_year < (2016 - 1900))
        {
            ESP_LOGI(SNTP_TAG, "Time is not set yet. Setting up network connection and getting time over NTP.");
            if (!obtain_time())
            {
                ESP_LOGE(SNTP_TAG, "Could not get time from NTP. Using default timestamp.");
            }
            time(&now);
        }
        localtime_r(&now, &timeinfo);
        ESP_LOGI(SNTP_TAG, "Current time: %d-%02d-%02d %02d:%02d:%02d", 
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    if (!isServerCreated)
    {
        xTaskCreatePinnedToCore(opcua_task, "opcua_task", 8192, NULL, 6, NULL, 1);
        ESP_LOGI(MEMORY_TAG, "Heap size after OPC UA Task : %d", esp_get_free_heap_size());
        isServerCreated = true;
    }
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
}

static void connection_scan(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, GOT_IP_EVENT, &opc_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(BASE_IP_EVENT, DISCONNECT_EVENT, &disconnect_handler, NULL));
    ESP_ERROR_CHECK(example_connect());
}

static void process_pc_command(const char *command)
{
    ESP_LOGI(UART_TAG, "Processing command: [%s]", command);

    // Simple parser for commands like "IN1 5", "IN2 10", "PING", "STATUS"
    char cmd[16];
    int value = 0;
    int parsed = sscanf(command, "%15s %d", cmd, &value);

    if (strcasecmp(cmd, "PING") == 0) {
        printf("PONG\n");
        uart_write_bytes(UART_DEVICE_NUM, "PING\n", 5);

    } else if (strcasecmp(cmd, "STATUS") == 0) {
        ESP_LOGI(UART_TAG, "Requesting status from Arduino");
        uart_write_bytes(UART_DEVICE_NUM, "STATUS\n", 7);

    } else if (strcasecmp(cmd, "IN1") == 0 && parsed == 2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "SET IN1 %d\n", value);
        ESP_LOGI(UART_TAG, "Forwarding command: %s", buffer);
        uart_write_bytes(UART_DEVICE_NUM, buffer, strlen(buffer));

    } else if (strcasecmp(cmd, "IN2") == 0 && parsed == 2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "SET IN2 %d\n", value);
        ESP_LOGI(UART_TAG, "Forwarding command: %s", buffer);
        uart_write_bytes(UART_DEVICE_NUM, buffer, strlen(buffer));

    } else {
        printf("ERROR: Unknown or invalid command '%s'\n", command);
        printf("Usage:\n  IN1 <1-12>\n  IN2 <1-12>\n  STATUS\n  PING\n");
    }
}

/**
 * @brief The main task for handling UART communication.
 * 
 * This task does two things in its main loop:
 * 1. Assembles complete command lines from the PC (stdin) and processes them.
 * 2. Reads and displays any asynchronous responses from the external device (UART2).
 */
static void uart_bridge_task(void *arg)
{
    // --- UART2 (Device) Configuration ---
    uart_config_t uart_config_device = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Install the driver, allocating RX and TX buffers
    ESP_ERROR_CHECK(uart_driver_install(UART_DEVICE_NUM, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_DEVICE_NUM, &uart_config_device));

    // Now, connect the UART peripheral to the correctly configured pins
    ESP_ERROR_CHECK(uart_set_pin(UART_DEVICE_NUM, TXD2_PIN, RXD2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // --- Buffers for communication ---
    uint8_t *device_data = (uint8_t *) malloc(UART_BUF_SIZE);
    static char pc_cmd_buffer[128];
    static int pc_cmd_index = 0;

    ESP_LOGI(UART_TAG, "UART Command Processor Initialized.");
    printf("Ready for commands (PING, STATUS, IN1 <1-12>, IN2 <1-12>)\n");

    while (1) {
        char c;
        int len_pc = read(STDIN_FILENO, &c, 1);

        if (len_pc > 0) {
            if (c == '\n' || c == '\r') {
                pc_cmd_buffer[pc_cmd_index] = '\0';
                if (pc_cmd_index > 0) {
                    process_pc_command(pc_cmd_buffer);
                }
                pc_cmd_index = 0;
            } else {
                if (pc_cmd_index < (sizeof(pc_cmd_buffer) - 1)) {
                    pc_cmd_buffer[pc_cmd_index++] = c;
                }
            }
        }

        int len_device = uart_read_bytes(UART_DEVICE_NUM, device_data, UART_BUF_SIZE, 20 / portTICK_PERIOD_MS);
        if (len_device > 0) {
            device_data[len_device] = '\0';
            printf("<<< Device Response: [%s]\n", (char*)device_data);
        }

        // Yield CPU time to other tasks
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    
    free(device_data);
    uart_driver_delete(UART_DEVICE_NUM);
}

void app_main(void)
{
    ++boot_count;
    // Workaround for CVE-2019-15894
    nvs_flash_init();
    if (esp_flash_encryption_enabled())
    {
        esp_flash_write_protect_crypt_cnt();
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    connection_scan();

    xTaskCreate(uart_bridge_task, "uart_bridge_task", 4096, NULL, 5, NULL);
}
