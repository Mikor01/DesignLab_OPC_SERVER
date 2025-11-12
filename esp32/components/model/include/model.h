/*
 * Header file for the OPC-UA server data model.
 * Defines nodes, data sources, and public variables.
 */
#ifndef MODEL_H
#define MODEL_H

#include "open62541.h"

/*
 * Global variables holding the current state for OPC-UA.
 * Declared here as 'extern' and defined in model.c.
 * 'volatile' is used as they are modified in the UART
 * task and read in the OPC-UA task.
 */
extern volatile UA_Int32 current_IN0_value;
extern volatile UA_Int32 current_IN1_value;
extern UA_String last_uart_status;


/*
 * Public functions to add nodes to the OPC-UA server.
 * Called from opcua_task in opcua_esp32.c
 */

/**
 * @brief Adds the IN0 control node (from UART) to the OPC-UA server.
 */
void addIN0ControlNode(UA_Server *server);

/**
 * @brief Adds the IN1 control node (from UART) to the OPC-UA server.
 */
void addIN1ControlNode(UA_Server *server);

/**
 * @brief Adds the (read-only) node for the last UART status.
 */
void addUARTStatusNode(UA_Server *server);

/**
 * @brief Updates the global UA_String (last_uart_status)
 * with a new status from UART.
 * @param new_status C-string with the new status.
 */
void update_uart_status(const char* new_status);


#endif // MODEL_H