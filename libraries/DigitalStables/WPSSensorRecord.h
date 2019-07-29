 #ifndef  WPSSENSORRECORD_H
 #define WPSSENSORRECORD_H
#include "Arduino.h"
//
// float 4 bytes
// long 4 bytes
//
struct WPSSensorRecord{

		float current;
		uint8_t stateOfCharge;
		long lastWPSRecordSeconds;
		float  batteryVoltage;
		float hourlyBatteryOutEnergy;
		float dailyBatteryOutEnergy;
		float hourlyPoweredDownInLoopSeconds;
		float dailyPoweredDownInLoopSeconds;
		boolean pauseDuringWPS;
		String operatingStatus;
		long totalDiskUse;
};
#endif
