/**
 *  @filename   :   display.cpp
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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "railcorefuse.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

static void setHeader()
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(18,4);
    display.println(F("Railcore II Fuse"));
}

void error_display(const char mes[]) {
    display.clearDisplay();
    setHeader();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5,25); 
    display.print(mes);
    display.display();
}

void clear_display() {
    display.clearDisplay();
    setHeader();
    display.display();
}

void flash_display() {
    display.clearDisplay();
    setHeader();
    display.setCursor(7,25);
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.print(F("FLASH"));
    display.display();
}

void config_display() {
    display.clearDisplay();
    setHeader();
    display.setCursor(18,25);
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.print(F("CONF"));
    display.display();
}

void temperature_display(uint8_t temp) {
    if ((temp < 10) and (temp > 199))
        return;

    char buffer[4];
    itoa(temp, buffer, 10);

    display.clearDisplay();
    setHeader();
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    if(temp < 100) {
        display.setCursor(42,25);
    } else {
        display.setCursor(18,25);
    }
    display.print(buffer);
    display.print((char)247); 
    display.display();
}

void initDisplay() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 Failed");
    }

    clear_display();

}

