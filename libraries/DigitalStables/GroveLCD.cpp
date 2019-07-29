/*
 * GroveLCD.cpp
 *
 *  Created on: 14 Mar. 2019
 *      Author: arifainchtein
 */
#include "Arduino.h"
#include <GroveLCD.h>
#include <rgb_lcd.h>

rgb_lcd lcd;

GroveLCD::GroveLCD() {
	// TODO Auto-generated constructor stub

}

void GroveLCD::begin(){
	lcd.begin(16,2);

}

void GroveLCD::clear(){
	lcd.clear();
}
void GroveLCD::setRGB(int r, int g, int b){
	lcd.setRGB(r,g,b);

}
void GroveLCD::setCursor(uint8_t c, uint8_t r){
	lcd.setCursor(c, r);
}
void GroveLCD::print( String  s){
	lcd.print(s);

}

void GroveLCD::println( String  s){
	lcd.print(s);

}

void GroveLCD::println(float s){
	lcd.print(s);
}

void GroveLCD::print(float s){
	lcd.print(s);
}

void GroveLCD::println(long s){
	lcd.print(s);
}

void GroveLCD::print(long s){
	lcd.print(s);
}
void GroveLCD::println(double s){
	lcd.print(s);
}

void GroveLCD::print(double s){
	lcd.print(s);
}
void GroveLCD::noDisplay(){
	lcd.noDisplay();
}

void GroveLCD::display(){
	lcd.display();
}

void GroveLCD::print(int s){
	lcd.print(s);
}

void GroveLCD::println(int s){
	lcd.println(s);
}

void GroveLCD::print(byte s){
	lcd.print(s);
}

void GroveLCD::println(byte s){
	lcd.println(s);
}

GroveLCD::~GroveLCD() {
	// TODO Auto-generated destructor stub
}

