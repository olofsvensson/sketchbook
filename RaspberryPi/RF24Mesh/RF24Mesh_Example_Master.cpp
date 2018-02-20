 
 
 /** RF24Mesh_Example_Master.ino by TMRh20
  * 
  * Note: This sketch only functions on -Arduino Due-
  *
  * This example sketch shows how to manually configure a node via RF24Mesh as a master node, which
  * will receive all data from sensor nodes.
  *
  * The nodes can change physical or logical position in the network, and reconnect through different
  * routing nodes as required. The master node manages the address assignments for the individual nodes
  * in a manner similar to DHCP.
  *
  */
  
#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <unistd.h>

#include "RF24Mesh/RF24Mesh.h"  
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>


RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  
RF24Network network(radio);
RF24Mesh mesh(radio,network);

struct payload_t {                  // Structure of our payload
  unsigned long nodeId;
  char topic[12];
  char message[12];
};


void mosq_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
  
  switch(level){
    //case MOSQ_LOG_DEBUG:
    //case MOSQ_LOG_INFO:
    //case MOSQ_LOG_NOTICE:
    case MOSQ_LOG_WARNING:
    case MOSQ_LOG_ERR: {
      printf("%i:%s\n", level, str);
    }
  }
  
	
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

        payload_t payload;
        payload.nodeId = 0;
        strcpy(payload.topic, message->topic);
        strcpy(payload.message, (char*) message->payload);
        mesh.update();
        if (!mesh.write(&payload, 'D', sizeof(payload), 5)) {
           printf("Send failed!");
        } else {
           printf("Send with success!");
	}
	//mosquitto_topic_matches_sub("arduino/saloon/+", message->topic, &match);
	//if (match) {
	//	printf("got message for saloon topic\n");
	//}

}

struct mosquitto *mosq = NULL;
void mqtt_setup(){

	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
  
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, clean_session, NULL);
  if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		exit(1);
	}
  
  mosquitto_log_callback_set(mosq, mosq_log_callback);
  mosquitto_message_callback_set(mosq, message_callback);

  
  if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		exit(1);
	}
  mosquitto_subscribe(mosq, NULL, "arduino/+", 0);
  int loop = mosquitto_loop_start(mosq);
  if(loop != MOSQ_ERR_SUCCESS){
    fprintf(stderr, "Unable to start loop: %i\n", loop);
    exit(1);
  }
}

int mqtt_send(char* topic, char *msg){
  return mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 0, 0);
}

int snd;

int main(int argc, char** argv) {
  
  mqtt_setup();
  // radio.setPayloadSize(72);
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  // Connect to the mesh
  printf("start\n");
  mesh.begin();
  radio.printDetails();
  char topic[64];

while(1)
{
  
  // Call network.update as usual to keep the network updated
  mesh.update();

  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  while(network.available()){
//    printf("rcv\n");
    RF24NetworkHeader header;
    network.peek(header);
    payload_t payload; 

    uint32_t dat=0;
    switch(header.type){
      // Display the incoming millis() values from the sensor nodes
      case 'M': network.read(header,&dat,sizeof(dat)); 
                printf("Rcv %u from 0%o\n",dat,header.from_node);
                 break;
      case 'D': network.read(header,&payload,sizeof(payload)); 
		printf("Received payload # %d | %s | %s\n",(int)payload.nodeId, \
                       payload.topic, payload.message);
                sprintf(topic, "%lu", payload.nodeId);
                strcat(topic, "/");
                strcat(topic, payload.topic);
                snd = mqtt_send(topic, payload.message);
                if(snd != 0) printf("mqtt_send error=%i\n", snd);
                break;
      default:  network.read(header,0,0); 
                printf("Rcv bad type %d from 0%o\n",header.type,header.from_node); 
                break;
    }
  }
delay(2);
  }
return 0;
}

      
      
      
