#include "open62541.h"
#include "model.h"
#include "DHT22.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>
#include <stdio.h>

UA_Int32 current_IN1_value = 0;
UA_Int32 current_IN2_value = 0;
static UA_String last_uart_status = {0, NULL};

extern const int UART_DEVICE_NUM; 

static void configureGPIO();


static void configureGPIO(void) {
    gpio_set_direction(RELAY_0_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(RELAY_1_GPIO, GPIO_MODE_INPUT_OUTPUT);
}


UA_StatusCode
readCurrentTemperature(UA_Server *server,
                       const UA_NodeId *sessionId, void *sessionContext,
                       const UA_NodeId *nodeId, void *nodeContext,
                       UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                       UA_DataValue *dataValue) {
    f; 
    
    UA_Float temperature = ReadTemperature(DHT22_GPIO);
    UA_Variant_setScalarCopy(&dataValue->value, &temperature,
                             &UA_TYPES[UA_TYPES_FLOAT]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

void
addCurrentTemperatureDataSourceVariable(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Temperature");
    attr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "temperature");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "Ambient Temperature");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource timeDataSource;
    timeDataSource.read = readCurrentTemperature;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        timeDataSource, NULL, NULL);
}

/* Relay 0 */
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
    if (data->value.type != &UA_TYPES[UA_TYPES_BOOLEAN]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }
    UA_Boolean newState = *(UA_Boolean*)data->value.data;
    

    gpio_set_level(RELAY_0_GPIO, newState ? 1 : 0); 
    

    return UA_STATUSCODE_GOOD;
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

/* Relay 1 */
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
    if (data->value.type != &UA_TYPES[UA_TYPES_BOOLEAN]) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }
    UA_Boolean newState = *(UA_Boolean*)data->value.data;
    
  
    gpio_set_level(RELAY_1_GPIO, newState ? 1 : 0); 
    

    return UA_STATUSCODE_GOOD;
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

/* UART Command Functions */
void send_uart_command_from_opcua(const char *cmd, int value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "SET %s %d\n", cmd, value);
    uart_write_bytes(UART_DEVICE_NUM, buffer, strlen(buffer));
}

/* IN1 Control Node */
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
    
  
    if (value < 1 || value > 12) {
        return UA_STATUSCODE_BADOUTOFRANGE; 
    }
    
    
    send_uart_command_from_opcua("IN1", value);
    
    
    current_IN1_value = value;
    
    return UA_STATUSCODE_GOOD;
}

void
addIN1ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "IN1 Control");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN1 output (1-12)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    
    UA_Int32 initialValue = 0;
    current_IN1_value = initialValue; 
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
                                        parentNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        in1DataSource, NULL, NULL);
}

/* IN2 Control Node */
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
    
    if (value < 1 || value > 12) {
        return UA_STATUSCODE_BADOUTOFRANGE; 
    }
    
    
    send_uart_command_from_opcua("IN2", value);
    
    
    current_IN2_value = value;
    
    return UA_STATUSCODE_GOOD;
}

void
addIN2ControlNode(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "IN2 Control");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Control IN2 output (1-12)");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    
    UA_Int32 initialValue = 0;
    current_IN2_value = initialValue; 
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
                                        parentNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        in2DataSource, NULL, NULL);
}


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
    attr.description = UA_LOCALIZEDTEXT("en-US", "Last status from Arduino");
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
                                        parentNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        statusDataSource, NULL, NULL);
}

void update_uart_status_string(const char* new_status) {
   
    if (last_uart_status.data != NULL) {
        UA_String_clear(&last_uart_status);
    }
    last_uart_status = UA_String_fromChars(new_status);
}


void update_in_values_from_status(const char* status_response) {
    int in1_val = -1;
    int in2_val = -1;
    
    
    int result = sscanf(status_response, "IN1=%d IN2=%d", &in1_val, &in2_val);

  
    if (result == 2) {
      
        if (in1_val >= 1 && in1_val <= 12) {
            current_IN1_value = in1_val;
        }
        if (in2_val >= 1 && in2_val <= 12) {
            current_IN2_value = in2_val;
        }
    }
    
    // Zapisujemy całą odpowiedź do węzła statusu
    update_uart_status_string(status_response);
}
