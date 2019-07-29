/*
 * EEPROMDataStorageManager.h
 *
 *  Created on: 2 May 2019
 *      Author: arifainchtein
 */

#ifndef LIBRARIES_DIGITALSTABLES_NODATASTORAGEMANAGER_H_
#define LIBRARIES_DIGITALSTABLES_NODATASTORAGEMANAGER_H_

#include <DataStorageManager.h>
#include <DataStorageManagerInitParams.h>

class NoDataStorageManager: public DataStorageManager {

	TimeManager & timeManager;
	GeneralFunctions & generalFunctions;
	HardwareSerial& _HardSerial;
	LCDDisplay&  lcdDisplay;
	DataStorageManagerInitParams& dataStorageManagerInitParams;
public:
	NoDataStorageManager();
	virtual ~NoDataStorageManager();
	
	NoDataStorageManager(DataStorageManagerInitParams& d, TimeManager & t, GeneralFunctions  & f, HardwareSerial& serial, LCDDisplay& l);
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
	boolean setTime(String);

	

};

#endif /* LIBRARIES_DIGITALSTABLES_NODATASTORAGEMANAGER_H_ */
