/*
 * SDCardManager.cpp
 *
 *  Created on: 13 Feb. 2019
 *      Author: arifainchtein
 */


#include "Arduino.h"
#include <SDCardManager.h>
#include <GeneralFunctions.h>
#include <TimeManager.h>

#include <WPSSensorRecord.h>
#include <GravityRtc.h>
#include <RTCInfoRecord.h>
#include <DiscreteRecord.h>
#include <DataStorageManagerInitParams.h>

#include <SPI.h>
#include <SD.h>

#define SD_PIN 4

const char *MAXIMUM_VALUE="Max";
const char *MINIMUM_VALUE="Min";
const char *AVERAGE_VALUE="Avg";

extern char sensorDirName[10];
extern char lifeCycleFileName[10];
extern char remFileName[10];

File currentlyOpenFile;
const char *currentlyOpenFileName;
boolean cardOk=false;
SDCardManager::SDCardManager(DataStorageManagerInitParams& d,TimeManager& t,HardwareSerial& serial, LCDDisplay& l ):dataStorageManagerInitParams(d), timeManager(t),_HardSerial(serial), lcdDisplay(l)
{}

boolean SDCardManager::start(){
	//setup SD card
	_HardSerial.print("Initializing SD card...");

	if (!SD.begin(SD_PIN)) {
		//		_HardSerial.println("No SD-card.");
		//		_HardSerial.flush();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("No SD-card.") ;
		_HardSerial.print("No SD-card.");
		cardOk=false;
		return false;
	}else{
		// Check dir for db files
		if (!SD.exists(sensorDirName )) {
			//			_HardSerial.println("wpsSensorData Dir does not exist, creating...");
			//			_HardSerial.flush();
			SD.mkdir(sensorDirName);
		}
		if (!SD.exists(lifeCycleFileName)) {
			//			_HardSerial.println("LifeCycle Dir does not exist, creating...");
			//			_HardSerial.flush();
			SD.mkdir(lifeCycleFileName);
		}
		if (!SD.exists(remFileName)) {
			SD.mkdir(remFileName);
		}
		File sensorFile = SD.open(sensorDirName );
		long totalDiskUse=getSDCardDiskUse(sensorFile);

		File lifeCycleFile = SD.open(lifeCycleFileName );
		totalDiskUse+=getSDCardDiskUse(lifeCycleFile);

		File rememberedValueFile = SD.open(remFileName );
		totalDiskUse+=getSDCardDiskUse(rememberedValueFile);

		sensorFile.close();
		lifeCycleFile.close();
		rememberedValueFile.close();
		cardOk=true;
		_HardSerial.println("card initialized.");
		_HardSerial.flush();
		return true;

	}
}

//
// Functions that represents commands received via the serial port
//
boolean SDCardManager::testWPSSensor(float batteryVoltage, float current, int stateOfCharge, String operatingStatus){
	if(!cardOk)return false;
	long lastWPSRecordSeconds = timeManager.getCurrentTimeInSeconds();
	char fileName[25];
	snprintf(fileName, sizeof fileName, "/%s/%s", WPSSensorDataDirName, unstraferedFileName);

	File untransferredFile = SD.open(fileName, FILE_WRITE);
	if (untransferredFile) {
		// Write to file
		;

		untransferredFile.print(lastWPSRecordSeconds);
		untransferredFile.print("#");
		untransferredFile.print(batteryVoltage);
		untransferredFile.print("#");
		untransferredFile.print(current);
		untransferredFile.print("#");
		untransferredFile.print(stateOfCharge);
		untransferredFile.print("#");
		untransferredFile.print(operatingStatus);


		File sensorFile = SD.open(sensorDirName);
		long totalDiskUse=getDiskUsage();
		untransferredFile.print("#");
		untransferredFile.println(totalDiskUse/1024);


		untransferredFile.close(); // close the file
	}
}

float SDCardManager::listFiles(){
if(!cardOk)return -9999.0;
	File sensorFile = SD.open(sensorDirName );
	File lifeCycleFile = SD.open(lifeCycleFileName );
	File rememberedValueFile = SD.open(remFileName );

	long totalDiskUse=printDirectory(sensorFile, 1);
	_HardSerial.println(" ");
	_HardSerial.println(lifeCycleFileName);
	totalDiskUse+=printDirectory(lifeCycleFile, 1);
	_HardSerial.println(" ");
	_HardSerial.println(remFileName);
	totalDiskUse+=printDirectory(rememberedValueFile, 1);
	sensorFile.close();
	lifeCycleFile.close();
	rememberedValueFile.close();
	float total= (float)totalDiskUse/1024;
	return total;
}


//
// End of Functions that represents commands received via the serial port
//

boolean SDCardManager::readUntransferredFileFromSDCardByDate(int moveData, boolean sendToSerial,const char *dirName, int date, int month, int year){
	//GetRememberedValueData#0
if(!cardOk)return false;
	char fileName[25] = "/";
	snprintf(fileName, sizeof fileName, "/%s/%s", dirName, unstraferedFileName);

	File uf = SD.open(fileName, FILE_WRITE);
	File tf;
	boolean result=false;
	if(moveData==1){
		char fileNameTF[24];
		snprintf(fileNameTF, sizeof fileName, "/%s/%i_%i_%i.txt", dirName, date,month, year);
		tf = SD.open(fileNameTF, FILE_WRITE);
	}

	if (uf) {
		uf.seek(0);
		while (uf.available()){
			//
			// Read each line, send it to the serial port
			// and copy it into today's file
			String line = uf.readStringUntil('\n');
			if(sendToSerial)_HardSerial.print(line);
			if(moveData==1)tf.print(line);
		}
		uf.close(); // close the file
		if(moveData==1){
			tf.close(); // close the file
			//
			// since we just transferred the records and copy them
			// delete the file untransferredFile
			SD.remove(fileName);
		}
		result=true;
	}
	return result;
}


boolean SDCardManager::readUntransferredFileFromSDCard(int moveData, boolean sendToSerial, const char *dirName){
	if(!cardOk)return false;
	RTCInfoRecord anRTCInfoRecord = timeManager.getCurrentDateTime();
	int year = anRTCInfoRecord.year-2000;
	int month = anRTCInfoRecord.month-1;
	return readUntransferredFileFromSDCardByDate( moveData,  sendToSerial,  dirName,  anRTCInfoRecord.date,  month,  year);
}



void SDCardManager::storeRememberedValue(long time,const static_str<16>& name, float value, String unit){
	//File untransferredFile = SD.open("/" + RememberedValueDataDirName + "/" + unstraferedFileName, FILE_WRITE);
if(!cardOk)return ;
	static_str<28> untransferredFileName;
	untransferredFileName.format("/%s/%s",RememberedValueDataDirName,unstraferedFileName);
	File untransferredFile = SD.open(untransferredFileName, FILE_WRITE);

	if (untransferredFile) {
		// Write to file
		untransferredFile.print(time);
		untransferredFile.print("#");
		untransferredFile.print(name);
		untransferredFile.print("#");
		untransferredFile.print(value);
		untransferredFile.print("#");
		untransferredFile.println(unit);
		untransferredFile.close(); // close the file
	}
}



void SDCardManager::storeDiscreteRecord(DiscreteRecord &discreteRec){
	if(!cardOk)return;
	static_str<32> untransferredFileName;
	untransferredFileName.format("/%s/%s",DiscreteRecordDirName,unstraferedFileName);
	File untransferredFile = SD.open(untransferredFileName, FILE_WRITE);
	if (untransferredFile) {
		// Write to file
		untransferredFile.write((uint8_t *)&discreteRec, sizeof(discreteRec));
		untransferredFile.close(); // close the file
	}
}


boolean SDCardManager::openDiscreteRecordFile()
{
	if(!cardOk)return false;
	static_str<32> untransferredFileName;

	sprintf(untransferredFileName,"/%s/%s",DiscreteRecordDirName,unstraferedFileName);
	currentlyOpenFile = SD.open(untransferredFileName, FILE_WRITE);
	return currentlyOpenFile;
}

boolean SDCardManager::readDiscreteRecord(uint16_t index,DiscreteRecord& rec)
{
	if(!cardOk)return false;
	boolean toReturn=false;
	if (currentlyOpenFile) {
		toReturn = currentlyOpenFile.seek(index*sizeof(rec));
		if(toReturn){
			currentlyOpenFile.read((uint8_t*)&rec,sizeof(rec));
		}
	}
	return toReturn;
}

void SDCardManager::storeEventRecord(const char *EventRecordDirName, const byte *eventData,int eventSize ){
	if(!cardOk)return ;
	char untransferredFileName[25];
	sprintf(untransferredFileName,"/%s/%s",EventRecordDirName,unstraferedFileName);
	File untransferredFile = SD.open(untransferredFileName, FILE_WRITE);
	if (untransferredFile) {
		// Write to file
		untransferredFile.write(eventData, eventSize);
		untransferredFile.close(); // close the file
	}
}

boolean SDCardManager::openEventRecordFile(const char *filename)
{
	if(!cardOk)return false;
	char untransferredFileName[25];

	sprintf(untransferredFileName,"/%s/%s",EventRecordDirName,filename);
	currentlyOpenFile = SD.open(untransferredFileName, FILE_WRITE);
	currentlyOpenFileName=filename;
	return currentlyOpenFile;
}

boolean SDCardManager::readEventRecord(uint16_t index, byte *eventData,int eventSize, boolean moveData)
{
	if(!cardOk)return false;
	boolean toReturn=false;
	if (currentlyOpenFile) {
		File tf;
		if(moveData==1){

			char fileName[25] = "/";
			snprintf(fileName, sizeof fileName, "/%s/%s", EventRecordDirName, currentlyOpenFileName);





			char fileNameTF[24];
			RTCInfoRecord anRTCInfoRecord = timeManager.getCurrentDateTime();
			int year = anRTCInfoRecord.year-2000;
			int month = anRTCInfoRecord.month-1;
			snprintf(fileNameTF, sizeof fileName, "/%s/%s_%i_%i_%i.txt", EventRecordDirName,currentlyOpenFileName, anRTCInfoRecord.date,month, year);
			tf = SD.open(fileNameTF, FILE_WRITE);
		}

		toReturn = currentlyOpenFile.seek(index*eventSize);
		if(toReturn){
			currentlyOpenFile.read(eventData,eventSize);

			if(moveData==1){
				tf.write(eventData, eventSize);
			}

		}
		if(moveData==1){
			tf.close();
		}
	}
	return toReturn;
}

void SDCardManager::closeDiscreteRecordFile()
{
	if(currentlyOpenFile){
		currentlyOpenFile.close();
	}
}

void SDCardManager::closeEventRecordFile(boolean clearEventFile)
{
	if(currentlyOpenFile){
		currentlyOpenFile.close();
		if(clearEventFile){
			SD.remove(currentlyOpenFileName);
		}
	}
}

float SDCardManager::searchRememberedValue(const char *label, int date, int month, int year, char *whatToSearchFor){
if(!cardOk)return -9999.0;
	float result=-9999;
	String line="";
	char anyLabel[50];
	float value=0;
	int sampleCount=0;
	int sampleSum=0;
	char fileName[32];
	sprintf(fileName,"/%s/%i_%i_%i/.txt",RememberedValueDataDirName,date,month,year);
	File todayFile = SD.open(fileName, FILE_WRITE);


	//File todayFile = SD.open("/" + RememberedValueDataDirName + "/" + date + "_" +  month + "_" + year + ".txt", FILE_WRITE);

	if (todayFile) {
		todayFile.seek(0);
		while (todayFile.available()){
			//
			// Read each line, send it to the serial port
			// and copy it into today's file
			line = todayFile.readStringUntil('\n');

			GeneralFunctions::getValue(line, '#', 1).toCharArray(anyLabel, sizeof anyLabel);
			if(strcmp(label, anyLabel) == 0){
				value =GeneralFunctions::getValue(line, '#', 2).toFloat();
				if(whatToSearchFor == MAXIMUM_VALUE){
					if(value>result)result=value;
				}else if(whatToSearchFor == MINIMUM_VALUE){
					if(value<result)result=value;
				}else if(whatToSearchFor == AVERAGE_VALUE){
					sampleCount++;
					sampleSum+=value;
				}
			}

		}
		todayFile.close(); // close the file
	}
	if(whatToSearchFor == AVERAGE_VALUE && sampleCount>0){
		result = sampleSum/sampleCount;
	}
	return result;
}

void SDCardManager::storeLifeCycleEvent(long time, const char *eventType, int eventValue){
if(!cardOk)return ;
	char untransferredFileName[25];
	sprintf(untransferredFileName,"/%s/%s",LifeCycleDataDirName,unstraferedFileName);
	File untransferredFile = SD.open(untransferredFileName, FILE_WRITE);


	//File untransferredFile = SD.open("/" + LifeCycleDataDirName + "/" + unstraferedFileName, FILE_WRITE);
	if (untransferredFile) {
		// Write to file
		untransferredFile.print(time);
		untransferredFile.print("#");
		untransferredFile.print(eventType);
		untransferredFile.print("#");
		untransferredFile.println(eventValue);
		untransferredFile.close(); // close the file
	}
}


boolean SDCardManager::getHistoricalData(const char *dirName, int date, int month, int year){
	if(!cardOk)return false;
	boolean result=false;
	char fileName[24];
	snprintf(fileName, sizeof fileName, "/%s/%i_%i_%i.txt", dirName, date,month, year);


	File todayFile1 = SD.open(fileName, FILE_WRITE);

	if (todayFile1) {
		todayFile1.seek(0);
		while (todayFile1.available()){
			//
			// Read each line, send it to the serial port
			// and copy it into today's file
			String line = todayFile1.readStringUntil('\n');
			_HardSerial.print(line);
		}
		todayFile1.close(); // close the file
		//
		// tell the hypothalamus we are done
		//
		result=true;
	}
	return result;
}

long SDCardManager::getSDCardDiskUse(File dir ) {
if(!cardOk)return -9999L;
	long total=0L;
	while(true) {

		File entry =  dir.openNextFile();
		if (! entry) {
			// no more files
			break;
		}
		if (entry.isDirectory()) {
			total+=getSDCardDiskUse(entry);
		} else {
			// files have sizes, directories do not
			total+=entry.size();
		}
		entry.close();
	}
	return total;
}


long SDCardManager::getDiskUsage(){
	if(!cardOk)return -9999L;
	File sensorFile = SD.open(sensorDirName );
	File lifeCycleFile = SD.open(lifeCycleFileName );
	File rememberedValueFile = SD.open(remFileName );

	long totalDiskUse=getSDCardDiskUse(sensorFile);
	totalDiskUse+=getSDCardDiskUse(lifeCycleFile);
	totalDiskUse+=getSDCardDiskUse(rememberedValueFile);

	sensorFile.close();
	lifeCycleFile.close();
	rememberedValueFile.close();

	return totalDiskUse;
}

long SDCardManager::printDirectory(File dir, int numTabs) {
	if(!cardOk)return -9999L;
	long total=0L;
	while(true) {

		File entry =  dir.openNextFile();
		if (! entry) {
			// no more files
			break;
		}
		for (uint8_t i=0; i<numTabs; i++) {
			_HardSerial.print('\t');
		}
		_HardSerial.print(entry.name());
		if (entry.isDirectory()) {
			_HardSerial.println("/");
			printDirectory(entry, numTabs+1);
		} else {
			// files have sizes, directories do not
			_HardSerial.print("\t\t");
			total+=entry.size();
			_HardSerial.println(entry.size(), DEC);
		}
		entry.close();
	}
	return total;
}


void SDCardManager::saveWPSSensorRecord(WPSSensorRecord anWPSSensorRecord){
	if(!cardOk)return ;
	char fileName[30];
	snprintf(fileName, sizeof fileName, "/%s/%s", WPSSensorDataDirName, unstraferedFileName);
	File untransferredFile = SD.open(fileName, FILE_WRITE);
	if (untransferredFile) {
		// Write to file
		lcdDisplay.setRGB(0,0,255);
		float batteryVoltage = anWPSSensorRecord.batteryVoltage;
		float current = anWPSSensorRecord.current;
		int sc = anWPSSensorRecord.stateOfCharge;
		untransferredFile.print(anWPSSensorRecord.lastWPSRecordSeconds);
		untransferredFile.print("#");
		untransferredFile.print(batteryVoltage);
		untransferredFile.print("#");
		untransferredFile.print(current);
		untransferredFile.print("#");
		//
		// calculate the energy used in mAhr
		//
		//		float energy = wpsPulseFrequencySeconds*current/3600;
		//		dailyBatteryOutEnergy+=energy;
		//		untransferredFile.print(energy);
		//		untransferredFile.print("#");
		//		hourlyBatteryOutEnergy+=energy;

		untransferredFile.print(anWPSSensorRecord.hourlyBatteryOutEnergy);
		untransferredFile.print("#");

		untransferredFile.print(anWPSSensorRecord.dailyBatteryOutEnergy);
		untransferredFile.print("#");

		untransferredFile.print(anWPSSensorRecord.hourlyPoweredDownInLoopSeconds);
		untransferredFile.print("#");

		untransferredFile.print(anWPSSensorRecord.dailyPoweredDownInLoopSeconds);
		untransferredFile.print("#");

		if(anWPSSensorRecord.pauseDuringWPS){
			untransferredFile.print(1);
		}else{
			untransferredFile.print(0);
		}
		untransferredFile.print("#");

		untransferredFile.print(sc);
		untransferredFile.print("#");
		if(anWPSSensorRecord.operatingStatus){
			untransferredFile.print(1);
		}else{
			untransferredFile.print(0);
		}



		untransferredFile.print("#");
		untransferredFile.println(anWPSSensorRecord.totalDiskUse/1024);

		untransferredFile.close(); // close the file
		lcdDisplay.setCursor(0,0);
		lcdDisplay.print("saved wps record");
		lcdDisplay.setCursor(0,1);
		lcdDisplay.print("to SD card ");


	} else {
		lcdDisplay.setRGB(255,255,0);
		lcdDisplay.setCursor(0,0);
		lcdDisplay.print("error opening");
		lcdDisplay.setCursor(0,1);
		lcdDisplay.print(fileName);
		delay(2000);
	}
}

