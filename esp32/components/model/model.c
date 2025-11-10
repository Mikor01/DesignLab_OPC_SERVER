
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
volatile UA_Int32 current_IN1_value = 0;
volatile UA_Int32 current_IN2_value = 0;
UA_String last_uart_status = {0, NULL};


// This is defined in opcua_esp32.c
extern const int UART_DEVICE_NUM;


/* --- GPIO Configuration --- */
static void configureGPIO(void) {
    gpio_set_direction(RELAY_0_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(RELAY_1_GPIO, GPIO_MODE_INPUT_OUTPUT);
}


/* --- Relay 0 --- */
UA_StatusCode
readRelay0State(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue) {
    UA_Boolean relay0_State = gpio_get_level(RELAY_0_GPIO);
    UA_Variant_setScalarCopy(&dataValue->value, &relay0_State,
                             &UA_TYPES[UA_TYPES_BOOLEAN]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
setRelay0State(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_Boolean currentState = gpio_get_level(RELAY_0_GPIO);
    int level = currentState == true ? 0:1;
    gpio_set_level(RELAY_0_GPIO, level);
    UA_Boolean relay0_state_after_write = gpio_get_level(RELAY_0_GPIO);
    UA_StatusCode status = relay0_state_after_write == level ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADINTERNALERROR;
    return status;
}

void
addRelay0ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Relay0");
    attr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "relay0");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "Control Relay 0");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource relay0;
    configureGPIO();
    relay0.read = readRelay0State;
    relay0.write = setRelay0State;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        relay0, NULL, NULL);
}


/* --- Relay 1 --- */
UA_StatusCode
readRelay1State(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue) {
    UA_Boolean relay1_State = gpio_get_level(RELAY_1_GPIO);
    UA_Variant_setScalarCopy(&dataValue->value, &relay1_State,
                             &UA_TYPES[UA_TYPES_BOOLEAN]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
setRelay1State(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_Boolean currentState = gpio_get_level(RELAY_1_GPIO);
    int level = currentState == true ? 0:1;
    gpio_set_level(RELAY_1_GPIO, level);
    UA_Boolean relay1_state_after_write = gpio_get_level(RELAY_1_GPIO);
    UA_StatusCode status = relay1_state_after_write == level ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADINTERNALERROR;
    return status;
}

void
addRelay1ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Relay1");
    attr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "relay1");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "Control Relay 1");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource relay1;
    relay1.read = readRelay1State;
    relay1.write = setRelay1State;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        relay1, NULL, NULL);
}


/**
 * @brief Sends a Pico-format command to Arduino via UART.
 * @param input_channel (1 or 2)
 * @param output_value (-1=OFF, 0-15=OUT)
 */
void send_uart_command_from_opcua(int input_channel, int output_value) {
    char buffer[64];
    int len;
   
    if (output_value == -1) {
        // "in <1-2> off"
        len = snprintf(buffer, sizeof(buffer), "in %d off\n", input_channel);
    } else if (output_value >= 1 && output_value <= 12) {
        // "in <1-2> out <0-15>"
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
    if (value <= 0 || value >= 15) {
        return UA_STATUSCODE_BADOUTOFRANGE;
    }
   
    UA_Int32 target_value = value;
    UA_Int32 old_value = current_IN1_value;

    // If value is already set, do nothing
    if (old_value == target_value) {
        return UA_STATUSCODE_GOOD;
    }

    // Send command and request status
    send_uart_command_from_opcua(1, target_value);
   
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

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "in1_control");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "IN1 Control");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource in1DataSource;
    in1DataSource.read = readIN1Value;
    in1DataSource.write = writeIN1Value;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        in1DataSource, NULL, NULL);
}


/* --- IN2 Control Node --- */
UA_StatusCode
readIN2Value(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *nodeId, void *nodeContext,
             UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
             UA_DataValue *dataValue) {
    UA_Variant_setScalarCopy(&dataValue->value, &current_IN2_value,
                             &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
writeIN2Value(UA_Server *server,
              const UA_NodeId *sessionId, void *sessionContext,
              const UA_NodeId *nodeId, void *nodeContext,
              const UA_NumericRange *range, const UA_DataValue *data) {
    if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }
   
    UA_Int32 value = *(UA_Int32*)data->value.data;
   
    // -1=OFF, 0-15=OUT
    if (value <= 0 || value >= 15) {
        return UA_STATUSCODE_BADOUTOFRANGE;
    }

    UA_Int32 target_value = value;
    UA_Int32 old_value = current_IN2_value;

    // If value is already set, do nothing
    if (old_value == target_value) {
        return UA_STATUSCODE_GOOD;
    }

    // Send command and request status
    send_uart_command_from_opcua(2, target_value);
   
    // Wait for uart_bridge_task to update the global variable
    // Timeout after ~2 seconds (40 * 50ms)
    int retries = 40; // ZWIĘKSZONO z 20 do 40
    while(retries-- > 0 && current_IN2_value == old_value) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        // This yield allows uart_bridge_task to run and parse the reply
    }

    // Check if the update was successful
    if (current_IN2_value == old_value) {
        // Value did not change, timeout occurred
        return UA_STATUSCODE_BADTIMEOUT;
    }
   
    // Value changed, return GOOD. Server will now send notification.
    return UA_STATUSCODE_GOOD;
}

void
addIN2ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "IN2 Control");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN2 output (-1=OFF, 0-15=OUTPUT)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
   
    UA_Int32 initialValue = 0;
    UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "in2_control");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "IN2 Control");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource in2DataSource;
    in2DataSource.read = readIN2Value;
    in2DataSource.write = writeIN2Value;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        in2DataSource, NULL, NULL);
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
    attr.description = UA_LOCALIZEDTEXT("en-US", "Last status from Arduino (format: IN1=X IN2=Y)");
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