/**
 *  @filename   :   main.cpp
 *  @brief      :   Railcore Heatbed Fuse
 *  @author     :   Kevin Kessler
 *
 * Copyright (C) 2019 Kevin Kessler
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>          
#include <DNSServer.h>
//#include <ESPmDNS.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <Wire.h> 
#include "Ticker.h"
#include "railcorefuse.h"

const char *hostname = "railcorefuse";
char mqttServer[MQTT_SERVER_LENGTH];
char mqttTopic[MQTT_TOPIC_LENGTH];
uint16_t mqttPort;
bool configMode = false;
uint8_t flash_toggle=0;
volatile bool buttonLongPress = false; 
volatile uint32_t lastPressTime;

Ticker readLoop(readTemperature, 10000);

struct mqttConfig {
  uint32_t valid;
  char server[MQTT_SERVER_LENGTH];
  uint16_t port;
  char topic[MQTT_TOPIC_LENGTH]; 
};


void configModeCallback(WiFiManager *wfm) {
  Serial.println(F("Config Mode"));
  config_display();
  Serial.println(WiFi.softAPIP());
  configMode = true;
  Serial.println(wfm->getConfigPortalSSID());
}



void printMQTT() {
  Serial.print(F("Server "));
  Serial.println(mqttServer);

  Serial.print(F("Port "));
  Serial.println(mqttPort,DEC);

  Serial.print(F("Topic "));
  Serial.println(mqttTopic);
}

void readEEPROM() {
  mqttConfig conf;
  EEPROM.get(0,conf);
  
  if (conf.valid ==0xDEADBEEF) {
    strncpy(mqttServer, conf.server, MQTT_SERVER_LENGTH);
    strncpy(mqttTopic, conf.topic, MQTT_TOPIC_LENGTH);
    mqttPort=conf.port;
  }
  else {
    WiFiManager wifiManager;

    strncpy(mqttServer,"",MQTT_SERVER_LENGTH);
    strncpy(mqttTopic,"",MQTT_TOPIC_LENGTH);
    mqttPort = 1883;

    Serial.println(F("Setup WIFI Manager"));
    printMQTT();
  
    callWFM(false);
  }
}

void writeEEPROM() {
  Serial.println(F("Writing MQTT Config"));

  mqttConfig conf;

  conf.valid = 0xDEADBEEF;
  strncpy(conf.server, mqttServer, MQTT_SERVER_LENGTH);
  strncpy(conf.topic, mqttTopic, MQTT_TOPIC_LENGTH);
  conf.port = mqttPort;

  EEPROM.put(0,conf);
  EEPROM.commit();
}

void callWFM(bool connect) {
  WiFiManager wfm;

  wfm.setAPCallback(configModeCallback);

  WiFiManagerParameter mqtt_server("server", "MQTT Server", mqttServer, MQTT_SERVER_LENGTH);
  char port_string[6];
  itoa(mqttPort, port_string,10);
  WiFiManagerParameter mqtt_port("port", "MQTT port", port_string, 6);
  WiFiManagerParameter mqtt_topic("topic", "MQTT Topic", mqttTopic,MQTT_TOPIC_LENGTH);

  wfm.addParameter(&mqtt_server);
  wfm.addParameter(&mqtt_port);
  wfm.addParameter(&mqtt_topic);

  if(connect) {
    if(!wfm.autoConnect()) {
      Serial.println(F("Failed to connect and hit timeout"));
      error_display("Failed to connect and hit timeout");
      ESP.restart();
      delay(5000);
    }
  } else {
      if(!wfm.startConfigPortal()) {
        Serial.println(F("Portal Error"));
        error_display("Portal Error");
        ESP.restart();
        delay(5000);
    }

  }

  strncpy(mqttServer, mqtt_server.getValue(), MQTT_SERVER_LENGTH);
  strncpy(mqttTopic, mqtt_topic.getValue(), MQTT_TOPIC_LENGTH);
  mqttPort = atoi(mqtt_port.getValue());

  if(configMode) 
    writeEEPROM();
  
  clear_display();
}

void otaSetup() {

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
 
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
    //Slow down the flashing of "FLASH" by a factor of 3, so it looks better
    if(++flash_toggle == 6)
      flash_toggle=0;

    Serial.printf("%u %u \n",flash_toggle, flash_toggle / 3);
    if(flash_toggle / 3) {
      flash_display();
    } else {
      clear_display();
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("Auth Failed"));
      error_display("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("Begin Failed"));
      error_display("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("Connect Failed"));
      error_display("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("Receive Failed"));
      error_display("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("End Failed"));
      error_display("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(128);
  Wire.begin(21,22);

  initDisplay();

  readEEPROM();

  printMQTT();

  callWFM(true);

  otaSetup();

  initMQTT();
  initThermistor();
  
  pinMode(CONFIG_BUTTON,INPUT);
  attachInterrupt(CONFIG_BUTTON, longPress, CHANGE);

  readLoop.start();

  Serial.print(F("IP Addr "));
  Serial.println(WiFi.localIP()); 

}

void loop() {
  if(buttonLongPress) {
    callWFM(false);
    buttonLongPress = false;
  }
  

  readLoop.update();
  ArduinoOTA.handle();
}

void readTemperature() {
  uint8_t temp=getTemperature();

  // Keep error message on screen if there is a publish error
  if(publishTemp(temp))
    temperature_display(temp);
  
}

ICACHE_RAM_ATTR void longPress() {
  uint8_t curState = digitalRead(CONFIG_BUTTON);
  
  if (curState) {
  // Button Released
    if((millis() - lastPressTime) > 1000)
    {
      buttonLongPress = true;
    }

  } else {
  // Button Pressed
    lastPressTime = millis();
    buttonLongPress = false;
  }


}