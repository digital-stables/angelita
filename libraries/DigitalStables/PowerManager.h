/*
  PowerManager.h - Library Managing the Power Management
  Created by Ari Fainchtein, March 13, 2019.
  Released into the public domain.
 */
#include "Arduino.h"

#ifndef PowerManager_h
#define PowerManager_h
#include <DataStorageManager.h>
#include <LCDDisplay.h>
#include <WPSSensorRecord.h>
#include <SD.h>
#include <GeneralFunctions.h>
#include <TimeManager.h>
#include <SecretManager.h>


	DEFINE_RSTR(UNIT_VOLT ,"Volt");
	DEFINE_PSTR(UNIT_SECONDS,"sec");
	DEFINE_PSTR(UNIT_MILLI_AMPERES ,"mA");
	DEFINE_PSTR(UNIT_MILLI_AMPERES_HOURS ,"mAh");
	DEFINE_PSTR(FORCED_PI_TURN_OFF ,"FPTO");
	DEFINE_PSTR(BATTERY_VOLTAGE_BEFORE_PI_ON ,"BVBTPO");
	DEFINE_PSTR(BATTERY_VOLTAGE_ATER_PI_ON,"BVATPO");
	DEFINE_PSTR(BATTERY_VOLTAGE_DIFFERENTIAL_AFTER_PI_ON ,"BVDATPO");
	DEFINE_PSTR(PI_TURN_OFF ,"Pi Turn Off");

	DEFINE_RSTR(UNIT_NO_UNIT ," ");
	DEFINE_RSTR(UNIT_PERCENTAGE,"%");


	DEFINE_PSTR(LIFE_CYCLE_EVENT_FORCED_START_WPS ,"FSWPS");
	DEFINE_PSTR(LIFE_CYCLE_MANUAL_SHUTDOWN    ,"MS");
	DEFINE_PSTR(LIFE_CYCLE_EVENT_START_WPS    ,"SW");
	DEFINE_PSTR(LIFE_CYCLE_EVENT_END_WPS     ,"EWS");
	DEFINE_PSTR(LIFE_CYCLE_EVENT_START_COMMA ,"SC");
	DEFINE_RSTR(LIFE_CYCLE_EVENT_END_COMMA ,"EC");

	DEFINE_PSTR( LIFE_CYCLE_EVENT_START_EXTENDED_OPERON_EXECUTION,"SEOE");
	DEFINE_PSTR( LIFE_CYCLE_EVENT_END_EXTENDED_OPERON_EXECUTION,"EEOE");
	DEFINE_PSTR( LIFE_CYCLE_EVENT_UPDATE_EXTENDED_OPERON_EXECUTION,"UEOE");

	DEFINE_PSTR(DAILY_STATS_TIMESTAMP,"DST");
	DEFINE_PSTR(DAILY_MINIMUM_BATTERY_VOLTAGE,"DMiBV");
	DEFINE_PSTR(DAILY_MAXIMUM_BATTERY_VOLTAGE,"DMaBV");
	DEFINE_PSTR(DAILY_MINIMUM_BATTERY_CURRENT,"DMiBC");
	DEFINE_PSTR(DAILY_MAXIMUM_BATTERY_CURRENT,"DMaBC");
	DEFINE_PSTR(DAILY_ENERGY,"DE");
	DEFINE_PSTR(DAILY_POWERED_DOWN_IN_LOOP_SECONDS,"DPDInLS");
	DEFINE_PSTR(HOURLY_ENERGY,"HE");
	DEFINE_PSTR(HOURLY_POWERED_DOWN_IN_LOOP_SECONDS,"HPDILS");
	DEFINE_PSTR(HOURLY_OPERATING_IN_LOOP_SECONDS,"HOILS");
	DEFINE_PSTR(DiscreteDirName,"Discrete");
	
	DEFINE_RSTR(WPSSensorDataDirName,"WPSSensr");
	DEFINE_RSTR(LifeCycleDataDirName,"LifeCycl");
	DEFINE_RSTR(RememberedValueDataDirName,"RememVal");
	DEFINE_RSTR(unstraferedFileName ,"Untransf.txt");

class PowerManager{


public:

	String operatingStatus ="Normal";


	int currentViewIndex=0;

	boolean isHost=true;
	long poweredDownInLoopSeconds;
	float dailyMinBatteryVoltage=0;
	float dailyMaxBatteryVoltage=0;

	float dailyMinBatteryCurrent=0;
	float dailyMaxBatteryCurrent=0;
	float dailyBatteryOutEnergy=0;
	float dailyPoweredDownInLoopSeconds=0;

	float hourlyBatteryOutEnergy=0;
	float hourlyPoweredDownInLoopSeconds=0;
	boolean pauseDuringWPS=false;
	boolean inPulse=false;
	String pulseStartTime="";
	String pulseStopTime="";
	#define PI_POWER_PIN 0
	#define IPHONE_POWER_PIN 1
	
	long secondsToTurnPowerOff = 30;
	long secondsToNextPiOn=90L;
	long currentSecondsToPowerOff=0L;
	boolean wpsCountdown=false;
	boolean wpsSleeping=false;
	boolean inWPS=false;
	float minWPSVoltage=12.4;
	float enterWPSVoltage=12.6;
	float exitWPSVoltage=12.8;
	long lastWPSStartUp=0L;
	long lastWPSRecordSeconds=0L;
	int wpsPulseFrequencySeconds=60;
	boolean waitingForWPSConfirmation=false;
	long currentSleepStartTime=0L;
	volatile int f_wdt=1;
	long wpsCountDownStartSeconds=0L;

	TimeManager  timeManager;
	SecretManager  secretManager;
	DataStorageManager  &dataStorageManager;
	HardwareSerial _HardSerial;
	LCDDisplay&  lcd;

	constexpr static const int LIFE_CYCLE_EVENT_AWAKE_VALUE=3;
	constexpr static const int LIFE_CYCLE_EVENT_WPS_VALUE=2;
	constexpr static const int LIFE_CYCLE_EVENT_COMMA_VALUE=1;

	PowerManager();
	PowerManager(LCDDisplay & l , SecretManager & s, DataStorageManager & sd, TimeManager & t, HardwareSerial& serial);
	void start();
	void hourlyTasks(long time, int previousHour );
	void dailyTasks(long time, int yesterdayDate, int yesterdayMonth, uint16_t yesterdayYear );
	void monthlyTasks(long time);
	void yearlyTasks(long time);
	float getCurrentFromBattery(void);
	float getCurrentInputFromSolarPanel(void);
	float getSolarPanelVoltage();
	float getBatteryVoltage();
	void initializeWDT();
	void enterArduinoSleep(void);
	void pauseWPS();
	void sendWPSAlert(long time, char *faultData, int batteryVoltage);
	void turnPiOffForced(long time);
	void turnPiOff(long time);
	void turnPiOn(long time);
	void defineState();
	boolean processDefaultCommands(String command);
	void endOfLoopProcessing();
	float getLockCapacitorVoltage();
	void toggleWDT();
	void printBaseSensorStringToSerialPort();
};

#endif
