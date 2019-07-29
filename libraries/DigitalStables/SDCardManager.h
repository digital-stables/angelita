/*
  SDCardManager.h - Library Managing the high level functions of the Sd Card
  Created by Ari Fainchtein, Feb 13, 2019.
  Released into the public domain.
 */
#ifndef SDCardManager_h
#define SDCardManager_h
#include <WPSSensorRecord.h>
#include <LCDDisplay.h>
#include "Arduino.h"
#include <SD.h>
#include <GeneralFunctions.h>
#include <DataStorageManager.h>
#include <LCDDisplay.h>
#include <TimeManager.h>
#include <DiscreteRecord.h>
#include <DataStorageManagerInitParams.h>

class SDCardManager: public DataStorageManager{

	TimeManager & timeManager;
	GeneralFunctions & generalFunctions;
	HardwareSerial& _HardSerial;
	LCDDisplay&  lcdDisplay;
	DataStorageManagerInitParams& dataStorageManagerInitParams;
	const char  *WPSSensorDataDirName="WPSSensr";
	const char  *LifeCycleDataDirName="LifeCycl";
	const char  *RememberedValueDataDirName  = "RememVal";
	const char  *DiscreteRecordDirName  = "Discrete";
	const char  *EventRecordDirName  = "Events";
	const char  *unstraferedFileName ="Untransf.txt";

public:
	SDCardManager(DataStorageManagerInitParams& d, TimeManager & t, GeneralFunctions  & f, HardwareSerial& serial, LCDDisplay& l);
	boolean start();
	boolean readUntransferredFileFromSDCardByDate(int moveData, boolean sendToSerial,const char *dirName, int date, int month, int year);
	boolean readUntransferredFileFromSDCard(int moveData, boolean sendToSerial, const char *dirName);
	void storeRememberedValue(long time, const char *name, float value, String unit);

	void storeDiscreteRecord( DiscreteRecord &discreteRec);
	boolean readDiscreteRecord(uint16_t index,DiscreteRecord& rec);
	boolean openDiscreteRecordFile();
	void closeDiscreteRecordFile();

	void storeEventRecord(const char *EventRecordDirName, const byte *eventData,int eventSize );
	boolean readEventRecord(uint16_t index, byte *eventData,int eventSize, boolean moveData);
	boolean openEventRecordFile(const char *filename);
	void closeEventRecordFile(boolean);

	float searchRememberedValue(const char *label, int date, int month, int year, char *whatToSearchFor);
	void storeLifeCycleEvent(long time, const char *eventType, int eventValue);
	long printDirectory(File dir, int numTabs);
	long getDiskUsage();
	long getSDCardDiskUse(File dir );
	boolean getHistoricalData(const char *dirName, int date, int month, int year);
	void saveWPSSensorRecord(WPSSensorRecord anWPSSensorRecord);

	//
	// Functions that represent Serial commands
	//
	boolean testWPSSensor(float batteryVoltage, float current, int stateOfCharge, String operatingStatus);
	float listFiles();
	
private:
	File dataFile;

};

#endif
