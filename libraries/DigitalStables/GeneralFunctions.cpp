/*
 * GeneralFunctions.cpp
 *
 *  Created on: 13 Feb. 2019
 *      Author: arifainchtein
 */

#include "Arduino.h"
#include "GeneralFunctions.h"

#define PASSLENGTH 8



GeneralFunctions::GeneralFunctions(){

}



int GeneralFunctions::freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

char* GeneralFunctions::generatePassword(){
	static char pass[PASSLENGTH+1];
	const char*  alphabeth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'?^&/%$£-+";
	const long alphabethLength = sizeof(alphabeth);

	for (int n = 0; n < PASSLENGTH; n++){
		pass[n] = alphabeth[random(0, alphabethLength)];
	}
	pass[PASSLENGTH] = '\0';
	return pass;
}


float GeneralFunctions::stringToFloat(String s){
	char buffer3[10];
	s.toCharArray(buffer3, 10);
	float result = atof(buffer3);
	return result;
}

String GeneralFunctions::getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {
			0, -1  };
	int maxIndex = data.length()-1;
	for(int i=0; i<=maxIndex && found<=index; i++){
		if(data.charAt(i)==separator || i==maxIndex){
			found++;
			strIndex[0] = strIndex[1]+1;
			strIndex[1] = (i == maxIndex) ? i+1 : i;
		}
	}
	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

byte GeneralFunctions::getStateOfCharge(double batteryVoltage){
	if(batteryVoltage>12.90)return 100;
	else if(batteryVoltage>=12.84)return 90;
	else if(batteryVoltage>=12.81)return 85;
	else if(batteryVoltage>=12.78)return 80;
	else if(batteryVoltage>=12.74)return 75;

	else if(batteryVoltage>=12.70)return 70;
	else if(batteryVoltage>=12.66)return 65;

	else if(batteryVoltage>=12.61)return 60;
	else if(batteryVoltage>=12.57)return 55;

	else if(batteryVoltage>=12.51)return 50;
	else if(batteryVoltage>=12.45)return 45;

	else if(batteryVoltage>=12.39)return 40;
	else if(batteryVoltage>=12.33)return 35;

	else if(batteryVoltage>=12.26)return 30;
	else if(batteryVoltage>=12.18)return 25;
	else if(batteryVoltage>=12.10)return 20;
	else if(batteryVoltage>=12.06)return 15;

	else if(batteryVoltage>=12.00)return 10;
	else if(batteryVoltage>=11.95)return 5;

	else if(batteryVoltage>=11.90)return 0;
	return 0;
}


