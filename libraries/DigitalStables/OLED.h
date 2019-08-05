/*
 * OLED.h
 *
 *  Created on: 2 May 2019
 *      Author: arifainchtein
 */
#include "Arduino.h"

#ifndef LIBRARIES_DIGITALSTABLES_OLED_H_
#define LIBRARIES_DIGITALSTABLES_OLED_H_

#include <LCDDisplay.h>

class OLED: public LCDDisplay {
public:
	OLED();
	virtual ~OLED();
	void begin();
	void display();
	void clear();
	void setRGB(int, int, int);
	void print(const char*);
	void println(const char*);
	void print(const String&);
	void println(const String&);
	void print(float);
	void println(float);
	void noDisplay();
	void setCursor(uint8_t, uint8_t);
	void print(long);
	void println(long);
	void print(double);
	void println(double);
	void print(int);
	void println(int);
	void print(byte);
	void println(byte);

};

#endif /* LIBRARIES_DIGITALSTABLES_OLED_H_ */
