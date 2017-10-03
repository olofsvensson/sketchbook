
/** RF24Mesh_Example.ino by TMRh20

   This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
   master node.
   The nodes will refresh their network address as soon as a single write fails. This allows the
   nodes to change position in relation to each other and the master node.
*/


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <DHT.h>
//#include <printf.h>

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
   User Configuration: nodeID - A unique identifier for each radio. Allows addressing
   to change dynamically with physical changes to the mesh.

   In this example, configuration takes place below, prior to uploading the sketch to the device
   A unique value from 1-255 must be configured for each node.
   This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.

 **/
#define nodeID 1


uint32_t displayTimer = 0;

struct payload_t {                  // Structure of our payload
  unsigned long nodeId;
  unsigned long topic_length;
  unsigned long message_length;
  char topic[32];
  char message[32];
};

payload_t payload;

void setup() {

  radio.setPayloadSize(72);
  Serial.begin(115200);
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
}

void sendTopicAndMessage(String topic, String message) {
  payload.nodeId = 1;
  payload.topic_length = topic.length();
  payload.message_length = message.length();
  topic.toCharArray(payload.topic, topic.length()+1);
  message.toCharArray(payload.message, message.length()+1);
  Serial.println(payload.nodeId);
  Serial.println(payload.topic_length);
  Serial.println(payload.topic);
  Serial.println(payload.message_length);
  Serial.println(payload.message);
  Serial.println(sizeof(payload));
  Serial.println(F("Sending...\r\n"));
  // Send an 'D' type message containing the current millis()
  if (!mesh.write(&payload, 'D', sizeof(payload))) {
    // If a write fails, check connectivity to the mesh network
    if ( ! mesh.checkConnection() ) {
      //refresh the network address
      Serial.println("Renewing Address");
      mesh.renewAddress();
    } else {
      Serial.println("Send fail, Test OK");
    }
  } else {
    Serial.print("Send OK: "); Serial.println(displayTimer);
  }
}


float hum;  //Stores humidity value
float temp; //Stores temperature value
const float voltage_reference = 3.7; // 5.0V
const int num_measurements = 64;
const int voltage_pin = A0;
float voltage_reading;
unsigned int i, reading;
unsigned long thisNode = 1;
String topic, message;

static char hum_string[15];
static char temp_string[15];

void loop() {

  mesh.update();

  // Send to the master node every 10 seconds
  if (millis() - displayTimer >= 10000) {
    displayTimer = millis();

    // Humidity and temperature
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    dtostrf(hum, 4, 1, hum_string);
    dtostrf(temp, 4, 1, temp_string);

    // Take the voltage reading 
    i = num_measurements;
    reading = 0;
    while(i--)
      reading += analogRead(voltage_pin);
 
    voltage_reading = (float)reading / num_measurements * voltage_reference / 512.0;
    
    topic = "saloon/temperature";
    message = String(temp_string);
    sendTopicAndMessage(topic, message);

    topic = "saloon/humidity";
    message = String(hum_string);
    sendTopicAndMessage(topic, message);

  }
}






