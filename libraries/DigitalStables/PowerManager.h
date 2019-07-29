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
	GeneralFunctions  generalFunctions;
	DataStorageManager  &dataStorageManager;
	HardwareSerial _HardSerial;
	LCDDisplay&  lcd;

	constexpr static const char *UNIT_VOLT ="Volt";
	constexpr static const char *UNIT_SECONDS="sec";
	constexpr static const char *UNIT_MILLI_AMPERES ="mA";
	constexpr static const char *UNIT_MILLI_AMPERES_HOURS ="mAh";
	constexpr static const char *UNIT_PERCENTAGE ="%";
	constexpr static const char *FORCED_PI_TURN_OFF ="FPTO";
	constexpr static const char *BATTERY_VOLTAGE_BEFORE_PI_ON ="BVBTPO";
	constexpr static const char *BATTERY_VOLTAGE_ATER_PI_ON="BVATPO";
	constexpr static const char *BATTERY_VOLTAGE_DIFFERENTIAL_AFTER_PI_ON ="BVDATPO";
	constexpr static const char *PI_TURN_OFF ="Pi Turn Off";
	constexpr static const char *UNIT_NO_UNIT =" ";


	constexpr static const char *LIFE_CYCLE_EVENT_FORCED_START_WPS ="FSWPS";
	constexpr static const char *LIFE_CYCLE_MANUAL_SHUTDOWN    ="MS";
	constexpr static const char *LIFE_CYCLE_EVENT_START_WPS    ="SW";
	constexpr static const char *LIFE_CYCLE_EVENT_END_WPS     ="EWS";
	constexpr static const char *LIFE_CYCLE_EVENT_START_COMMA ="SC";
	constexpr static const char *LIFE_CYCLE_EVENT_END_COMMA ="EC";
	constexpr static const int LIFE_CYCLE_EVENT_AWAKE_VALUE=3;
	constexpr static const int LIFE_CYCLE_EVENT_WPS_VALUE=2;
	constexpr static const int LIFE_CYCLE_EVENT_COMMA_VALUE=1;

	constexpr static const char * LIFE_CYCLE_EVENT_START_EXTENDED_OPERON_EXECUTION="SEOE";
	constexpr static const char * LIFE_CYCLE_EVENT_END_EXTENDED_OPERON_EXECUTION="EEOE";
	constexpr static const char * LIFE_CYCLE_EVENT_UPDATE_EXTENDED_OPERON_EXECUTION="UEOE";

	constexpr static const char *DAILY_STATS_TIMESTAMP="DST";
	constexpr static const char *DAILY_MINIMUM_BATTERY_VOLTAGE="DMiBV";
	constexpr static const char *DAILY_MAXIMUM_BATTERY_VOLTAGE="DMaBV";
	constexpr static const char *DAILY_MINIMUM_BATTERY_CURRENT="DMiBC";
	constexpr static const char *DAILY_MAXIMUM_BATTERY_CURRENT="DMaBC";
	constexpr static const char *DAILY_ENERGY="DE";
	constexpr static const char *DAILY_POWERED_DOWN_IN_LOOP_SECONDS="DPDInLS";
	constexpr static const char *HOURLY_ENERGY="HE";
	constexpr static const char *HOURLY_POWERED_DOWN_IN_LOOP_SECONDS="HPDILS";
	constexpr static const char *HOURLY_OPERATING_IN_LOOP_SECONDS="HOILS";
	const char  *WPSSensorDataDirName="WPSSensr";

	const char  *DiscreteDirName="Discrete";
	const char  *LifeCycleDataDirName="LifeCycl";
	const char  *RememberedValueDataDirName  = "RememVal";
	const char  *unstraferedFileName ="Untransf.txt";
	PowerManager();
	PowerManager(LCDDisplay & l , SecretManager & s, DataStorageManager & sd, TimeManager & t, GeneralFunctions  & f, HardwareSerial& serial);
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

protected:


private:


};

#endif
