#include "Arduino.h"
#include <LCDDisplay.h>
#include <PowerManager.h>
#include <DataStorageManager.h>
#include <GeneralFunctions.h>
#include <TimeManager.h>
#include <MemoryFree.h>

#include <SecretManager.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <totp.h>

#include <SPI.h>
#include <SD.h>
//
// the wps variables
#define LOCK_CAPACITOR_PIN A5
#define BATTERY_VOLTAGE_PIN A1

char *faultData;
long secondsToForcedWPS=60L;
long wpsAlertTime=0L;

float capacitorVoltage= 0;

//char remFileName[10]="";
//char sensorDirName[10]="";
//char lifeCycleFileName[10]="";

long shutDownRequestedseconds= 0L;
boolean shuttingDownPiCountdown=false;
boolean manualShutdown=false;
boolean waitingManualPiStart=false;

// the battery voltage always in A1
// for compatibility with Gloria, Wally and Valentino
//
    //effective current
boolean powerSupplyOn=false;

//
// current view index
// Controls what the user sees in the lcdnow
// it starts with a value of 99 which means is locked

boolean showingAct=false;
byte currentHour=0;
byte currentDay=0;
byte currentMonth=0;
byte currentYear=0;
//
// the virtual micrcntroller
String currentIpAddress="No IP";
String currentSSID="No SSID";
byte delayTime=1;

byte SHARED_SECRET_LENGTH;

long previousUpdate;

PowerManager::PowerManager(LCDDisplay& l, SecretManager& s, DataStorageManager& sd, TimeManager& t,HardwareSerial& serial): lcd(l),secretManager(s), dataStorageManager(sd),timeManager(t), _HardSerial(serial)
{}

void PowerManager::start(){
	// pinMode(52, OUTPUT);
	// digitalWrite(52, LOW);
	SPI.begin();
	pinMode(PI_POWER_PIN, OUTPUT);
	pinMode(IPHONE_POWER_PIN, OUTPUT);
	
	long now = timeManager.getCurrentTimeInSeconds();
	turnPiOff(now);
	initializeWDT();
}
void PowerManager::hourlyTasks(long time, int previousHour ){

	dataStorageManager.storeRememberedValue(time,GET_PSTR(HOURLY_ENERGY), hourlyBatteryOutEnergy, GET_PSTR(UNIT_MILLI_AMPERES_HOURS));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(HOURLY_POWERED_DOWN_IN_LOOP_SECONDS), hourlyPoweredDownInLoopSeconds, GET_PSTR(UNIT_SECONDS));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(HOURLY_OPERATING_IN_LOOP_SECONDS), 3600-hourlyPoweredDownInLoopSeconds, GET_PSTR(UNIT_SECONDS));
	hourlyBatteryOutEnergy=0;
	hourlyPoweredDownInLoopSeconds=0;
}

/*
 * this function is called at the beginning of a new day
 */
void PowerManager::dailyTasks(long time, int yesterdayDate, int yesterdayMonth, uint16_t yesterdayYear ){
	//
	// move whatever is in untrasferred to the correct date
	boolean result = dataStorageManager.readUntransferredFileFromSDCardByDate( 1,false, RememberedValueDataDirName,yesterdayDate, yesterdayMonth, yesterdayYear );
	result = dataStorageManager.readUntransferredFileFromSDCardByDate( 1,false, WPSSensorDataDirName,yesterdayDate, yesterdayMonth, yesterdayYear);
	result = dataStorageManager.readUntransferredFileFromSDCardByDate( 1,false, LifeCycleDataDirName,yesterdayDate, yesterdayMonth, yesterdayYear);
	long yesterdayDateSeconds = timeManager.dateAsSeconds(yesterdayYear,yesterdayMonth,yesterdayDate, 0, 0, 0);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_STATS_TIMESTAMP), yesterdayDateSeconds, GET_RSTR(UNIT_NO_UNIT));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_MINIMUM_BATTERY_VOLTAGE), dailyMinBatteryVoltage, GET_RSTR(UNIT_VOLT));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_MAXIMUM_BATTERY_VOLTAGE), dailyMaxBatteryVoltage, GET_RSTR(UNIT_VOLT));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_MINIMUM_BATTERY_CURRENT), dailyMinBatteryCurrent, GET_PSTR(UNIT_MILLI_AMPERES));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_MAXIMUM_BATTERY_CURRENT), dailyMaxBatteryCurrent, GET_PSTR(UNIT_MILLI_AMPERES));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_ENERGY), dailyBatteryOutEnergy, GET_PSTR(UNIT_MILLI_AMPERES_HOURS));
	dataStorageManager.storeRememberedValue(time,GET_PSTR(DAILY_POWERED_DOWN_IN_LOOP_SECONDS), dailyPoweredDownInLoopSeconds, GET_PSTR(UNIT_SECONDS));
	dailyMinBatteryVoltage = 9999.0;
	dailyMaxBatteryVoltage = -1.0;
	dailyMinBatteryCurrent = 9999.0;
	dailyMaxBatteryCurrent = -1.0;
	dailyBatteryOutEnergy=0.0;
	dailyPoweredDownInLoopSeconds=0.0;

}

void PowerManager::monthlyTasks(long time){

}

void PowerManager::yearlyTasks(long time){

}




float PowerManager::getCurrentFromBattery(void){
	return 0.0;
}

float PowerManager::getCurrentInputFromSolarPanel(void){
	return 0.0;
}

float PowerManager::getSolarPanelVoltage(void){
	return 0.0;
}


//float PowerManager::getCurrentValue(void){
//	int sensorValue;             //value read from the sensor
//	int sensorMax = 0;
//	uint32_t start_time = millis();
//	while((millis()-start_time) < 100)//sample for 1000ms
//	{
//		sensorValue = analogRead(CURRENT_SENSOR);
//		if (sensorValue > sensorMax)
//		{
//			//record the maximum sensor value
//			sensorMax = sensorValue;
//		}
//	}
//
//	//the VCC on the Grove interface of the sensor is 5v
//	amplitude_current=(float)(sensorMax-512)/1024*5/185*1000000;
//	effective_value=amplitude_current/1.414;
//	return effective_value;
//}

float PowerManager::getBatteryVoltage(){
	int NUM_SAMPLES=10;
  int sample_count=0;
  float sum=0;
  while (sample_count < NUM_SAMPLES) {
        sum += analogRead(BATTERY_VOLTAGE_PIN);
        sample_count++;
        delay(10);
    }
   // sum=sum/10;
    // calculate the voltage
    // use 5.0 for a 5.0V ADC reference voltage
    // 5.015V is the calibrated reference voltage
    float voltage = 11.0*((float)sum / (float)NUM_SAMPLES * 5.16) / 1024.0;
    return voltage;
}


void PowerManager::initializeWDT(){
	/*** Setup the WDT ***/

	/* Clear the reset flag. */
	MCUSR &= ~(1<<WDRF);

	/* In order to change WDE or the prescaler, we need to
	 * set WDCE (This will allow updates for 4 clock cycles).
	 */
	WDTCSR |= (1<<WDCE) | (1<<WDE);

	/* set new watchdog timeout prescaler value */
	WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */

	/* Enable the WD interrupt (note no reset). */
	WDTCSR |= _BV(WDIE);

}




/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void PowerManager::enterArduinoSleep(void)
{
	digitalWrite(PI_POWER_PIN, LOW);



	wdt_reset();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
	//sleep_enable();
	long currentSleepSeconds = timeManager.getCurrentTimeInSeconds();
	/* Now enter sleep mode. */
	sleep_mode();

	/* The program will continue from here after the WDT timeout*/

	//
	// check the voltage of the battery, if its higher than
	// the min for wps then go into wps,
	// otherwise go back to comma
	//
	long lastSleepSeconds = timeManager.getCurrentTimeInSeconds()-currentSleepSeconds ;
	poweredDownInLoopSeconds+=lastSleepSeconds;
	float batteryVoltage = getBatteryVoltage();
	if(batteryVoltage>minWPSVoltage){
		// STORE a lifecycle comma exit record
		long now = timeManager.getCurrentTimeInSeconds();
		dataStorageManager.storeLifeCycleEvent(now, LIFE_CYCLE_EVENT_END_COMMA,LIFE_CYCLE_EVENT_COMMA_VALUE);
		lcd.display();
		lcd.setRGB(255,255,0);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(F("Out of Comma"));
		lcd.setCursor(0,1);
		lcd.print(batteryVoltage);
		lcd.print(F("V "));
		lcd.print(lastSleepSeconds);
		lcd.print(F("V "));

		operatingStatus="WPS";
		currentSleepStartTime = now;
		wpsSleeping=true;
		inWPS=true;
		sleep_disable(); /* First thing to do is disable sleep. */
		/* Re-enable the peripherals. */
		power_all_enable();
	}else{
		lcd.display();
		lcd.setRGB(255,0,0);
		lcd.clear();
		lcd.print(batteryVoltage);
		lcd.print(F("V"));
		delay(500);
		lcd.noDisplay();
		lcd.setRGB(0,0,0);
		enterArduinoSleep();
	}

}

/***************************************************
 *  Name:        pauseWPS
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: This method is similar to enterSleep except that is called
 *               during the time where the voltage is whithin the wps range
 *               and is a way to save power
 *               it is different than the comma because it does not recursively
 *               call itself and does not write lifecycle events
 *
 ***************************************************/
void PowerManager::pauseWPS(void)
{
	digitalWrite(PI_POWER_PIN, LOW);
	lcd.noDisplay();
	lcd.setRGB(0,0,0);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
	sleep_enable();

	long currentSleepSeconds = timeManager.getCurrentTimeInSeconds();
	/* Now enter sleep mode. */
	sleep_mode();

	/* The program will continue from here after the WDT timeout*/

	//
	// check the voltage of the battery, if its higher than
	// the min for wps then go into wps,
	// otherwise go back to comma
	//
	long lastSleepSeconds = timeManager.getCurrentTimeInSeconds()-currentSleepSeconds ;
	poweredDownInLoopSeconds+=lastSleepSeconds;

	lcd.display();
	lcd.setRGB(255,255,0);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(F("Out of Pause"));
	lcd.setCursor(0,1);
	float batteryVoltage = getBatteryVoltage();
	lcd.print(batteryVoltage);
	lcd.print(F("V "));
	lcd.print(pauseDuringWPS);

	operatingStatus="WPS";
	//lcd.setCursor(0, 1);
	//lcd.print(F("Awake"));
	sleep_disable(); /* First thing to do is disable sleep. */
	/* Re-enable the peripherals. */
	power_all_enable();
}

void PowerManager::sendWPSAlert(long time, char *faultData, int batteryVoltage){
	waitingForWPSConfirmation=true;
	wpsCountdown=false;
	inWPS=true;
	operatingStatus="WPS";
	wpsAlertTime=timeManager.getCurrentTimeInSeconds();
	dataStorageManager.storeRememberedValue(time,faultData, batteryVoltage, UNIT_VOLT);
}

void PowerManager::turnPiOffForced(long time){
	float batteryVoltageBefore = getBatteryVoltage();
	digitalWrite(PI_POWER_PIN, LOW);
	delay(1000);
	float batteryVoltageAfter = getBatteryVoltage();
	float voltageDifferential = 1-(batteryVoltageBefore/batteryVoltageAfter);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(FORCED_PI_TURN_OFF),0 , operatingStatus);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_BEFORE_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_ATER_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_DIFFERENTIAL_AFTER_PI_ON), voltageDifferential, UNIT_PERCENTAGE);
}

void PowerManager::turnPiOff(long time){
	float batteryVoltageBefore = getBatteryVoltage();
	digitalWrite(PI_POWER_PIN, LOW);
	delay(1000);
	float batteryVoltageAfter = getBatteryVoltage();
	float voltageDifferential = 1-(batteryVoltageBefore/batteryVoltageAfter);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(PI_TURN_OFF),0 , operatingStatus);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_BEFORE_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_ATER_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_DIFFERENTIAL_AFTER_PI_ON), voltageDifferential, UNIT_PERCENTAGE);
}


void PowerManager::turnPiOn(long time){
	float batteryVoltageBefore = getBatteryVoltage();
	digitalWrite(PI_POWER_PIN, HIGH);
	delay(1000);
	float batteryVoltageAfter = getBatteryVoltage();
	float voltageDifferential = 1-(batteryVoltageAfter/batteryVoltageBefore);

	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_BEFORE_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_ATER_PI_ON), batteryVoltageBefore, UNIT_VOLT);
	dataStorageManager.storeRememberedValue(time,GET_PSTR(BATTERY_VOLTAGE_DIFFERENTIAL_AFTER_PI_ON), voltageDifferential, UNIT_PERCENTAGE);
}



void PowerManager::defineState(){
	
	float batteryVoltage = getBatteryVoltage();
	byte internalBatteryStateOfCharge = GeneralFunctions::getStateOfCharge(batteryVoltage);
	float currentFromBattery = getCurrentFromBattery();
	
	lcd.clear();
	lcd.setCursor(0, 0);

	lcd.print(GeneralFunctions::freeRam());
	lcd.print(F("b ")) ;

	lcd.print(batteryVoltage) ;
	lcd.print(F("V ")) ;
	lcd.print(internalBatteryStateOfCharge);
	lcd.print("%");
	lcd.setCursor(0, 1);
	RTCInfoRecord r =timeManager.getCurrentDateTime();
	lcd.print(r.hour);
	lcd.print(F(":"));
	if(r.minute<10)lcd.print(F("0"));
	lcd.print(r.minute);
	lcd.print(F(":"));
	if(r.second<10)lcd.print(F("0"));
	lcd.print(r.second);
	
}
// void PowerManager::defineState(){
// 	poweredDownInLoopSeconds=0;
// 	long time = timeManager.getCurrentTimeInSeconds();

// 	float batteryVoltage = getBatteryVoltage();
// 	byte internalBatteryStateOfCharge = GeneralFunctions::getStateOfCharge(batteryVoltage);
// 	float currentFromBattery = getCurrentFromBattery();
// 	float inputFromSOlarPanel =  getCurrentInputFromSolarPanel();
// 	float solarPanelVolltage = getSolarPanelVoltage();


// 	boolean piIsOn = digitalRead(PI_POWER_PIN);

// 	if(shuttingDownPiCountdown){
// 		currentSecondsToPowerOff = secondsToTurnPowerOff -( time - shutDownRequestedseconds );
// 		lcd.clear();
// 		lcd.setCursor(0,0);
// 		String s = "Shutting down";
// 		lcd.print(s);
// 		lcd.setCursor(0,1);

// 		if(currentSecondsToPowerOff<=0){
// 			shuttingDownPiCountdown=false;
// 			manualShutdown=true;
// 			inPulse=false;
// 			turnPiOff(time);
// 			dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_MANUAL_SHUTDOWN, LIFE_CYCLE_EVENT_COMMA_VALUE);
// 			lcd.print(F("Pi is OFF"));
// 			currentViewIndex=3;
// 		}else{
// 			lcd.print(F("in "));
// 			lcd.print(	currentSecondsToPowerOff);
// 		}
// 	}else if(batteryVoltage>exitWPSVoltage){
// 		if(!piIsOn && !manualShutdown)turnPiOn(time);
// 		operatingStatus="Normal";
// 		lcd.setRGB(0, 225, 0);
// 		operatingStatus="Normal";
// 		wpsCountdown=false;
// 		wpsSleeping=false;
// 		inWPS=false;
// 		waitingForWPSConfirmation=false;

// 		if(inPulse){
// 			lcd.clear();
// 			lcd.setCursor(0, 0);
// 			lcd.print(F("Executing Pulse" );
// 			lcd.setCursor(0, 1);
// 			lcd.print( "Started at " );
// 			lcd.print(  pulseStartTime );
// 		}else{
// 			//
// 			// if we are here it means
// 			// that we are not in pulse and not in wps
// 			// so display user data according to the value of
// 			// currentViewIndex
// 			// currentViewIndex = 0 means show main data
// 			// currentViewIndex = 1 means Generate Password
// 			// currentViewIndex = 2 show Network info
// 			// currentViewIndex = 3 means shutdown request
// 			// currentViewIndex = 4 shutdown in process
// 			// i
// 			switch(currentViewIndex){
// 			case 0:
// 				//
// 				// this is the most
// 				// common state so as to
// 				//avoid flickering, only refresh once per second
// 				if((millis()-previousUpdate) >1000){
// 					previousUpdate = millis();
// 					lcd.clear();
// 					lcd.setCursor(0, 0);

// 					lcd.print(freeRam());
// 					lcd.print(F("b "));

// 					lcd.print(batteryVoltage) ;
// 					lcd.print(F("V "));
// 					lcd.print(internalBatteryStateOfCharge);
// 					lcd.print(F("%"));
// 					lcd.setCursor(0, 1);
// 					RTCInfoRecord r =timeManager.getCurrentDateTime();
// 					lcd.print(r.hour);
// 					lcd.print(F(":"));
// 					lcd.print(r.minute);
// 					lcd.print(F(":"));
// 					lcd.print(r.second);
// 				}

// 				break;

// 			case 1:
// 				lcd.clear();
// 				lcd.setCursor(0, 0);
// 				lcd.print(F("Create Password"));
// 				lcd.setCursor(0, 1);
// 				lcd.print(F(" "));
// 				break;
// 			case 2:
// 				lcd.clear();
// 				lcd.setCursor(0, 0);
// 				lcd.print(currentSSID);
// 				lcd.setCursor(0, 1);
// 				lcd.print(currentIpAddress);
// 				break;
// 			case 3:
// 				lcd.clear();
// 				lcd.setCursor(0, 0);
// 				if(manualShutdown){
// 					lcd.print(F("Pi is Off"));
// 					lcd.setCursor(0, 1);
// 					lcd.print(F("Turn On Pi?"));
// 				}else if(waitingManualPiStart){
// 					lcd.print(F("Waiting for Pi" );
// 					lcd.setCursor(0, 1);
// 					lcd.print(F("To Start" );

// 				}else{
// 					lcd.print(F("Turn Off Pi"));
// 					lcd.setCursor(0, 1);
// 					lcd.print(F("Are You Sure?"));
// 				}

// 				break;

// 			case 30:
// 				lcd.clear();
// 				lcd.setCursor(0, 0);
// 				lcd.print(F("Shutting Down Pi" );
// 				lcd.setCursor(0, 1);
// 				lcd.print(F(" " );
// 				break;


// 			case 35:
// 				// the pi was just turned on
// 				// manually
// 				lcd.clear();
// 				lcd.setCursor(0, 0);
// 				lcd.print(F("Waiting for Pi" );
// 				lcd.setCursor(0, 1);
// 				lcd.print(F("To Start" );
// 				currentViewIndex=3;
// 				waitingManualPiStart=true;
// 				break;
// 			}
// 		}
// 	}else if(batteryVoltage>enterWPSVoltage && batteryVoltage<=exitWPSVoltage){
// 		if(wpsSleeping){
// 			//delay(1000);
// 			//lcd.noDisplay();
// 			long piSleepingRemaining = secondsToNextPiOn-(time - currentSleepStartTime) ;
// 			lcd.clear();
// 			lcd.display();
// 			lcd.setCursor(0,0);
// 			lcd.setRGB(255,255,0);

// 			if(piSleepingRemaining<=0){
// 				wpsSleeping=false;
// 				if(!digitalRead(PI_POWER_PIN))turnPiOn(time);
// 				dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_END_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);

// 				lcd.print(F("Pi ON WPS "));
// 				lcd.setCursor(0,1);
// 				lcd.print(batteryVoltage);
// 				lcd.print(F("V "));
// 				lcd.print(internalBatteryStateOfCharge);
// 				lcd.print(F("%"));
// 				lastWPSStartUp = time;
// 			}else{
// 				//
// 				// if we are here is because we are in the
// 				// sleep period of the wps cycle.
// 				// check to see if we need to store a record in the sd card
// 				//
// 				long z =time-lastWPSRecordSeconds;
// 				lcd.print(F("wps rec in "));
// 				long netWPSRecordIn = (long)wpsPulseFrequencySeconds-z;

// 				lcd.print(netWPSRecordIn);
// 				lcd.setCursor(0,1);
// 				lcd.print(F("pi on in "));
// 				long piremaining = secondsToNextPiOn-(time - currentSleepStartTime) ;
// 				lcd.print(piremaining);


// 				//delay(1000);

// 				if(netWPSRecordIn<=0){
// 					lcd.clear();
// 					lcd.display();

// 					lastWPSRecordSeconds = timeManager.getCurrentTimeInSeconds();
// 					WPSSensorRecord anWPSSensorRecord;
// 					anWPSSensorRecord.batteryVoltage= getBatteryVoltage();
// 					anWPSSensorRecord.current = getCurrentFromBattery();
// 					anWPSSensorRecord.stateOfCharge = GeneralFunctions::getStateOfCharge.(batteryVoltage);
// 					anWPSSensorRecord.lastWPSRecordSeconds=lastWPSRecordSeconds;
// 					anWPSSensorRecord.hourlyBatteryOutEnergy=hourlyBatteryOutEnergy;
// 					anWPSSensorRecord.dailyBatteryOutEnergy=dailyBatteryOutEnergy;
// 					anWPSSensorRecord.hourlyPoweredDownInLoopSeconds=hourlyPoweredDownInLoopSeconds;
// 					anWPSSensorRecord.dailyPoweredDownInLoopSeconds=dailyPoweredDownInLoopSeconds;
// 					anWPSSensorRecord.pauseDuringWPS=pauseDuringWPS;
// 					anWPSSensorRecord.operatingStatus=operatingStatus;
// 					anWPSSensorRecord.totalDiskUse= 0; dataStorageManager.getDiskUsage();


// 					dataStorageManager.saveWPSSensorRecord( anWPSSensorRecord);
// 					lcd.setRGB(255,255,0);
// 				}else{
// 					//
// 					// if we are here is because we are in the sleeping part of the
// 					// WPS and is not time to take another record
// 					// now check if there is any reason to keep it from comma
// 					// ie if its raining and the sensor needs to stay on
// 					// if not sleep for 8 seconds
// 					//


// 					if(pauseDuringWPS){
// 						pauseWPS();
// 					}
// 				}
// 			}
// 		}else if(piIsOn){
// 			lcd.clear();
// 			lcd.setCursor(0,0);
// 			lcd.print(F("pi ON WPS "));
// 			lcd.print(batteryVoltage);
// 			lcd.print(F(" V"));
// 			lcd.setCursor(0,1);
// 			lcd.print(F("Runtime "));
// 			long secsRunning = time-lastWPSStartUp;
// 			lcd.print(secsRunning);
// 		}
// 	}else if(batteryVoltage>minWPSVoltage && batteryVoltage<=enterWPSVoltage){
// 		if(!inWPS){
// 			faultData="Enter WPS";
// 			sendWPSAlert(time, faultData, batteryVoltage);
// 			lcd.clear();
// 			lcd.setRGB(225, 225, 0);
// 			lcd.setCursor(0, 0);
// 			lcd.print(F("WPS Alert Sent"));

// 		}else{
// 			if(waitingForWPSConfirmation){
// 				delay(1000);
// 				long z = time-wpsAlertTime;
// 				long remaining = secondsToForcedWPS-z;
// 				lcd.clear();
// 				lcd.setRGB(225,225,0);
// 				lcd.setCursor(0,0);

// 				if( remaining <= 0  ){
// 					waitingForWPSConfirmation=false;
// 					operatingStatus="WPS";
// 					dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_FORCED_START_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);
// 					lcd.print(F("pi off"));
// 					wpsSleeping=true;
// 					currentSleepStartTime = time;
// 					currentSecondsToPowerOff=0L;
// 					turnPiOff(time);
// 					wpsCountdown=false;
// 				}else{
// 					lcd.print(F("Waiting EnterWPS"));
// 					lcd.setCursor(0,1);
// 					long remaining = secondsToForcedWPS-z;
// 					lcd.print(remaining);
// 					lcd.print(F("  "));
// 					lcd.print(batteryVoltage);
// 					lcd.print(F("V "));
// 				}
// 			}else if(wpsCountdown){
// 				currentSecondsToPowerOff = secondsToTurnPowerOff -( time - wpsCountDownStartSeconds);
// 				lcd.clear();
// 				lcd.setCursor(0,0);
// 				lcd.print(F("wps countdown "));
// 				lcd.setCursor(0,1);
// 				lcd.print(	currentSecondsToPowerOff);
// 				if(currentSecondsToPowerOff<=0){
// 					currentSecondsToPowerOff=0;
// 					turnPiOff(time);
// 					dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_START_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);
// 					wpsSleeping=true;
// 					wpsCountdown=false;
// 					currentSleepStartTime=time;
// 				}
// 			}else if(wpsSleeping){
// 				//delay(1000);
// 				//lcd.noDisplay();
// 				long piSleepingRemaining = secondsToNextPiOn-(time - currentSleepStartTime) ;
// 				lcd.clear();
// 				lcd.display();
// 				lcd.setCursor(0,0);
// 				lcd.setRGB(255,255,0);

// 				if(piSleepingRemaining<=0){
// 					wpsSleeping=false;
// 					if(!digitalRead(PI_POWER_PIN))turnPiOn(time);
// 					dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_END_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);

// 					lcd.print(F("Pi ON WPS "));
// 					lcd.setCursor(0,1);
// 					lcd.print(batteryVoltage);
// 					lcd.print(F("V "));
// 					lcd.print(internalBatteryStateOfCharge);
// 					lcd.print(F("%"));
// 					lastWPSStartUp = time;
// 				}else{
// 					//
// 					// if we are here is because we are in the
// 					// sleep period of the wps cycle.
// 					// check to see if we need to store a record in the sd card
// 					//
// 					long z =time-lastWPSRecordSeconds;
// 					lcd.print(F("WPS rec in "));
// 					long netWPSRecordIn = (long)wpsPulseFrequencySeconds-z;

// 					lcd.print(netWPSRecordIn);
// 					lcd.setCursor(0,1);
// 					lcd.print(F("pi on in "));
// 					long piremaining = secondsToNextPiOn-(time - currentSleepStartTime) ;
// 					lcd.print(piremaining);


// 					//delay(1000);

// 					if(netWPSRecordIn<=0){
// 						lcd.clear();
// 						lcd.display();

// 						lastWPSRecordSeconds = timeManager.getCurrentTimeInSeconds();
// 						WPSSensorRecord anWPSSensorRecord;
// 						anWPSSensorRecord.batteryVoltage= getBatteryVoltage();
// 						anWPSSensorRecord.current = getCurrentFromBattery();
// 						anWPSSensorRecord.stateOfCharge = GeneralFunctions::getStateOfCharge(batteryVoltage);
// 						anWPSSensorRecord.lastWPSRecordSeconds=lastWPSRecordSeconds;
// 						anWPSSensorRecord.hourlyBatteryOutEnergy=hourlyBatteryOutEnergy;
// 						anWPSSensorRecord.dailyBatteryOutEnergy=dailyBatteryOutEnergy;
// 						anWPSSensorRecord.hourlyPoweredDownInLoopSeconds=hourlyPoweredDownInLoopSeconds;
// 						anWPSSensorRecord.dailyPoweredDownInLoopSeconds=dailyPoweredDownInLoopSeconds;
// 						anWPSSensorRecord.pauseDuringWPS=pauseDuringWPS;
// 						anWPSSensorRecord.operatingStatus=operatingStatus;
// 						anWPSSensorRecord.totalDiskUse=989; dataStorageManager.getDiskUsage();

// 						dataStorageManager.saveWPSSensorRecord( anWPSSensorRecord);
// 						lcd.setRGB(255,255,0);
// 					}else{
// 						//
// 						// if we are here is because we are in the sleeping part of the
// 						// WPS and is not time to take another record
// 						// now check if there is any reason to keep it from comma
// 						// ie if its raining and the sensor needs to stay on
// 						// if not sleep for 8 seconds
// 						//

// 						if(pauseDuringWPS){
// 							pauseWPS();
// 						}
// 					}
// 				}
// 			}else{
// 				if(piIsOn){
// 					lcd.clear();
// 					lcd.setCursor(0,0);
// 					lcd.print(F("pi ON WPS "));
// 					lcd.print(batteryVoltage);
// 					lcd.print(F(" V"));
// 					lcd.setCursor(0,1);
// 					lcd.print(F("Runtime "));
// 					long secsRunning = time-lastWPSStartUp;
// 					lcd.print(secsRunning);
// 				}else{

// 				}

// 			}
// 		}

// 	}else if(batteryVoltage<minWPSVoltage ){
// 		if(!inWPS ){
// 			faultData="Enter WPS";
// 			sendWPSAlert(time, faultData, batteryVoltage);
// 			lcd.clear();
// 			lcd.setRGB(225, 0, 0);
// 			lcd.setCursor(0, 0);
// 			lcd.print(F("Comma Alert Sent"));

// 		}else{
// 			if(waitingForWPSConfirmation){
// 				delay(1000);
// 				long z = time-wpsAlertTime;
// 				long remaining = secondsToForcedWPS-z;
// 				lcd.clear();
// 				lcd.setCursor(0,0);
// 				lcd.setRGB(255,0,0);
// 				lcd.setCursor(0,0);
// 				if( remaining <= 0  ){
// 					waitingForWPSConfirmation=false;
// 					operatingStatus="WPS";
// 					dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_FORCED_START_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);
// 					wpsSleeping=false;
// 					currentSecondsToPowerOff=0L;
// 					if(piIsOn)turnPiOff(time);
// 					wpsCountdown=false;

// 					if(f_wdt == 1){
// 						/* Don't forget to clear the flag. */
// 						f_wdt = 0;
// 						/* Re-enter sleep mode. */
// 						lcd.print(F("Enter Comma"));
// 						operatingStatus="Comma";
// 						lcd.setCursor(0,1);
// 						lcd.print(batteryVoltage);
// 						lcd.print(F(" V"));
// 						delay(2000);
// 						lcd.setRGB(0,0,0);
// 						lcd.noDisplay();
// 						dataStorageManager.storeLifeCycleEvent(time,LIFE_CYCLE_EVENT_START_COMMA, LIFE_CYCLE_EVENT_COMMA_VALUE);
// 						enterArduinoSleep();
// 					}
// 				}else{
// 					lcd.print(F("Waiting EnterWPS"));
// 					lcd.setCursor(0,1);
// 					long remaining = secondsToForcedWPS-z;
// 					lcd.print(remaining);
// 					lcd.print(F("  "));
// 					lcd.print(batteryVoltage);
// 					lcd.print(F("V "));
// 				}
// 			}else if(wpsCountdown){
// 				currentSecondsToPowerOff = secondsToTurnPowerOff -( time - wpsCountDownStartSeconds);
// 				lcd.clear();
// 				lcd.setCursor(0,0);
// 				lcd.print(F("wps countdown "));
// 				lcd.setCursor(0,1);
// 				lcd.print(	currentSecondsToPowerOff);
// 				if(currentSecondsToPowerOff<=0){
// 					currentSecondsToPowerOff=0;
// 					if(piIsOn)turnPiOff(time);
// 					dataStorageManager.storeLifeCycleEvent(time, LIFE_CYCLE_EVENT_START_WPS, LIFE_CYCLE_EVENT_WPS_VALUE);
// 					wpsSleeping=false;
// 					wpsCountdown=false;
// 					if(f_wdt == 1){
// 						/* Don't forget to clear the flag. */
// 						f_wdt = 0;
// 						/* Re-enter sleep mode. */
// 						lcd.print(F("Enter Comma"));
// 						operatingStatus="Comma";
// 						lcd.setCursor(0,1);
// 						lcd.print(batteryVoltage);
// 						lcd.print(F(" V"));
// 						delay(2000);
// 						lcd.setRGB(0,0,0);
// 						lcd.noDisplay();
// 						dataStorageManager.storeLifeCycleEvent(time,LIFE_CYCLE_EVENT_START_COMMA, LIFE_CYCLE_EVENT_COMMA_VALUE);
// 						enterArduinoSleep();
// 					}
// 				}
// 			}else if(wpsSleeping){
// 				//
// 				// if the pi is asleep then go into a comma
// 				//
// 				if(f_wdt == 1){
// 					/* Don't forget to clear the flag. */
// 					f_wdt = 0;
// 					/* Re-enter sleep mode. */
// 					lcd.clear();
// 					lcd.setRGB(255,0,0);
// 					lcd.setCursor(0,0);
// 					lcd.print(F("Enter Comma"));
// 					operatingStatus="Comma";
// 					lcd.setCursor(0,1);
// 					lcd.print(batteryVoltage);
// 					lcd.print(F(" V"));
// 					delay(2000);
// 					lcd.setRGB(0,0,0);
// 					lcd.noDisplay();
// 					dataStorageManager.storeLifeCycleEvent(time,LIFE_CYCLE_EVENT_START_COMMA, LIFE_CYCLE_EVENT_COMMA_VALUE);
// 					enterArduinoSleep();
// 				}
// 			}else if(piIsOn){
// 				//
// 				// i we are here it means the pi is n
// 				// and voltage has dropped into
// 				// comma range so
// 				faultData="Enter WPS";
// 				sendWPSAlert(time, faultData, batteryVoltage);
// 				lcd.clear();
// 				lcd.setRGB(225, 0, 0);
// 				lcd.setCursor(0, 0);
// 				lcd.print(F("Comma Alert Sent"));
// 			}
// 		}
// 	}
// }

boolean PowerManager::processDefaultCommands(String command)
{
	boolean processed=false;

	if(command==F("TestWPSSensor")){
		float batteryVoltage = getBatteryVoltage();
		float current = getCurrentFromBattery();
		int stateOfCharge= GeneralFunctions::getStateOfCharge(batteryVoltage);
		boolean result = dataStorageManager.testWPSSensor( batteryVoltage,  current,  stateOfCharge,  operatingStatus);
		if(result){
			_HardSerial.println(F("Ok-TestWPSSensor"));
		}else{
			_HardSerial.println(F("Failure-TestWPSSensor"));
		}
		_HardSerial.flush();
		processed=true;
	}else if(command==F("TestLifeCycle")){
		long now = timeManager.getCurrentTimeInSeconds();
		dataStorageManager.storeLifeCycleEvent(now, LIFE_CYCLE_EVENT_END_COMMA,LIFE_CYCLE_EVENT_COMMA_VALUE);
		_HardSerial.println(F("Ok-TestLifeCycle"));
		_HardSerial.flush();

	}else if(command==F("ListFiles")){
		_HardSerial.println(" ");
		_HardSerial.println(" ");
		//_HardSerial.println(sensorDirName);
		float total = dataStorageManager.listFiles();

		_HardSerial.println(" ");

		_HardSerial.print(F("Used (Kb):  "));
		_HardSerial.println(total);

		_HardSerial.println("");
		_HardSerial.println(F("Ok-ListFiles"));
		_HardSerial.flush();
		processed=true;
	}else if(command==F("Ping")){

		_HardSerial.println(F("Ok-Ping"));
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("SetTime"))){

		if(capacitorVoltage==0){
			//
			// we are in normal operation
			//
			_HardSerial.println(F("Failure-SetTime"));
			_HardSerial.flush();

		}else{
			boolean result = timeManager.setTime(command);
			if(result){
				_HardSerial.println(F("Ok-SetTime"));
			}else{
				_HardSerial.println(F("Failure-SetTime"));
			}

			_HardSerial.flush();
		}
		processed=true;

	}else if(command.startsWith(F("GetTime"))){
		RTCInfoRecord r =timeManager.getCurrentDateTime();
		_HardSerial.print(r.date);
		_HardSerial.print("/");
		_HardSerial.print(r.month);
		_HardSerial.print("/");
		int year = r.year-2000;
		_HardSerial.print(year);
		_HardSerial.print(" ");
		_HardSerial.print(r.hour);
		_HardSerial.print(":");
		_HardSerial.print(r.minute);
		_HardSerial.print(":");
		_HardSerial.println(r.second);

		
		_HardSerial.flush();
		_HardSerial.println(F("Ok-GetTime"));
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("VerifyUserCode"))){
		String codeInString = GeneralFunctions::getValue(command, '#', 1);
		long userCode = codeInString.toInt();
		boolean validCode = secretManager.checkCode( userCode);
		String result="Failure-Invalid Code";
		if(validCode)result="Ok-Valid Code";
		_HardSerial.println(result);
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("GetCommandCodeGenerationTime"))){

		long secOrig =timeManager.getTimeForCodeGeneration();

		_HardSerial.print(F("secOrig="));
		_HardSerial.println(secOrig);
		_HardSerial.flush();
		char secretCode[SHARED_SECRET_LENGTH];
		secretManager.readSecret(secretCode);
		_HardSerial.print(F("secretCode="));
		_HardSerial.println(secretCode);
		_HardSerial.flush();

		TOTP totp = TOTP(secretCode);
		long code=totp. gen_code(secOrig);


		//long code =secretManager.generateCode();
		_HardSerial.print(F("code="));
		_HardSerial.println(code);
		_HardSerial.println(F("Ok-GetCommandCodeGenerationTime"));
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("GetCommandCode"))){

		long code =secretManager.generateCode();
		//
		// patch a bug in the totp library
		// if the first digit is a zero, it
		// returns a 5 digit number
		if(code<100000){
			_HardSerial.print(F("0"));
			_HardSerial.println(code);
		}else{
			_HardSerial.println(code);
		}
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("GetSecret"))){
		if(capacitorVoltage==0){
			//
			// we are in normal operation
			//
			_HardSerial.println(F("Failure-GetSecret"));
			_HardSerial.flush();
		}else{
			char secretCode[SHARED_SECRET_LENGTH];
			secretManager.readSecret(secretCode);
			_HardSerial.println(secretCode);
			_HardSerial.println(F("Ok-GetSecret"));
			_HardSerial.flush();
			delay(delayTime);
		}

		processed=true;
	} else if(command.startsWith(F("SetSecret"))){
		if(capacitorVoltage==0){
			//
			// we are in normal operation
			//
			_HardSerial.println(F("Failure-SetSecret"));
			_HardSerial.flush();
		}else{
			String secret = GeneralFunctions::getValue(command, '#', 1);
			int numberDigits = GeneralFunctions::getValue(command, '#', 2).toInt();
			int periodSeconds = GeneralFunctions::getValue(command, '#', 3).toInt();
			secretManager.saveSecret(secret, numberDigits, periodSeconds);

			_HardSerial.println(F("Ok-SetSecret"));
			_HardSerial.flush();
		}
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("PulseStart"))){
		inPulse=true;
		waitingManualPiStart=false;
		pulseStartTime = GeneralFunctions::getValue(command, '#', 1);
		_HardSerial.println(F("Ok-PulseStart"));
		_HardSerial.flush();
		lcd.clear();
		lcd.setRGB(255,0,0);
		processed=true;
	}else if(command.startsWith(F("PulseFinished"))){
		pulseStopTime = GeneralFunctions::getValue(command, '#', 1);
		inPulse=false;
		_HardSerial.println(F("Ok-PulseFinished"));
		_HardSerial.flush();
		lcd.clear();
		lcd.setRGB(255,255,255);
		processed=true;


	}else if(command.startsWith(F("IPAddr"))){
		currentIpAddress = GeneralFunctions::getValue(command, '#', 1);
		_HardSerial.println(F("Ok-IPAddr"));
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("SSID"))){
		currentSSID = GeneralFunctions::getValue(command, '#', 1);
		_HardSerial.println(F("Ok-currentSSID"));
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if(command.startsWith(F("HostMode"))){
		_HardSerial.println(F("Ok-HostMode"));
		_HardSerial.flush();
		delay(delayTime);
		isHost=true;
		processed=true;
	}else if(command.startsWith(F("NetworkMode"))){
		_HardSerial.println(F("Ok-NetworkMode"));
		_HardSerial.flush();
		delay(delayTime);
		isHost=false;
		processed=true;
	}else if(command.startsWith(F("EnterWPS"))){
		//EnterWPS#10#45#30#1
		secondsToTurnPowerOff = (long)GeneralFunctions::getValue(command, '#', 1).toInt();
		secondsToNextPiOn = (long)GeneralFunctions::getValue(command, '#', 2).toInt();
		wpsPulseFrequencySeconds = GeneralFunctions::getValue(command, '#', 3).toInt();
		int pauseDuringWPSi = GeneralFunctions::getValue(command, '#', 4).toInt();
		if(pauseDuringWPSi==1)pauseDuringWPS=true;
		else pauseDuringWPS=false;
		waitingForWPSConfirmation=false;
		wpsCountdown=true;
		operatingStatus="WPS";
		wpsCountDownStartSeconds= timeManager.getCurrentTimeInSeconds();
		currentSecondsToPowerOff=0L;

		_HardSerial.println(F("Ok-EnterWPS"));
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("ExitWPS"))){

		_HardSerial.println(F("Ok-ExitWPS"));
		_HardSerial.flush();
		inWPS=false;
		operatingStatus="Normal";
		currentSecondsToPowerOff=0L;
		wpsCountdown=false;
		processed=true;
	}else if(command.startsWith(F("UpdateWPSParameters"))){
		static_str<16> minWPSVoltageS = GeneralFunctions::getValue(command, '#', 1);
		minWPSVoltage=minWPSVoltageS.toFloat();

		minWPSVoltage = GeneralFunctions::getValue(command, '#', 1).toFloat();
		enterWPSVoltage = GeneralFunctions::getValue(command, '#', 2).toFloat();
		exitWPSVoltage = GeneralFunctions::getValue(command, '#', 3).toFloat();

		secondsToForcedWPS = GeneralFunctions::getValue(command, '#', 4).toInt();
		_HardSerial.println(F("Ok-UpdateWPSParameters"));
		_HardSerial.flush();

		processed=true;

	}else if(command.startsWith(F("GetRememberedValueData"))){
		//GetRememberedValueData#0
		int transferData = GeneralFunctions::getValue(command, '#', 1).toInt();
		boolean result = dataStorageManager.readUntransferredFileFromSDCard( transferData,true, RememberedValueDataDirName);
		if(result){
			_HardSerial.println(F("Ok-GetRememberedValueData"));
		}else {
			static_str<44> text;
			text.format(F("Failure-error opening %s/%s"),RememberedValueDataDirName,unstraferedFileName);
			_HardSerial.println(text);
		}
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("GetLifeCycleData"))){
		//GetLifeCycleData#0
		int transferData = GeneralFunctions::getValue(command, '#', 1).toInt();
		boolean result = dataStorageManager.readUntransferredFileFromSDCard( transferData,true, LifeCycleDataDirName);
		if(result){
			_HardSerial.println(F("Ok-GetLifeCycleData"));
		}else {
			static_str<44> text;
			text.format(F("Failure-error opening %s/%s"), LifeCycleDataDirName, unstraferedFileName);
			_HardSerial.println(text);
		}
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("GetWPSSensorData"))){
		//GetWPSSensorData#0
		//GetLifeCycleData#0
		int transferData = GeneralFunctions::getValue(command, '#', 1).toInt();
		boolean result = dataStorageManager.readUntransferredFileFromSDCard( transferData,true, WPSSensorDataDirName);
		if(result){
			_HardSerial.println(F("Ok-GetWPSSensorData"));
		}else {

			static_str<44> text;
			text.format(F("Failure-error opening /%s/%s"), WPSSensorDataDirName, unstraferedFileName);
			_HardSerial.println(text);

		}
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("GetHistoricalWPSSensorData"))){

		int date = GeneralFunctions::getValue(command, '#', 1).toInt();
		int month = GeneralFunctions::getValue(command, '#', 2).toInt();
		int year = GeneralFunctions::getValue(command, '#', 3).toInt();
		boolean result  =dataStorageManager.getHistoricalData( WPSSensorDataDirName,  date,  month,  year);
		if(result){
			_HardSerial.println(F("Ok-GetWPSSensorDataHistory"));
		}else {
			static_str<44> text;
			text.format(F("Failure-error opening %s/%s"), WPSSensorDataDirName, unstraferedFileName);

			_HardSerial.println(text);
		}
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("GetHistoricalLifeCycleData"))){
		//GetHistoricalLifeCycleData#12#1#19
		int date = GeneralFunctions::getValue(command, '#', 1).toInt();
		int month = GeneralFunctions::getValue(command, '#', 2).toInt();
		int year = GeneralFunctions::getValue(command, '#', 3).toInt();
		boolean result  = dataStorageManager.getHistoricalData( LifeCycleDataDirName,  date,  month,  year);
		if (result) {
			_HardSerial.println(F("Ok-GetHistoricalLifeCycleData"));
		}else {
			static_str<44> text;
			text.format(F("Failure-error opening %s/%s"), LifeCycleDataDirName, unstraferedFileName);
			_HardSerial.println(text);
		}
		_HardSerial.flush();
		processed=true;
	}else if(command.startsWith(F("GetHistoricalRememberedValueData"))){
		//GetHistoricalLifeCycleData#12#1#19
		int date = GeneralFunctions::getValue(command, '#', 1).toInt();
		int month = GeneralFunctions::getValue(command, '#', 2).toInt();
		int year = GeneralFunctions::getValue(command, '#', 3).toInt();
		boolean result  = dataStorageManager.getHistoricalData( RememberedValueDataDirName,  date,  month,  year);
		if (result) {
			_HardSerial.println(F("Ok-GetHistoricalRememberedValueData"));
		}else {
			static_str<44> text;
			text.format(F("Failure-error opening %s/%s"), RememberedValueDataDirName, unstraferedFileName);

			_HardSerial.println(text);
		}
		_HardSerial.flush();
		processed=true;
	}else if (command == "AsyncData" ){
		_HardSerial.println(F("Ok-No Data"));
		_HardSerial.flush();
		processed=true;
	}else if (command.startsWith(F("FaultData")) ){
		//_HardSerial.println(faultData);
		if(faultData==F("Enter WPS")){

			_HardSerial.print(F("Fault#WPS Alert#Enter WPS#"));
			_HardSerial.print(secretManager.generateCode());

			_HardSerial.print(F("#@On Load:Notify And Shutdown:Voltage At WPS#"));
			_HardSerial.println(getBatteryVoltage());
			waitingForWPSConfirmation=true;

		}else{
			_HardSerial.println(F("Ok"));
		}

		_HardSerial.flush();
		faultData="";
		delay(delayTime);
		processed=true;
	}else if (command.startsWith(F("UserCommand")) ){
		//
		// this function is not used in Ra2
		// because Ra2 has no buttons
		// but in the case that a teleonome does have
		//human interface buttons connected to the microcontrller
		// or there is a timer, here is where it will
		_HardSerial.println(F("Ok-UserCommand"));
		_HardSerial.flush();
		delay(delayTime);
		processed=true;
	}else if (command.startsWith(F("TimerStatus")) ){
		//
		// this function is not used in Ra2
		// because Ra2 has no btimers
		// but in the case that a teleonome does have
		//human interface buttons connected to the microcontrller
		// or there is a timer, here is where it will be
		_HardSerial.println(F("Ok-TimerStatus"));
		_HardSerial.flush();
		delay(delayTime);
		processed=true;

	}
	return processed;
}
void PowerManager::endOfLoopProcessing(){
	long now = timeManager.getCurrentTimeInSeconds();
	int loopConsumingPowerSeconds = timeManager.getCurrentTimeInSeconds()-now -poweredDownInLoopSeconds;
	dailyBatteryOutEnergy+= loopConsumingPowerSeconds*getCurrentFromBattery()/3600;
	hourlyBatteryOutEnergy+= loopConsumingPowerSeconds*getCurrentFromBattery()/3600;
	dailyPoweredDownInLoopSeconds+=poweredDownInLoopSeconds;
	hourlyPoweredDownInLoopSeconds+=poweredDownInLoopSeconds;
}

float PowerManager::getLockCapacitorVoltage(){
	long  lockCapacitorValue = analogRead(LOCK_CAPACITOR_PIN);
	float capacitorVoltage= lockCapacitorValue * (5.0 / 1023.0);
	return capacitorVoltage;
}
void PowerManager::toggleWDT(){
	if(f_wdt == 0)
	{
		f_wdt=1;
	}
	else
	{
		//_HardSerial.println(F("WDT Overrun!!!"));
	}
}

void PowerManager::printBaseSensorStringToSerialPort(){
	lcd.clear();
	lcd.setCursor(0,0);
	long now = millis();
	float batteryVoltage = getBatteryVoltage();
	lcd.print(F("S1:"));
	long dur = millis()-now;
	lcd.print(dur);

	byte internalBatteryStateOfCharge = GeneralFunctions::getStateOfCharge(batteryVoltage);
	now = millis();

	float currentValue = getCurrentFromBattery();
	lcd.print(F(" S2:"));
	 dur = millis()-now;
		lcd.print(dur);
	lcd.setCursor(0,1);
	now = millis();
	float capacitorVoltage= getLockCapacitorVoltage();
	lcd.print(F("S3:"));
	 dur = millis()-now;
		lcd.print(dur);

	//boolean piIsOn = digitalRead(PI_POWER_PIN);
	// Generate the SensorData String
	String sensorDataString="";
	//
	// Sensor Request Queue Position 1
	//
	now = millis();
	_HardSerial.print(static_str<16>(batteryVoltage,4,1));
	_HardSerial.print(F("#"));
	lcd.print(F("S4:"));
	 dur = millis()-now;
		lcd.print(dur);
	//
	// Sensor Request Queue Position 2
	//
	_HardSerial.print(static_str<16>(currentValue,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 3
	//
	_HardSerial.print(static_str<16>(capacitorVoltage,2,1));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 4
	//
	_HardSerial.print( internalBatteryStateOfCharge);
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 5
	//
	_HardSerial.print( operatingStatus);
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 6
	//
	_HardSerial.print(static_str<16>(dailyMinBatteryVoltage,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 7
	//
	_HardSerial.print(static_str<16>(dailyMaxBatteryVoltage,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 8
	//
	_HardSerial.print(static_str<16>(dailyMinBatteryCurrent,4, 0)) ;
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 9
	//
	_HardSerial.print(static_str<16>(dailyMaxBatteryCurrent,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 10
	//
	_HardSerial.print(static_str<16>(dailyBatteryOutEnergy,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 11
	//
	_HardSerial.print(static_str<16>(dailyPoweredDownInLoopSeconds,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 12
	//
	_HardSerial.print(static_str<16>(hourlyBatteryOutEnergy,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 13
	//
	_HardSerial.print(static_str<16>(hourlyPoweredDownInLoopSeconds,4,0));
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 14
	//
	now = millis();
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(F("S5:"));
	long totalDiskUse=dataStorageManager.getDiskUsage();
	dur = millis()-now;

	lcd.print(dur);
	_HardSerial.print(totalDiskUse/1024);
	_HardSerial.print(F("#"));
	//
	// Sensor Request Queue Position 15
	//
	_HardSerial.print(pauseDuringWPS);
	_HardSerial.print(F("#"));
	_HardSerial.flush();
}
