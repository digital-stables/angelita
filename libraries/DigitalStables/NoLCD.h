/*
 * NoLCD.h
 *
 *  Created on: 14 Mar 2019
 *      Author: arifainchtein
 */

#ifndef LIBRARIES_DIGITALSTABLES_NOLCD_H_
#define LIBRARIES_DIGITALSTABLES_NOLCD_H_

#include <Arduino.h>
#include <LCDDisplay.h>
class NoLCD: public LCDDisplay {
public:
	NoLCD(){}
	virtual ~NoLCD(){}
	virtual void begin(){}
	virtual void display(){}
	virtual void clear(){}
	virtual void setRGB(int, int, int){}
	virtual void setCursor(uint8_t, uint8_t){}
	virtual void print(const char*){}
	virtual void print(const String&){}
	virtual void println(const String&){}
	virtual void print(float){}
	virtual void println(const char*){}
	virtual void println(float){}
	virtual void noDisplay(){}
	virtual void print(long){}
	virtual void println(long){}
	virtual void print(double){}
	virtual void println(double){}
	virtual void print(int){}
	virtual void println(int){}
	virtual void print(byte){}
	virtual void println(byte){}
};

#endif /* LIBRARIES_DIGITALSTABLES_NOLCD_H_ */
