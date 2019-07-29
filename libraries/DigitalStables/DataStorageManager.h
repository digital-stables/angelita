/*
 * DataStorageManager.h
 *
 *  Created on: 30 Apr 2019
 *      Author: arifainchtein
 */

#ifndef LIBRARIES_DIGITALSTABLES_DATASTORAGEMANAGER_H_
#define LIBRARIES_DIGITALSTABLES_DATASTORAGEMANAGER_H_
#include "Arduino.h"
#include <SD.h>
#include <GeneralFunctions.h>
#include <LCDDisplay.h>
#include <TimeManager.h>
#include <DiscreteRecord.h>
#include <DataStorageManagerInitParams.h>
#include <WPSSensorRecord.h>
#include <LCDDisplay.h>

class DataStorageManager {
public:
	DataStorageManager();

	DataStorageManager(DataStorageManagerInitParams&  d, TimeManager & t, GeneralFunctions  & f, HardwareSerial& serial, LCDDisplay& l);
	virtual boolean start()=0;
	virtual boolean readUntransferredFileFromSDCardByDate(int moveData, boolean sendToSerial,const char *dirName, int date, int month, int year)=0;
	virtual boolean readUntransferredFileFromSDCard(int moveData, boolean sendToSerial, const char *dirName)=0;
	virtual void storeRememberedValue(long time, const char *name, float value, String unit)=0;
	virtual void storeDiscreteRecord( DiscreteRecord& discreteRec)=0;
	virtual boolean readDiscreteRecord(uint16_t index,DiscreteRecord& rec)=0;
	virtual boolean openDiscreteRecordFile()=0;
	virtual void closeDiscreteRecordFile()=0;

	virtual void storeEventRecord(const char *EventRecordDirName, const byte *eventData,int eventSize )=0;
	virtual boolean readEventRecord(uint16_t index, byte *eventData,int eventSize, boolean moveData)=0;
	virtual boolean openEventRecordFile(const char *filename)=0;
	virtual void closeEventRecordFile(boolean)=0;


	virtual float searchRememberedValue(const char *label, int date, int month, int year, char *whatToSearchFor)=0;
	virtual void storeLifeCycleEvent(long time, const char *eventType, int eventValue)=0;
	virtual long printDirectory(File dir, int numTabs)=0;
	virtual long getDiskUsage()=0;
	virtual long getSDCardDiskUse(File dir )=0;
	virtual boolean getHistoricalData(const char *dirName, int date, int month, int year)=0;
	virtual void saveWPSSensorRecord(WPSSensorRecord anWPSSensorRecord)=0;

	//
	// Functions that represent Serial commands
	//
	virtual boolean testWPSSensor(float batteryVoltage, float current, int stateOfCharge, String operatingStatus)=0;
	virtual float listFiles()=0;
	
	virtual ~DataStorageManager();
};



#endif /* LIBRARIES_DIGITALSTABLES_DATASTORAGEMANAGER_H_ */
