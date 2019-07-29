/*
 * TimeManager.cpp
 *
 *  Created on: 13 Feb. 2019
 *      Author: arifainchtein
 */
#include "Arduino.h"
#include <TimeManager.h>

#include <RTCInfoRecord.h>

#include <GravityRtc.h>

#include <GeneralFunctions.h>


#define LEAP_YEAR(_year) ((_year%4)==0)

int timeZoneHours=11;
int SECONDOFFSET=10;
static  byte monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
GravityRtc rtc;

TimeManager::TimeManager(GeneralFunctions& g, HardwareSerial& serial):generalFunctions(g),  _HardSerial(serial){

}

//
// Functions that represents commands received via the serial port
//



//SetTime#23#05#2019#4#05#48#20


boolean TimeManager::setTime(String command){
	int date = generalFunctions.getValue(command, '#', 1).toInt();
	int month = generalFunctions.getValue(command, '#', 2).toInt();
	int year = generalFunctions.getValue(command, '#', 3).toInt();
	int dw = generalFunctions.getValue(command, '#', 4).toInt();
	int hour = generalFunctions.getValue(command, '#', 5).toInt();
	int min = generalFunctions.getValue(command, '#', 6).toInt();
	int sec = generalFunctions.getValue(command, '#', 7).toInt();

	_HardSerial.print(rtc.day);
		_HardSerial.print("/");
		_HardSerial.print(rtc.month);
		_HardSerial.print("/");
		_HardSerial.print(rtc.year);
		_HardSerial.print(" ");
		_HardSerial.print(rtc.hour);
		_HardSerial.print(":");
		_HardSerial.print(rtc.minute);
		_HardSerial.print(":");
		_HardSerial.print(rtc.second);


	rtc.adjustRtc(year,month,date,dw,hour,min,sec);




	//	setTime(hour,min,sec,date,month,year);
	//	RTC.set(now());
	//getTime();
	return true;

}

boolean TimeManager::printTimeToSerial(){

	rtc.read();
	String displayTime =  "";
	_HardSerial.print(rtc.day);
	_HardSerial.print("/");
	_HardSerial.print(rtc.month);
	_HardSerial.print("/");
	_HardSerial.print(rtc.year);
	_HardSerial.print(" ");
	_HardSerial.print(rtc.hour);
	_HardSerial.print(":");
	_HardSerial.print(rtc.minute);
	_HardSerial.print(":");
	_HardSerial.print(rtc.second);


	return true;
}
//
// End of Functions that represents commands received via the serial port
//


void TimeManager::start(){
	rtc.setup();

}


long TimeManager::dateAsSeconds(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second){


	// note year argument is full four digit year (or digits since 2000), i.e.1975, (year 8 is 2008

	int i;


	if(year < 69){
		year+= 2000;
	}

	// seconds from 1970 till 1 jan 00:00:00 this year
	long seconds= (year-1970)*(60*60*24L*365);

	// add extra days for leap years
	for (i=1970; i<year; i++) {
		if (LEAP_YEAR(i)) {
			seconds+= 60*60*24L;
		}
	}
	// add days for this year
	for (i=0; i<month; i++) {
		if (i==1 && LEAP_YEAR(year)) {
			seconds+= 60*60*24L*29;
		} else {
			seconds+= 60*60*24L*monthDays[i];
		}
	}

	seconds+= (date-1)*3600*24L;
	seconds+= hour*3600L;
	seconds+= minute*60L;
	seconds -=  timeZoneHours*3600L;
	seconds+=second;
	return seconds;
}

// String TimeManager::getCurrentTimeForDisplay(){
// 	rtc.read();
// 	String displayTime =  "";

// 	displayTime.concat(rtc.hour);
// 	displayTime.concat(":");
// 	displayTime.concat(rtc.minute);
// 	displayTime.concat(":");
// 	displayTime.concat(rtc.second);
// 	return displayTime;
// }

// String TimeManager::getCurrentDateTimeForDisplay(){
// 	rtc.read();
// 	String displayTime =  "";
// 	displayTime.concat(rtc.day);
// 	displayTime.concat("/");
// 	displayTime.concat(rtc.month);
// 	displayTime.concat("/");
// 	int year = rtc.year-2000;
// 		displayTime.concat(year);
// 	displayTime.concat(" ");
// 	displayTime.concat(rtc.hour);
// 	displayTime.concat(":");
// 	displayTime.concat(rtc.minute);
// 	displayTime.concat(":");
// 	displayTime.concat(rtc.second);

// 	return displayTime;
// }

// String TimeManager::getCurrentDateForDisplay(){
// 	rtc.read();
// 	String displayTime =  "";
// 	displayTime.concat(rtc.day);
// 	displayTime.concat("/");
// 	displayTime.concat(rtc.month);
// 	displayTime.concat("/");
// 	int year = rtc.year-2000;
// 	displayTime.concat(year);

// 	return displayTime;
// }


long TimeManager::getTimeForCodeGeneration(){

	RTCInfoRecord anRTCInfoRecord =getCurrentDateTime();
	uint8_t seconds = anRTCInfoRecord.second+SECONDOFFSET;
	uint8_t month = anRTCInfoRecord.month-1;
	uint16_t year = anRTCInfoRecord.year;


	//uint8_t date = rtc.day;
	//uint8_t hour = rtc.hour;
	//uint8_t minute=rtc.minute;
//	_HardSerial.print("getTimeForCodeGeneration,about to call dateAsSeconds, year=");
//	_HardSerial.println(year);
//	_HardSerial.flush();
	return dateAsSeconds(year, month, anRTCInfoRecord.date, anRTCInfoRecord.hour, anRTCInfoRecord.minute, anRTCInfoRecord.second );
}



RTCInfoRecord TimeManager::getCurrentDateTime(){
	RTCInfoRecord aRTCInfoRecord;
	rtc.read();
	aRTCInfoRecord.date=rtc.day;
	aRTCInfoRecord.month=rtc.month;
	aRTCInfoRecord.year=rtc.year;
	aRTCInfoRecord.hour=rtc.hour;
	aRTCInfoRecord.minute=rtc.minute;
	aRTCInfoRecord.second=rtc.second;

	return aRTCInfoRecord;
}

long TimeManager::getCurrentTimeInSeconds(){
	rtc.read();
	int month = rtc.month-1;
	long now=dateAsSeconds(rtc.year, month, rtc.day, rtc.hour, rtc.minute, rtc.second);
	return now;
}

String TimeManager::getElapsedTimeHoursMinutesSecondsString(long elapsedTime) {
	//String seconds = String(elapsedTime % 60);
	long seconds = elapsedTime/1000;
	int minutes = (seconds % 3600) / 60;
	String minP ="";
	if(minutes<10)minP="0";


	int hours = seconds / 3600;
	String hoursS = "";
	if(hours<10)hoursS="0";


	String time =  hoursS + hours + ":" + minP + minutes;// + ":" + seconds;
	return time;
}
