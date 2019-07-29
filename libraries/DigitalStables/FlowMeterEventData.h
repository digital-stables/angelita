/*
 * FlowMeterEventData.h
 *
 *  Created on: 6 May 2019
 *      Author: arifainchtein
 */

#ifndef LIBRARIES_DIGITALSTABLES_FLOWMETEREVENTDATA_H_
#define LIBRARIES_DIGITALSTABLES_FLOWMETEREVENTDATA_H_

#include "Arduino.h"
struct FlowMeterSample{
	long sampleTime;
	double flow;
};
typedef struct FlowMeterEventData{
	long startTime;
	long endTime;
	//
	// the eventGroupStartTime is a form of an id
	// it represents the  start time of the event of
	// flow meter 0, ie the input flow meter
	// and is a way to associate all the events that
	// happened from the moment the input flowmeter
	// starts detecting flow to the moment flowmeter stops
	// detecting flow.
	// obviously, for the event that represents flowmeter 0
	// the eventGroupTime will be equal to the startTime
	//
	long eventGroupStartTime;
	double averageflow;
	uint8_t numberOfSamples;
	double totalVolume;
	uint8_t flowMeterId;
	uint16_t sampleFrequencySeconds;
	FlowMeterSample samples[10];

	String toTokenizedString(char separator){
		String toReturn;
		toReturn.reserve(200);

		toReturn.concat(startTime) ;
		toReturn.concat("#") ;
		toReturn.concat(endTime) ;
		toReturn.concat("#") ;
		toReturn.concat(eventGroupStartTime) ;
		toReturn.concat("#") ;
		char aString[15];
		dtostrf(averageflow,5, 2, aString);
		toReturn.concat(aString) ;
		toReturn.concat("#") ;
		toReturn.concat(flowMeterId) ;
		toReturn.concat("#") ;

		char aString2[15];
		dtostrf(totalVolume,5, 2, aString2);
		toReturn.concat(aString2) ;
		toReturn.concat("#") ;

		toReturn.concat(sampleFrequencySeconds) ;
		toReturn.concat("#") ;
		toReturn.concat(numberOfSamples) ;
		toReturn.concat("#") ;
		FlowMeterSample sample;

		char aString3[15];
		for(int i=0;i<numberOfSamples;i++){
			sample = samples[i];
			toReturn.concat(sample.sampleTime) ;
			toReturn.concat("#") ;
			dtostrf(sample.flow,5, 2, aString3);
			toReturn.concat(aString3) ;
			toReturn.concat("#") ;
		}
		return toReturn;

	}
};



typedef union FlowMeterEventDataUnion{
	FlowMeterEventData aFlowMeterEventData;
	byte flowMeterEventDataBytes[sizeof(FlowMeterEventData)];
};



typedef struct FlowMeterEventFaultData{
	//
	// a fault is a tokenized string separated with #
	// token 0 is the word fault
	// token 1 is the type f mutation
	// token 2 is the name of the mutation to be executed
	// token 3 is the commandCode for the command about the fault
	// token 4 is the payload for the command about the fault

};

//typedef union FlowMeterEventFaultUnion{
//	FlowMeterEventFaultData aFlowMeterEventFaultData;
//	byte flowMeterEventFaultBytes[sizeof(FlowMeterEventFaultData)];
//};



#endif /* LIBRARIES_DIGITALSTABLES_FLOWMETEREVENTDATA_H_ */
