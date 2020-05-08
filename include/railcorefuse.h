/**
 *  @filename   :   railcorefuse.h
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

#ifndef INCLUDE_RAILCOREFUSE_H_
#define INCLUDE_RAILCOREFUSE_H_

#define MQTT_SERVER_LENGTH 30
#define MQTT_TOPIC_LENGTH 40

#define CONFIG_BUTTON 0
#define ADC_PIN 35

void callWFM(bool);
ICACHE_RAM_ATTR void longPress(void);
void initDisplay(void);
void clear_display(void);
void config_display(void);
void flash_display(void);
void temperature_display(uint8_t);
void error_display(const char* );
void initMQTT(void);
boolean publishTemp(uint8_t);
void readTemperature(void);
void initThermistor(void);
uint8_t getTemperature(void);
boolean publishConfig(void);
#endif /* INCLUDE_RAILCOREFUSE_H_ */