/*
 * GeneralFunctions.h
 *
 *  Created on: 13 Feb. 2019
 *      Author: arifainchtein
 */

#ifndef GENERALFUNCTIONS_H_
#define GENERALFUNCTIONS_H_
#include "Arduino.h"

class GeneralFunctions{
public:
	GeneralFunctions();
	String getValue(String data, char separator, int index);
	byte getStateOfCharge(double batteryVoltage);
	float stringToFloat(String s);
	char * generatePassword();
	int freeRam () ;
};
#endif /* GENERALFUNCTIONS_H_ */

