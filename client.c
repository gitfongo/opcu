#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "open62541.h"

// const char* ADDR = "opc.tcp://localhost:4840";
const char* ADDR = "opc.tcp://devhk.rocketdt.com:4840";

void handler_onChanged(UA_UInt32 monId, UA_DataValue *value, void *context) {
    printf("Value = %.2f\n", *(UA_Double*) value->value.data);
}

UA_Client* client = NULL;
UA_UInt32 subId = 0;
int intFlag = false;

void intHandler(int dummy) {
	intFlag = true;
}

int main(int argc, char *argv[]) {
	signal(SIGINT, intHandler);

	client = UA_Client_new(UA_ClientConfig_standard);

    /* Listing endpoints */
    UA_EndpointDescription* endpointArray = NULL;
    size_t endpointArraySize = 0;
    UA_StatusCode retval = UA_Client_getEndpoints(client, ADDR,
        &endpointArraySize, &endpointArray);

    if (retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        UA_Client_delete(client);
        return (int)retval;
	}
    UA_Array_delete(endpointArray,endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:16664"); */
    retval = UA_Client_connect(client, ADDR);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return (int)retval;
    }

    /* Read attribute */
    UA_Double value = 0;
    /* printf("\nReading the value of node (4, \"V_004\"):\n"); */
    UA_Variant *val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(4, "Var01"), val);
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
       val->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            value = *(UA_Double*)val->data;
            printf("Var01: %.2f\n", value);
    } else {
        printf("Return value = %x\n", retval);
    }
    UA_Variant_delete(val);

    /* Create a subscription */
    UA_Client_Subscriptions_new(client, UA_SubscriptionSettings_standard, &subId);
    if (subId) {
        /* printf("Create subscription succeeded, id %u\n", subId); */
    }
    /* Add a MonitoredItem */
    UA_NodeId monitorThis = UA_NODEID_STRING(4, "Var01");
    UA_UInt32 monId = 0;
    UA_Client_Subscriptions_addMonitoredItem(client, subId, monitorThis, UA_ATTRIBUTEID_VALUE,
                                             &handler_onChanged, NULL, &monId);
    if (monId) {
        /* printf("Monitoring '.V_004', id %u\n", subId); */
    }
    /* The first publish request should return the initial value of the variable */
    for (int i = 0; i < 1000; i++) {
    	if (!intFlag) {
			UA_Client_Subscriptions_manuallySendPublishRequest(client);
		} else {
			break;
		}
	    usleep(1000);
	}

	if (!UA_Client_Subscriptions_remove(client, subId)) {
    	printf("Subscription removed\n");
   	}

    printf("Disconnect client\n");
	UA_Client_disconnect(client);
    UA_Client_delete(client);

    return (int) UA_STATUSCODE_GOOD;
}
