// Adafruit IO Publish Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <dht.h>
#include <NewPing.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define DHT11_PIN 2    //D4

dht DHT;
NewPing sonar(13, 15, 20);

float temp;
float humid;
float prevtemp;
float prevhumid;
long currMillis;
long prevMillis;
int dist;
int prevusindex;
int usindex;
int sm;
int smindex;
int prevsmindex;

// set up the feed
AdafruitIO_Feed *c1 = io.feed("humidity");
AdafruitIO_Feed *c2 = io.feed("temperature");
AdafruitIO_Feed *c3 = io.feed("relay 1");
AdafruitIO_Feed *c4 = io.feed("relay 2");
AdafruitIO_Feed *c5 = io.feed("water level");
AdafruitIO_Feed *c6 = io.feed("soil moisture");

void setup() {

  // start the serial connection
  Serial.begin(115200);
  prevMillis=long(0);
  prevtemp=0;
  prevhumid=0;
  usindex=0;
  prevusindex=0;
  prevsmindex=0;

  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // wait for serial monitor to open
  while(! Serial);
  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();


  c4->onMessage(handle_r2_Message);
  c4->get();

  // wait for a connection
  while(io.mqttStatus() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  pinMode(14,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(15,INPUT);
  pinMode(16,OUTPUT);
  pinMode(0,INPUT);

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  lcd.setCursor(0, 0);
  lcd.print("IoT Farming");
  lcd.setCursor(0, 1);
  lcd.print("Code running...");
  delay(2000);
  lcd.clear();
}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
  

  //DHT11
  currMillis = (long)(millis());
  if (currMillis - prevMillis >= 30000) {
  int chk = DHT.read11(DHT11_PIN);
  temp = DHT.temperature;
  humid = DHT.humidity;
  if (prevtemp != temp) {
    c2->save(temp);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp= ");
    lcd.print(temp);
    Serial.print(temp);
    prevtemp=temp;
  }
  if (prevhumid != humid) {
    c1->save(humid);
    lcd.setCursor(0, 1);
    lcd.print("Humid= ");
    lcd.print(humid);
    Serial.println(humid);
    prevhumid=humid;
  }
  prevMillis = currMillis;
  }

  //Ultrasonic
  currMillis = (long)(millis());
  if (currMillis - prevMillis >= 5000){
    dist = (int)(sonar.ping_cm());
    if (dist > 0 && dist <= 5) {
      usindex = 3;
    } else if (dist > 5 && dist <= 10) {
      usindex = 2;
    } else if (dist > 10) {
      usindex = 1;
    }
    if (prevusindex!=usindex){
      c5->save(usindex);
      Serial.println(usindex);
      if (usindex==1){
        digitalWrite(16, HIGH);
      }
      else
      {
        digitalWrite(16, LOW);
      }
      prevusindex=usindex;
    }
    prevMillis=currMillis;
  }

  //Soil Moisture
  currMillis = (long)(millis());
  if (currMillis - prevMillis >= 5000){
    sm = digitalRead(0);
    if (sm== HIGH) {
      smindex = 1;
    } else if (sm== LOW) {
      smindex = 0;
    }
    if (prevsmindex != smindex){
      c6->save(smindex);
      if (smindex==0){
        digitalWrite(14, HIGH);
        c3->save(smindex);
  }
  else if(smindex==1){
    digitalWrite(14,LOW);
    c3->save(smindex);
  }
      Serial.println(smindex);
      prevsmindex=smindex;
    }
    prevMillis=currMillis;
  }
}
// Adafruit IO is rate limited for publishing, so a delay is required in
// between feed->save events.



//Relay Functions
void handle_r2_Message(AdafruitIO_Data *data) {

  int reading1= data->toInt();
  Serial.print("received <- ");
  Serial.println(reading1);
  if (reading1 == 0){
    digitalWrite(12, HIGH);
  }
  else if (reading1 == 1){
    digitalWrite(12,LOW);
  }
}
