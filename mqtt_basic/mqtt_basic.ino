/*
 *************************************************************************
   RF24Ethernet Arduino library by TMRh20 - 2014-2015

   Automated (mesh) wireless networking and TCP/IP communication stack for RF24 radio modules

   RF24 -> RF24Network -> UIP(TCP/IP) -> RF24Ethernet
                       -> RF24Mesh

        Documentation: http://tmrh20.github.io/RF24Ethernet/

 *************************************************************************
 *
 **** EXAMPLE REQUIRES: PubSub MQTT library: https://github.com/knolleary/pubsubclient ***
 * 
 * Install using the Arduino library manager
 * 
 *************************************************************************
  Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <RF24Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

float hum;  //Stores humidity value
float temp; //Stores temperature value

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

RF24 radio(9,10);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
RF24EthernetClass RF24Ethernet(radio,network,mesh);

IPAddress ip(10,10,2,4);
IPAddress gateway(10,10,2,2); //Specify the gateway in case different from the server
IPAddress server(10,10,2,2);

char *clientID = {"arduinoClient "};

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String levelTmp = String((char *)payload);
  String level = levelTmp.substring(0,length);
  Serial.print(level);
  Serial.println();
  pinMode(led, OUTPUT);     
  if (level.equals("LOW")) {
    digitalWrite(led, LOW);
    Serial.print("LOW");
    Serial.println();
  } else {
    digitalWrite(led, HIGH);
    Serial.print("HIGH");
    Serial.println();
  } 
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("saloon/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(led, OUTPUT);     

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(ip);
  Ethernet.set_gateway(gateway);
  if (mesh.begin()) {
    Serial.println(" OK");
  } else {
    Serial.println(" Failed");
  }
  clientID[13] = ip[3] + 48; //Convert last octet of IP to ascii & use in clientID
}

uint32_t mesh_timer = 0;
uint32_t mesh_timer2 = 0;
static char hum_string[15];
static char temp_string[15];

void loop()
{
  if(millis()-mesh_timer > 30000){ //Every 30 seconds, test mesh connectivity
    mesh_timer = millis();
    if( ! mesh.checkConnection() ){
        mesh.renewAddress();
     }
  }  
  if (!client.connected()) {
    reconnect();
  } else {
    client.loop();
    if(millis()-mesh_timer2 > 10000){ //Every 10 seconds, send temp + humid
      mesh_timer2 = millis();
      // Humidity and temperature
      hum = dht.readHumidity();
      temp= dht.readTemperature();    
      dtostrf(hum, 4, 1, hum_string);
      dtostrf(temp, 4, 1, temp_string);
      Serial.println(temp_string);
      Serial.println(hum_string);
      Serial.println(F("Sending...\r\n"));
      client.publish("saloon/temperature", temp_string);
      client.publish("saloon/humidity", hum_string);
    }
  } 
}
