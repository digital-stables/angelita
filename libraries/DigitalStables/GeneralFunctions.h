/*
 * GeneralFunctions.h
 *
 *  Created on: 13 Feb. 2019
 *      Author: arifainchtein
 */

#ifndef GENERALFUNCTIONS_H_
#define GENERALFUNCTIONS_H_
#include "Arduino.h"
#include <static_str.h>

class GeneralFunctions{
public:
	static String getValue(String data, char separator, int index);
	static byte getStateOfCharge(double batteryVoltage);
	static const char * generatePassword();
	static int freeRam ();
};
#endif /* GENERALFUNCTIONS_H_ */

