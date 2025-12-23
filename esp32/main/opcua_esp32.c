#include "opcua_esp32.h"
#include "model.h"
#include "uart_bridge.h"

#define TAG "OPCUA_ESP32"
#define SNTP_TAG "SNTP"
#define MEMORY_TAG "MEMORY"
#define ENABLE_MDNS 1

#define UART_TAG "UART_BRIDGE"

const int UART_DEVICE_NUM = UART_NUM_2;

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
    UA_Int32 sendBufferSize = 8192;
    UA_Int32 recvBufferSize = 8192;

    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    ESP_LOGI(TAG, "Fire up OPC UA Server.");
    UA_Server *server = UA_Server_new();
    UA_ServerConfig *config = UA_Server_getConfig(server);

    ESP_LOGI(TAG, "Ustawiono maxSessions na: %d", (int)config->maxSessions);
    UA_ServerConfig_setMinimalCustomBuffer(config, 4840, 0, sendBufferSize, recvBufferSize);

    const char *appUri = "open62541.esp32.server";
    UA_String hostName = UA_STRING("opcua-esp32");
    
    UA_ServerConfig_setUriName(config, appUri, "OPC_UA_Server_ESP32");
    UA_ServerConfig_setCustomHostname(config, hostName);
    //config->maxSessions = 1;
    addIN1ControlNode(server);
    addIN2ControlNode(server);
    addUARTStatusNode(server);

    ESP_LOGI(TAG, "Heap Left : %d", xPortGetFreeHeapSize());
    UA_StatusCode retval = UA_Server_run_startup(server);
    
    static UA_Int32 opcua_known_IN1 = 0;
    static UA_Int32 opcua_known_IN2 = 0;
    UA_DataValue data_value;
    UA_DataValue_init(&data_value);
    data_value.hasValue = true;

    if (retval == UA_STATUSCODE_GOOD)
    {
        while (running)
        {

            UA_Server_run_iterate(server, false);
            /*UA_ServerStatistics stats = UA_Server_getStatistics(server);
            ESP_LOGI(TAG, "Aktualna liczba sesji: %zu", stats.ss.currentSessionCount);*/ // check how many sessions are connected
            if (current_IN1_value != opcua_known_IN1) {
                opcua_known_IN1 = current_IN1_value;
                UA_Variant_setScalar(&data_value.value, &opcua_known_IN1, &UA_TYPES[UA_TYPES_INT32]);
                UA_Server_writeDataValue(server, UA_NODEID_STRING(1, "IN1_control"), data_value);
            }
            
            if (current_IN2_value != opcua_known_IN2) {
                opcua_known_IN2 = current_IN2_value;
                UA_Variant_setScalar(&data_value.value, &opcua_known_IN1, &UA_TYPES[UA_TYPES_INT32]);
                UA_Server_writeDataValue(server, UA_NODEID_STRING(1, "IN2_control"), data_value);
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
            ESP_ERROR_CHECK(esp_task_wdt_reset());
            taskYIELD();
        }
        UA_DataValue_clear(&data_value);
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
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
    sntp_initialized = true;
}

static bool obtain_time(void)
{
    initialize_sntp();
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    memset(&timeinfo, 0, sizeof(struct tm));
    int retry = 0;
    const int retry_count = 20;
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
        xTaskCreatePinnedToCore(opcua_task, "opcua_task", 16384, NULL, 6, NULL, 1);
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

    if (strlen(command) == 0) {
        return;
    }

    if (strcasecmp(command, "help") == 0) {
        print_help();
        return;
    }

    // Convert the command to lowercase for consistent processing
    char cmd_lower[64]; // Increased buffer size for safety
    strncpy(cmd_lower, command, sizeof(cmd_lower) - 1);
    cmd_lower[sizeof(cmd_lower) - 1] = '\0';
    for (int i = 0; cmd_lower[i]; i++) {
        cmd_lower[i] = tolower(cmd_lower[i]);
    }

    // --- STREAMLINED LOGIC ---
    if (strcmp(cmd_lower, "off") == 0 ||
        strcmp(cmd_lower, "release") == 0 ||
        strcmp(cmd_lower, "clear") == 0 ||
        strcmp(cmd_lower, "draw") == 0 ||
        strcmp(cmd_lower, "screen") == 0 ||
        strcmp(cmd_lower, "status") == 0 || // Treat "status" like any other command
        strncmp(cmd_lower, "in ", 3) == 0) {
        
        // All valid Pico commands are sent through the bridge function
        send_raw_uart_command(cmd_lower);

    } else {
        printf("ERROR: Unknown command '%s'\n", command);
        printf("Type 'help' for available commands\n");
    }
}

static void uart_bridge_task(void *arg)
{
    uart_config_t uart_config_device = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_DEVICE_NUM, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_DEVICE_NUM, &uart_config_device));
    ESP_ERROR_CHECK(uart_set_pin(UART_DEVICE_NUM, TXD2_PIN, RXD2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uint8_t *device_data = (uint8_t *) malloc(UART_BUF_SIZE);
    static char pc_cmd_buffer[128];
    static int pc_cmd_index = 0;

    ESP_LOGI(UART_TAG, "UART Command Processor Initialized");
    printf("\n===========================================\n");
    printf("  Pico Multiplexer Controller Ready\n");
    printf("===========================================\n");
    printf("Type 'help' for available commands\n");
    printf("> ");

    while (1) {
        char c;
        int len_pc = read(STDIN_FILENO, &c, 1);

        if (len_pc > 0) {
            if (c == '\n' || c == '\r') {
                pc_cmd_buffer[pc_cmd_index] = '\0';
                if (pc_cmd_index > 0) {
                    printf("\n");
                    process_pc_command(pc_cmd_buffer);
                }
                pc_cmd_index = 0;
                printf("> ");
                fflush(stdout);
            } else if (c == '\x7F' || c == '\x08') {
                if (pc_cmd_index > 0) {
                    printf("\b \b");
                    fflush(stdout);
                    pc_cmd_index--;
                }
            } else {
                if (pc_cmd_index < (sizeof(pc_cmd_buffer) - 1)) {
                    pc_cmd_buffer[pc_cmd_index++] = c;
                    printf("%c", c);
                    fflush(stdout);
                }
            }
        }

        int len_device = uart_read_bytes(UART_DEVICE_NUM, device_data, UART_BUF_SIZE, 20 / portTICK_PERIOD_MS);
        
        if (len_device > 0) {
            device_data[len_device] = '\0';
            char* response = (char*)device_data;
            
            printf("\nPico: %s\n", response);
            
            char IN1_str[16], IN2_str[16];
            int parsed = sscanf(response, "IN1=%15s IN2=%15s", IN1_str, IN2_str);

            if (parsed == 2) {
                if (strcmp(IN1_str, "OFF") == 0) {
                    current_IN1_value = -1;
                } else {
                    current_IN1_value = atoi(IN1_str);
                }
                
                if (strcmp(IN2_str, "OFF") == 0) {
                    current_IN2_value = -1;
                } else {
                    current_IN2_value = atoi(IN2_str);
                }
                
                ESP_LOGI(UART_TAG, "Status updated: IN1=%d, IN2=%d",
                         current_IN1_value, current_IN2_value);
            }
            
            update_uart_status(response);
            
            //printf("> ");
            fflush(stdout);
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    
    free(device_data);
    uart_driver_delete(UART_DEVICE_NUM);
}

void app_main(void)
{
    ++boot_count;
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