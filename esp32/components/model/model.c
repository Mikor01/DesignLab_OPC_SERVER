#include "model.h"
#include "uart_bridge.h"
#define TAG "OPCUA_ESP32"
// This is defined in opcua_esp32.c
extern const int UART_DEVICE_NUM;

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
   
    // -1=OFF, 1-16=OUT
    if (value <= -2 || value > 16 || value == 0) {
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
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN1 output (-1=OFF, 1-16=OUTPUT)");
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
   
    // -1=OFF, 1-16=OUT
    if (value <= -2 || value > 16 || value == 0) {
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
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN2 output (-1=OFF, 1-16=OUTPUT)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
   
    UA_Int32 initialValue = 0;
    UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "IN2_control");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "IN2 Control");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource IN2DataSource;
    IN2DataSource.read = readIN2Value;
    IN2DataSource.write = writeIN2Value;
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        IN2DataSource, NULL, NULL);
}


/* --- UART Status Node (Read Only) --- */
UA_StatusCode
readUARTStatus(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
               UA_DataValue *dataValue) {

    UA_String last_uart_status = get_last_uart_status();

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

UA_StatusCode
writeUARTStatus(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                const UA_NumericRange *range, const UA_DataValue *data) {
    
    if (data->value.type != &UA_TYPES[UA_TYPES_STRING]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    UA_String *val = (UA_String*)data->value.data;
    
    char *cmd = (char*)UA_malloc(val->length + 1);
    if (!cmd) return UA_STATUSCODE_BADOUTOFMEMORY;
    
    memcpy(cmd, val->data, val->length);
    cmd[val->length] = '\0';


    send_raw_uart_command(cmd);
    
    UA_free(cmd);
    return UA_STATUSCODE_GOOD;
}

void
addUARTStatusNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "UART Status");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Last status or send custom command");
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    

    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "uart_status");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "UART Status");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource statusDataSource;
    statusDataSource.read = readUARTStatus;

    statusDataSource.write = writeUARTStatus; 
   
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        statusDataSource, NULL, NULL);
}