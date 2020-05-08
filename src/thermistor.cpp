/**
 *  @filename   :   thermistor.cpp
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
#include "railcorefuse.h"

// Hacks to try to get ADC to gice somewhat accurate results. Created 2 best fit lines, one up to 2700 counts and one above 2700 counts, 
// and use those to correct the non-linearity in the ESP32 ADC results
#define LUT_LEN 29
const int16_t ADC_ADJ[LUT_LEN] = {148, 147, 149, 156, 173, 168, 163, 173, 175, 185, 182, 187, 182, 182, 189, 190, 196, 202, 207, 207, 208, 207, 195, 193, 168, 148, 134, 118, 81};
const int16_t ADC_CNT[LUT_LEN] = {155, 225, 301, 385, 472, 594, 729, 853, 995, 1140, 1296, 1467, 1668, 1800, 1956, 2114, 2257, 2382, 2511, 2637, 2749, 2863, 2980,
3063, 3170, 3265, 3346, 3415, 3508};


// Keenovo Heater Thermister stats
#define KELVIN 273.15
#define MAX_ADC 4095.0

const float beta=4115;
const float roomTemp=KELVIN+25.0;
const float rRoomTemp=100000;
const float rDivider=10000;

uint32_t getTrueADC(uint8_t highPoint, uint8_t lowPoint, uint32_t adc) {

    // Find ADC correction value along a straight line between the adjacent LUT values
    // Slope = (float)(ADC_ADJ[highPoint]-ADC_ADJ[lowPoint])/(float)(ADC_CNT[highPoint]-ADC_CNT[lowPoint])
    // Distance along the straight line approximation = ((float)adc - (float)ADC_CNT[highPoint])
    // Offset to which to add the ADC adjustment = ADC_ADJ[highPoint]
    uint32_t correctADC = adc + (float)(ADC_ADJ[highPoint]-ADC_ADJ[lowPoint])/(float)(ADC_CNT[highPoint]-ADC_CNT[lowPoint])*((float)adc - (float)ADC_CNT[highPoint])+
                            ADC_ADJ[highPoint];
    Serial.printf("Correct ADC:%d\n",correctADC);
    return correctADC;
}
void initThermistor() {
    adcAttachPin(ADC_PIN);
    analogSetAttenuation(ADC_11db);
    //analogSetCycles(50);
    //analogSetSamples(5);
}

uint8_t getTemperature() {
    uint32_t totalADC=0;
    for(int n=0; n<16; n++) {
        totalADC += analogRead(ADC_PIN);
    }

    uint32_t outADC=totalADC/16;
    Serial.printf("ADC Value:%d\n",outADC);

    float correctedADC=0;
    if(outADC <= ADC_CNT[0])
    {
        correctedADC=getTrueADC(1,0,outADC);

    } else if(outADC >= ADC_CNT[LUT_LEN-1])
    {
        correctedADC=getTrueADC(LUT_LEN-1,LUT_LEN-2,outADC);

    } else {
        for (int n=1; n< LUT_LEN; n++) {
            if (outADC < ADC_CNT[n])
            {
                correctedADC=getTrueADC(n, n-1, outADC);
                break;
            }
        }
    }

    float rThermistor = rDivider * (MAX_ADC/correctedADC - 1);
    Serial.printf("rThermistor:%f\n",rThermistor);
    float tKelvin = (beta * roomTemp) / (beta +(roomTemp * log(rThermistor/rRoomTemp)));
    uint8_t temp = (tKelvin +0.5) - KELVIN;
    Serial.printf("Temperature:%d\n",temp);
    return temp;
}