
#include "open62541.h"
#include "model.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>


/*
 * Global variables, defined in model.h
 * These are volatile as they are written by the UART task
 * and read by the OPC-UA task.
 */
volatile UA_Int32 current_IN0_value = 0;
volatile UA_Int32 current_IN1_value = 0;
UA_String last_uart_status = {0, NULL};


// This is defined in opcua_esp32.c
extern const int UART_DEVICE_NUM;

/**
 * @brief Sends a Pico-format command to Arduino via UART.
 * @param input_channel (0 or 1)
 * @param output_value (-1=OFF, 0-15=OUT)
 */
void send_uart_command_from_opcua(int input_channel, int output_value) {
    char buffer[64];
    int len;
   
    if (output_value == -1) {
        // "in <0-1> off"
        len = snprintf(buffer, sizeof(buffer), "in %d off\n", input_channel);
    } else if (output_value >= 0 && output_value < 16) {
        // "in <0-1> out <0-15>"
        len = snprintf(buffer, sizeof(buffer), "in %d out %d\n", input_channel, output_value);
    } else {
        // Invalid value
        return;
    }
   
    // Send command to Arduino
    uart_write_bytes(UART_DEVICE_NUM, buffer, len);
   
    // Short delay for processing
    vTaskDelay(200 / portTICK_PERIOD_MS); // ZWIĘKSZONO z 50ms do 200ms
   
    // Request status update
    const char *status_cmd = "STATUS\n";
    uart_write_bytes(UART_DEVICE_NUM, status_cmd, strlen(status_cmd));
} 


/* --- IN0 Control Node --- */
UA_StatusCode
readIN0Value(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *nodeId, void *nodeContext,
             UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
             UA_DataValue *dataValue) {
    UA_Variant_setScalarCopy(&dataValue->value, &current_IN0_value,
                             &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
writeIN0Value(UA_Server *server,
              const UA_NodeId *sessionId, void *sessionContext,
              const UA_NodeId *nodeId, void *nodeContext,
              const UA_NumericRange *range, const UA_DataValue *data) {
    if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }
   
    UA_Int32 value = *(UA_Int32*)data->value.data;
   
    // -1=OFF, 0-15=OUT
    if (value <= -1 || value > 15) {
        return UA_STATUSCODE_BADOUTOFRANGE;
    }
   
    UA_Int32 target_value = value;
    UA_Int32 old_value = current_IN0_value;

    // If value is already set, do nothing
    if (old_value == target_value) {
        return UA_STATUSCODE_GOOD;
    }

    // Send command and request status
    send_uart_command_from_opcua(1, target_value);
   
    // Wait for uart_bridge_task to update the global variable
    // Timeout after ~2 seconds (40 * 50ms)
    int retries = 40; // ZWIĘKSZONO z 20 do 40
    while(retries-- > 0 && current_IN0_value == old_value) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        // This yield allows uart_bridge_task to run and parse the reply
    }

    // Check if the update was successful
    if (current_IN0_value == old_value) {
        // Value did not change, timeout occurred
        return UA_STATUSCODE_BADTIMEOUT;
    }
   
    // Value changed, return GOOD. Server will now send notification.
    return UA_STATUSCODE_GOOD;
}

void
addIN0ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "IN0 Control");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN0 output (-1=OFF, 0-15=OUTPUT)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
   
    UA_Int32 initialValue = 0;
    UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "IN0_control");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "IN0 Control");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource IN0DataSource;
    IN0DataSource.read = readIN0Value;
    IN0DataSource.write = writeIN0Value;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        IN0DataSource, NULL, NULL);
}


/* --- IN1 Control Node --- */
UA_StatusCode
readIN1Value(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *nodeId, void *nodeContext,
             UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
             UA_DataValue *dataValue) {
    UA_Variant_setScalarCopy(&dataValue->value, &current_IN1_value,
                             &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
writeIN1Value(UA_Server *server,
              const UA_NodeId *sessionId, void *sessionContext,
              const UA_NodeId *nodeId, void *nodeContext,
              const UA_NumericRange *range, const UA_DataValue *data) {
    if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }
   
    UA_Int32 value = *(UA_Int32*)data->value.data;
   
    // -1=OFF, 0-15=OUT
    if (value <= -1 || value > 15) {
        return UA_STATUSCODE_BADOUTOFRANGE;
    }

    UA_Int32 target_value = value;
    UA_Int32 old_value = current_IN1_value;

    // If value is already set, do nothing
    if (old_value == target_value) {
        return UA_STATUSCODE_GOOD;
    }

    // Send command and request status
    send_uart_command_from_opcua(2, target_value);
   
    // Wait for uart_bridge_task to update the global variable
    // Timeout after ~2 seconds (40 * 50ms)
    int retries = 40; // ZWIĘKSZONO z 20 do 40
    while(retries-- > 0 && current_IN1_value == old_value) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        // This yield allows uart_bridge_task to run and parse the reply
    }

    // Check if the update was successful
    if (current_IN1_value == old_value) {
        // Value did not change, timeout occurred
        return UA_STATUSCODE_BADTIMEOUT;
    }
   
    // Value changed, return GOOD. Server will now send notification.
    return UA_STATUSCODE_GOOD;
}

void
addIN1ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "IN1 Control");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN1 output (-1=OFF, 0-15=OUTPUT)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
   
    UA_Int32 initialValue = 0;
    UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "IN1_control");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "IN1 Control");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource IN1DataSource;
    IN1DataSource.read = readIN1Value;
    IN1DataSource.write = writeIN1Value;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        IN1DataSource, NULL, NULL);
}


/* --- UART Status Node (Read Only) --- */
UA_StatusCode
readUARTStatus(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
               UA_DataValue *dataValue) {
    if (last_uart_status.length > 0) {
        UA_Variant_setScalarCopy(&dataValue->value, &last_uart_status,
                                 &UA_TYPES[UA_TYPES_STRING]);
    } else {
        UA_String emptyStatus = UA_STRING("No status received");
        UA_Variant_setScalarCopy(&dataValue->value, &emptyStatus,
                                 &UA_TYPES[UA_TYPES_STRING]);
    }
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

void
addUARTStatusNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "UART Status");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Last status from Arduino (format: IN0=X IN1=Y)");
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "uart_status");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "UART Status");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource statusDataSource;
    statusDataSource.read = readUARTStatus;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        statusDataSource, NULL, NULL);
}

/**
 * @brief Updates the internal UA_String used by the OPC-UA status node.
 * This is called by the UART bridge task.
 */
void update_uart_status(const char* new_status) {
    // Clear old
    UA_String_clear(&last_uart_status);
   
    // Set new
    last_uart_status = UA_String_fromChars(new_status);
    }