    
/*
 * Header file for the OPC-UA server data model.
 */
#ifndef MODEL_H
#define MODEL_H

#include "open62541.h"

/*
 * REMOVE THE OLD DECLARATIONS. These are now in uart_bridge.h
 *
 * extern volatile UA_Int32 current_IN0_value;
 * extern volatile UA_Int32 current_IN1_value;
 * extern UA_String last_uart_status;
 * void update_uart_status(const char* new_status);
 */


/*
 * Public functions to add nodes to the OPC-UA server.
 * Called from opcua_task in opcua_esp32.c
 */

/**
 * @brief Adds the IN0 control node to the OPC-UA server.
 */
void addIN0ControlNode(UA_Server *server);

/**
 * @brief Adds the IN1 control node to the OPC-UA server.
 */
void addIN1ControlNode(UA_Server *server);

/**
 * @brief Adds the read-only node for the last UART status.
 */
void addUARTStatusNode(UA_Server *server);

#endif // MODEL_H

  