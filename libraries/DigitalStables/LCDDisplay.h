/*
 * LCDDisplay.h
 *
 *  Created on: 14 Mar. 2019
 *      Author: arifainchtein
 */
#include "Arduino.h"

#ifndef ARDUINO_LIBRARIES_DIGITALSTABLES_LCDDISPLAY_H_
#define ARDUINO_LIBRARIES_DIGITALSTABLES_LCDDISPLAY_H_

class LCDDisplay {
public:
	LCDDisplay();
	virtual ~LCDDisplay();
	virtual void begin()=0;
	virtual void clear()=0;
	virtual void setRGB(int, int, int)=0;
	virtual void setCursor(uint8_t, uint8_t)=0;
	virtual void print( String)=0;
	virtual void println( String)=0;
	virtual void print(float)=0;
	virtual void println(float)=0;
	virtual void print(long)=0;
	virtual void println(long)=0;
	virtual void print(double)=0;
	virtual void println(double)=0;
	virtual void print(int)=0;
	virtual void println(int)=0;
	virtual void print(byte)=0;
	virtual void println(byte)=0;
	
	virtual void noDisplay()=0;
	virtual void display()=0;
};

#endif /* ARDUINO_LIBRARIES_DIGITALSTABLES_LCDDISPLAY_H_ */
