/*
 * OLED.cpp
 *
 *  Created on: 2 May 2019
 *      Author: arifainchtein
 */
#include "Arduino.h"

#include <OLED.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c anSSD1306AsciiAvrI2c;


OLED::OLED() {
	// TODO Auto-generated constructor stub

}

void OLED::begin(){
#if RST_PIN >= 0
  anSSD1306AsciiAvrI2c.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  anSSD1306AsciiAvrI2c.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  // Call anSSD1306AsciiAvrI2c.setI2cClock(frequency) to change from the default frequency.

  anSSD1306AsciiAvrI2c.setFont(Arial14);

  uint32_t m = micros();
  anSSD1306AsciiAvrI2c.clear();
  anSSD1306AsciiAvrI2c.println("Hello world!");
}

void OLED::clear(){
	 anSSD1306AsciiAvrI2c.clear();
}
void OLED::setRGB(int r, int g, int b){


}
void OLED::setCursor(uint8_t c, uint8_t r){

}
void OLED::print(const String s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::print(float s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::println(const String s){
	anSSD1306AsciiAvrI2c.println(s);
}

void OLED::println(float s){
	anSSD1306AsciiAvrI2c.println(s);
}


void OLED::print(long s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::println(long s){
	anSSD1306AsciiAvrI2c.println(s);
}

void OLED::print(double s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::println(double s){
	anSSD1306AsciiAvrI2c.println(s);
}

void OLED::print(int s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::println(int s){
	anSSD1306AsciiAvrI2c.println(s);
}

void OLED::print(byte s){
	anSSD1306AsciiAvrI2c.print(s);
}

void OLED::println(byte s){
	anSSD1306AsciiAvrI2c.println(s);
}

void OLED::noDisplay(){

}

void OLED::display(){

}


OLED::~OLED() {
	// TODO Auto-generated destructor stub
}

