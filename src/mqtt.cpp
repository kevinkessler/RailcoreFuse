/**
 *  @filename   :   mqtt.cpp
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

#include <WiFi.h> 
#include "PubSubClient.h"
#include "railcorefuse.h"


WiFiClient espClient;
PubSubClient mqttClient(espClient);
char subName[25];

extern char mqttServer[MQTT_SERVER_LENGTH];
extern char mqttTopic[MQTT_TOPIC_LENGTH];
extern uint16_t mqttPort;

const char *configJson = "{\"device_class\":\"temperature\",\"name\": \"RailcoreFuse\",\"state_topic\": \"%s/%s\", \"unit_of_measurement\": \"°C\", \"value_template\": \"{{ value_json.temperature}}\" }";

void mqttCallback(char *topic, byte *payload, uint16_t length) {
    Serial.printf("Message Received on topic %s\n", topic);
}

static void reconnect() {
    if(mqttClient.connect(subName)) {
        Serial.println("MQTT Connected");
        publishConfig();
    } else {
        char errorMes[50];
        sprintf(errorMes, "MQTT Connection failed, rc=%d",mqttClient.state());
        error_display(errorMes);
        Serial.println(errorMes);
    }

}

static boolean publishMes(char *topic, char *payload) {
    if(!mqttClient.connected())
        reconnect();
        if(!mqttClient.connected())
            return false;

    if(!mqttClient.publish(topic, payload)) {
        char errorMes[50];
        sprintf(errorMes, "MQTT Publish failed, rc=%d",mqttClient.state());
        Serial.println(errorMes);
        error_display(errorMes);
        return false;
    }

    return true;
}

boolean publishConfig () {
    uint8_t configLen = strlen(configJson);
    char payload[configLen + MQTT_TOPIC_LENGTH + 10];
    sprintf(payload,configJson,mqttTopic,"state");

    char topic[MQTT_TOPIC_LENGTH+10];
    sprintf(topic,"%s/%s",mqttTopic,"config");

    return publishMes(topic,payload);
}

boolean publishTemp(uint8_t temperature) {
    char payload[5];
    itoa(temperature, payload, 10);

    char topic[MQTT_TOPIC_LENGTH + 10];
    sprintf(topic,"%s/%s",mqttTopic,"state");
    return publishMes(topic,payload);
}


void initMQTT() {
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallback);
    
    sprintf(subName, "railcorefuse-%s", &(WiFi.macAddress().c_str())[9]);
}
