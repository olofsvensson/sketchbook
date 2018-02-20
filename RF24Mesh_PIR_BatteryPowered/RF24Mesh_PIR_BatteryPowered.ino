
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
#include "LowPower.h"


/*** PIR sensor ***/
#define PIRPIN 2

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
#define nodeID 6

/*** DHT 22 config */

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

/*** Timer for sending new values */
uint32_t displayTimer = 0;

/*** Message structure */
struct payload_t { 
  unsigned long nodeId;
  char topic[12];
  char message[12];
};

payload_t payload;

void setup() {
  // Set up the led pin
  pinMode(PIRPIN, INPUT);     
  // Increase the payload size from default 32 to 72
  //radio.setPayloadSize(72);
  //Serial.begin(115200);
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  //Serial.println(F("Connecting to the mesh..."));
  dht.begin();
  mesh.begin();
}

void sendTopicAndMessage(String topic, String message) {
  payload.nodeId = nodeID;
  topic.toCharArray(payload.topic, topic.length()+1);
  message.toCharArray(payload.message, message.length()+1);
  //Serial.println(payload.nodeId);
  //Serial.println(payload.topic);
  //Serial.println(payload.message);
  //Serial.println(sizeof(payload));
  //Serial.println(F("Sending...\r\n"));
  // Send an 'D' type message containing the current millis()
  if (!mesh.write(&payload, 'D', sizeof(payload))) {
    // If a write fails, check connectivity to the mesh network
    if ( ! mesh.checkConnection() ) {
      //refresh the network address
      //Serial.println("Renewing Address");
      mesh.renewAddress();
    }// else {
      //Serial.println("Send fail, Test OK");
    //}
  }// else {
   // Serial.print("Send OK: "); Serial.println(displayTimer);
  //}
}


float hum;  //Stores humidity value
float temp; //Stores temperature value
float voltage_reading;
unsigned int i, reading, presence;
String topic, message;
bool presenceDetected;
static char hum_string[12];
static char temp_string[12];

long readVcc() {
  // From: https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

/**
void wakeUp()
{
    // Just a handler for the pin interrupt.
    // PIR readout
    radio.powerUp();
    presence = digitalRead(PIRPIN);    
    topic = "presence";
    message = String(presence);
    sendTopicAndMessage(topic, message);
} */

void loop() {


  // Send to the master node 
    
    topic = "pres";
    presenceDetected = false;
    
    for (int index=0; index < 30; index++) {
      presence = digitalRead(PIRPIN);
      if (presence == HIGH && !presenceDetected) {    
        radio.powerUp();
        message = String(1);
        mesh.update();
        delay(100);
        sendTopicAndMessage(topic, message);
        presenceDetected = true;
        radio.powerDown();
      }
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);  
    }
    if (!presenceDetected) {
        radio.powerUp();
        message = String(0);
        mesh.update();
        delay(100);
        sendTopicAndMessage(topic, message);
        radio.powerDown();
    }
        
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);  
    radio.powerUp();

    hum = dht.readHumidity();
    temp= dht.readTemperature();
    dtostrf(hum, 4, 1, hum_string);
    dtostrf(temp, 4, 1, temp_string);

    topic = "temp";
    message = String(temp_string);
    sendTopicAndMessage(topic, message);
    delay(100);

    topic = "humi";
    message = String(hum_string);
    sendTopicAndMessage(topic, message);
    delay(100);   
    
    topic = "batt";
    message = String(readVcc()/1000.0);
    mesh.update();
    delay(100);
    sendTopicAndMessage(topic, message); 
    radio.powerDown();

}






