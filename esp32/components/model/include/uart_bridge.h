#ifndef UART_BRIDGE_H // Renamed from UART_COMMANDS_H for consistency
#define UART_BRIDGE_H

// THIS IS THE FIX: Include the header that defines UA_String, UA_Int32, etc.
#include "open62541.h"

// Declare the global variables that will be defined in uart_bridge.c
extern volatile UA_Int32 current_IN1_value;
extern volatile UA_Int32 current_IN2_value;

/**
 * @brief Sends a command to the device connected via UART.
 */
void send_uart_command_from_opcua(int input_channel, int output_value);

/**
 * @brief Updates the UART status string for the OPC-UA server.
 */
void update_uart_status(const char* new_status);

/**
 * @brief Returns the last received UART status string.
 */
UA_String get_last_uart_status(void);

/**
 * @brief Sends a raw, pre-formatted command string to the UART device.
 * Automatically appends a newline and requests status afterwards.
 * @param command The command string to send (e.g., "clear", "in 0 out 5").
 */
void send_raw_uart_command(const char *command);

void print_help(void);

#endif // UART_BRIDGE_H