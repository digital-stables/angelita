/*
 * NoDataStorageManager.cpp
 *
 *  Created on: 2 May 2019
 *      Author: arifainchtein
 */

#include <NoDataStorageManager.h>
#include <WPSSensorRecord.h>

#include "Arduino.h"

#include <GeneralFunctions.h>
#include <DataStorageManager.h>
#include <DataStorageManagerInitParams.h>
#include <LCDDisplay.h>
#include <TimeManager.h>
#include <DiscreteRecord.h>
#include <Wire.h>





NoDataStorageManager::NoDataStorageManager(DataStorageManagerInitParams& d, TimeManager& t, GeneralFunctions& f,HardwareSerial& serial, LCDDisplay& l ): dataStorageManagerInitParams(d), timeManager(t), generalFunctions(f), _HardSerial(serial), lcdDisplay(l)
{}

boolean NoDataStorageManager::start(){

	

}

boolean NoDataStorageManager::readUntransferredFileFromSDCardByDate(int moveData, boolean sendToSerial,const char *dirName, int date, int month, int year){

}
boolean NoDataStorageManager::readUntransferredFileFromSDCard(int moveData, boolean sendToSerial, const char *dirName){

}
void NoDataStorageManager::storeRememberedValue(long time, const char *name, float value, String unit){

}
void NoDataStorageManager::storeDiscreteRecord( DiscreteRecord &discreteRec){

}
boolean NoDataStorageManager::readDiscreteRecord(uint16_t index,DiscreteRecord& rec){

}
boolean NoDataStorageManager::openDiscreteRecordFile(){

}
void NoDataStorageManager::closeDiscreteRecordFile(){

}

void NoDataStorageManager::storeEventRecord(const char *EventRecordDirName, const byte *eventData,int eventSize ){

}

boolean NoDataStorageManager::readEventRecord(uint16_t index, byte *eventData,int eventSize, boolean moveData){

}
boolean NoDataStorageManager::openEventRecordFile(const char *filename){

}
void NoDataStorageManager::closeEventRecordFile(boolean){

}




void storeEventRecord( byte eventData[]){

}
boolean readEventRecord(uint16_t index, byte *eventData,int eventSize){

}
boolean openEventRecordFile(const char *filename){

}
void closeEventRecordFile(){

}

float NoDataStorageManager::searchRememberedValue(const char *label, int date, int month, int year, char *whatToSearchFor){

}
void NoDataStorageManager::storeLifeCycleEvent(long time, const char *eventType, int eventValue){

}
long NoDataStorageManager::printDirectory(File dir, int numTabs){

}
long NoDataStorageManager::getDiskUsage(){

}
long NoDataStorageManager::getSDCardDiskUse(File dir ){

}
boolean NoDataStorageManager::getHistoricalData(const char *dirName, int date, int month, int year){

}
void NoDataStorageManager::saveWPSSensorRecord(WPSSensorRecord anWPSSensorRecord){

}

//
// Functions that represent Serial commands
//
boolean NoDataStorageManager::testWPSSensor(float batteryVoltage, float current, int stateOfCharge, String operatingStatus){

}
float NoDataStorageManager::listFiles(){

}
boolean NoDataStorageManager::setTime(String){

}





NoDataStorageManager::~NoDataStorageManager() {
	// TODO Auto-generated destructor stub
}

