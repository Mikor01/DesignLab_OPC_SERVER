#include "open62541.h"

/* GPIO Numbers */
#define DHT22_GPIO 4
#define RELAY_0_GPIO 32
#define RELAY_1_GPIO 33

/* Temperature */
UA_StatusCode
readCurrentTemperature(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue);

void
addCurrentTemperatureDataSourceVariable(UA_Server *server);

/* Relay 0 */
UA_StatusCode
readRelay0State(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue);

UA_StatusCode
setRelay0State(UA_Server *server,
                  const UA_NodeId *sessionId, void *sessionContext,
                  const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data);

void
addRelay0ControlNode(UA_Server *server);

/* Relay 1 */
UA_StatusCode
readRelay1State(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue);

UA_StatusCode
setRelay1State(UA_Server *server,
                  const UA_NodeId *sessionId, void *sessionContext,
                  const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data);

void
addRelay1ControlNode(UA_Server *server);

/* UART Command Functions */
void send_uart_command_from_opcua(const char *cmd, int value);

/* IN1 Control */
UA_StatusCode
readIN1Value(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *nodeId, void *nodeContext,
             UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
             UA_DataValue *dataValue);

UA_StatusCode
writeIN1Value(UA_Server *server,
              const UA_NodeId *sessionId, void *sessionContext,
              const UA_NodeId *nodeId, void *nodeContext,
              const UA_NumericRange *range, const UA_DataValue *data);

void
addIN1ControlNode(UA_Server *server);

/* IN2 Control */
UA_StatusCode
readIN2Value(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *nodeId, void *nodeContext,
             UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
             UA_DataValue *dataValue);

UA_StatusCode
writeIN2Value(UA_Server *server,
              const UA_NodeId *sessionId, void *sessionContext,
              const UA_NodeId *nodeId, void *nodeContext,
              const UA_NumericRange *range, const UA_DataValue *data);

void
addIN2ControlNode(UA_Server *server);

/* UART Status */
UA_StatusCode
readUARTStatus(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
               UA_DataValue *dataValue);

void
addUARTStatusNode(UA_Server *server);

