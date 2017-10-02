/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
//#include "LowPower.h"
#include <DHT.h>

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

int serial_putc( char c, FILE * ) 
{
  Serial.write( c );

  return c;
} 

void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}

//
// Hardware configuration
//

#define PIN_CS 10
#define PIN_CE 9

const float voltage_reference = 3.7; // 5.0V
const int num_measurements = 64;
const int voltage_pin = A0;
float voltage_reading;
unsigned int i, reading;

RF24 radio(PIN_CE, PIN_CS);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

// How often to send 'hello world to the other unit
const unsigned long interval = 2000; //ms

// When did we last send?
unsigned long last_sent;

unsigned long thisNode = 1;
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

struct payload_t {                  // Structure of our payload
  unsigned long nodeId;
  float supplyVoltage;
  float temperature;
  float humidity;
};

void setup(void)
{
  Serial.begin(115200);
  dht.begin();
  printf_begin();
  printf("RF24Network/examples/helloworld_tx/\n");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90,  /*node address*/ this_node);
  radio.printDetails();
}

void loop(void)
{
    network.update();  

    // Humidity and temperature
    hum = dht.readHumidity();
    temp= dht.readTemperature();

    // Take the voltage reading 
    i = num_measurements;
    reading = 0;
    while(i--)
      reading += analogRead(voltage_pin);
 
    voltage_reading = (float)reading / num_measurements * voltage_reference / 512.0;
    Serial.println(voltage_reading);
    Serial.println(temp);
    Serial.println(hum);
    Serial.println(F("Sending...\r\n"));
    payload_t payload = {thisNode, voltage_reading, temp, hum };
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header,&payload,sizeof(payload));

    if (ok)
      Serial.println(F("\tok.\r\n"));
    else
    {
      Serial.println(F("\tfailed.\r\n"));
      delay(250); // extra delay on fail to keep light on longer
    }
    delay(2000);  
    radio.powerDown();
    delay(2000);  
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
    //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
    radio.powerUp();
    thisNode++;
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
