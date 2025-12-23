#include "uart_bridge.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

// Definition of the global variables
volatile UA_Int32 current_IN1_value = 0;
volatile UA_Int32 current_IN2_value = 0;

// It is accessed through get_last_uart_status().
static UA_String last_uart_status = {0, NULL};

extern const int UART_DEVICE_NUM;

/**
 * @brief Sends a Pico-format command via UART.
 * @param input_channel (1 or 2)
 * @param output_value (-1=OFF, 1-16=OUT)
 */
void send_uart_command_from_opcua(int input_channel, int output_value) {
    char command_buffer[64]; // This buffer holds the command string

    if (output_value == -1)
    {
        snprintf(command_buffer, sizeof(command_buffer), "in %d off", input_channel);
    } 
    else if (output_value >= 1 && output_value <= 16) // check here if dont work
    {
        snprintf(command_buffer, sizeof(command_buffer), "in %d out %d", input_channel, output_value);
    }
    else 
    {
        // Invalid value
        return;
    }

    // Use the new centralized function to send the command and get status
    send_raw_uart_command(command_buffer);
}

/**
 * @brief Updates the internal UA_String used by the OPC-UA status node.
 * This is called by the UART bridge task.
 */
void update_uart_status(const char* new_status) {
    // Clear old
    if (last_uart_status.data) {
        UA_String_clear(&last_uart_status);
    }
    // Set new
    last_uart_status = UA_String_fromChars(new_status);
}

/**
 * @brief Returns the last received UART status string.
 */
UA_String get_last_uart_status(void) {
    return last_uart_status;
}

void send_raw_uart_command(const char *command) {
    char buffer[128];

    // Format the command with a newline character
    int len = snprintf(buffer, sizeof(buffer), "%s\n", command);

    ESP_LOGI("UART_BRIDGE", "Sending command: %s", command);
    uart_write_bytes(UART_DEVICE_NUM, buffer, len);

    // Delay to allow the device to process the command
    vTaskDelay(150 / portTICK_PERIOD_MS);

    // Automatically request a status update
    if(strcmp(command,"status") != 0)
    {
        const char *status_cmd = "status\n";
        uart_write_bytes(UART_DEVICE_NUM, status_cmd, strlen(status_cmd));
    }
}

void print_help(void){
        printf("\n=== Available Commands ===\n");
        printf("in <1-2> out <1-16>  - Set input to output\n");
        printf("in <1-2> off          - Turn off input\n");
        printf("release               - Release controller\n");
        printf("clear                 - Clear display\n");
        printf("draw                  - Redraw display\n");
        printf("screen                - Enable screensaver\n");
        printf("status                - Get current status\n");
        printf("\nExamples:\n");
        printf("  in 1 out 5          - Set IN1 to OUT5\n");
        printf("  in 2 out 12         - Set IN2 to OUT12\n");
        printf("  in 1 off            - Turn off IN1\n");
        printf("========================================\n\n");
}