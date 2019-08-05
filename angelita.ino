
#include "Arduino.h"
//#include <oled.h>
#include <OLED.h>

#include <DiscreteRecord.h>
//#include <SDCardManager.h>
#include <avr/wdt.h>
#include <PowerManager.h>
#include <NoDataStorageManager.h>
#include <MemoryFree.h>
#include <static_str.h>

/**
 * teleonome speciifc libraries
 */
#include <Adafruit_NeoPixel.h>

/**
 * teleonome speciifc variables
 */
OLED oled;

#define LED_PIN 23
#define LED_COUNT 2
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
boolean showColor = false;
//DEFINE_PSTR(LIFE_CYCLE_EVENT_SETUP_COMPLETED,"Setup Method Completed");
//==F(" const char *LIFE_CYCLE_EVENT_SETUP_COMPLETED = "Setup Method Completed";
const int LIFE_CYCLE_EVENT_COMMA_VALUE = 1;
//P_STR("did");
//==F(" ==F("_str<64> s(GET_PSTR(LIFE_CYCLE_EVENT_SETUP_COMPLETED));
DataStorageManagerInitParams dataStorageManagerInitParams;
TimeManager timeManager(Serial);
SecretManager secretManager(timeManager);
NoDataStorageManager noDataStorageManager(dataStorageManagerInitParams, timeManager, Serial, oled);
//SDCardManager sdCardManager( dataStorageManagerInitParams, timeManager, Serial, oled );
PowerManager aPowerManager(oled, secretManager, noDataStorageManager, timeManager, Serial);

/**
 * Teleonome Specific Functions
 *  Generated by the Spermatogenesis process
 */

void clearLEDs()
{
 //==F("_str<32> s2(F("did"));
 for (int i = 0; i < LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
  }
}

struct DiscreteRecord createDiscreteRecord(float la, float lo, int pan, int tag, uint8_t ss)
{
  DiscreteRecord discrete;
  discrete.lon = lo;
  discrete.lat = la;
  discrete.timestamp = timeManager.getCurrentTimeInSeconds();
  discrete.panID = pan;
  discrete.tagId = tag;
  discrete.signalStrength = ss;
  return discrete;
}
/**
 * End of Teleonome Specific Functions
 */

/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
ISR(WDT_vect)
{
  //lcd.setCursor(0, 1);
  //lcd.print("Waking Up") ;
  //wdt_reset();

  // uncomment
  //  aPowerManager.toggleWDT();

  //  if(f_wdt == 0)
  //  {
  //    f_wdt=1;
  //  }
  //  else
  //  {
  //    //_HardSerial.println("WDT Overrun!!!");
  //  }
}

void setup()
{
  //
  // Start the Serial Ports
  //
  Serial.begin(9600);

  //
  // if Hermes is present
  //
  //Serial1.begin(9600);

  //
  //the lcd screen
  //
  oled.begin();
  //  //
  //  // Start The Managers
  //  //
  oled.setCursor(0, 0);
  oled.clear();
  oled.print(F("Init Time Manager"));
  oled.setCursor(0, 1);
  oled.print(F("1 of 3"));
  timeManager.start();
  //
  oled.clear();
  oled.print(F("Init SDCard Manager"));
  oled.setCursor(0, 1);
  oled.print(F("2 of 3"));
  //  sdCardManager.start();
  //  //
  oled.clear();
  oled.print(F("Init Power Manager"));
  oled.setCursor(0, 1);
  oled.print(F("3 of 3"));
  aPowerManager.start();
  //
  long totalDiskUse = 0; //sdCardManager.getDiskUsage()/1024;

  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("Init Finished"));
  oled.setCursor(0, 1);

  oled.setCursor(0, 1);
  oled.print(F("SD use "));
  oled.print(totalDiskUse);
  oled.print(F("Kb"));

  //
  // end of initializing lcd
  //
  delay(1000);
  long now = timeManager.getCurrentTimeInSeconds();

  //  sdCardManager.storeLifeCycleEvent(now, LIFE_CYCLE_EVENT_SETUP_COMPLETED, LIFE_CYCLE_EVENT_COMMA_VALUE);

  leds.begin(); // Call this to start up the LED strip.
  for (int i = 0; i < 4; i++)
  {
    leds.setPixelColor(0, 255, 0, 255);
    leds.setPixelColor(1, 0, 255, 0);
    leds.show();

    delay(1250);
  }
}

void loop()
{

  wdt_reset();

  if (showColor)
  {
    leds.setPixelColor(0, 255, 255, 0);
    leds.setPixelColor(1, 0, 255, 0);
    delay(2000);
  }
  else
  {
    leds.setPixelColor(0, 255, 0, 0);
    leds.setPixelColor(1, 0, 255, 255);
    delay(2000);
  }
  leds.show();
  showColor = !showColor;
  //
  // Generate the SensorData String

  //
  // now define the state its in
  //
  // aPowerManager.defineState();
  float batteryVoltage = 14.0; //getBatteryVoltage();
  byte internalBatteryStateOfCharge = GeneralFunctions::getStateOfCharge(batteryVoltage);

  oled.clear();
  oled.setCursor(0, 0);

  oled.print(freeMemory());
  oled.print("b ");

  oled.print(batteryVoltage);
  oled.print("V ");
  oled.print(internalBatteryStateOfCharge);
  oled.print("%");
  oled.setCursor(0, 1);
  RTCInfoRecord r = timeManager.getCurrentDateTime();
  oled.print(r.hour);
  oled.print(":");
  if (r.minute < 10)
    oled.print("0");
  oled.print(r.minute);
  oled.print(":");
  if (r.second < 10)
    oled.print("0");
  oled.print(r.second);
  //
  //
  // the serial ports
  // the code below depends on the structure of the teleonome. in the biggest case
  // you have three, Serial which corresponds to the hypothalamus connection which is always present
  // then Serial2 which represents the XBee and Serial3 which represents RS485
  // //
  if (Serial.available() != 0)
  {

    oled.clear();
    oled.setCursor(0, 0);

    oled.print(F("in serial"));

    String command = Serial.readString();
    //
    // end of teleonome specific sensors
    //

    boolean commandProcessed = true; // aPowerManager.processDefaultCommands( command);

    /*
     * teleonome specific sensors
     */

    /*
     * end f teleonome specific sensors
     */
    if (!commandProcessed)
    {
      //
      // add device specific

      if (command.startsWith(F("GetSensorData")))
      {

        oled.clear();
        oled.setCursor(0, 0);

        oled.print(F("Sending Sensor Data"));
        Serial.print(F("Sending Sensor Data"));
        aPowerManager.printBaseSensorStringToSerialPort();

        //
        // now add the teleonome specific sensors
        //

        Serial.flush();
        oled.print(F("Sent Sensor Data"));
      }
      else if (command.startsWith(F("ReadDiscreteRecords")))
      {
        boolean fileOk = true; //sdCardManager.openDiscreteRecordFile();
        boolean keepGoing = true;
        DiscreteRecord discreteRecord;

        uint16_t index = 0;
        if (fileOk)
        {
          while (keepGoing)
          {
            keepGoing = false; //sdCardManager.readDiscreteRecord(index, discreteRecord);
            index++;
            if (keepGoing)
            {
              //
              // if we are here is because we do have data
              // so extract it
              //
              // generated code
              //

              Serial.print(discreteRecord.timestamp);
              Serial.print("#");
              Serial.print(discreteRecord.lat);
              Serial.print("#");
              Serial.print(discreteRecord.lon);
              Serial.print("#");
              Serial.print(discreteRecord.panID);
              Serial.print("#");
              Serial.print(discreteRecord.tagId);
              Serial.print("#");
              Serial.print(discreteRecord.signalStrength);
              Serial.println("#");
            }
            Serial.println(F("Ok-RD"));
            Serial.flush();
          }
          //sdCardManager.closeDiscreteRecordFile();
        }
      }
      else
      {
        //
        // call read to flush the incoming
        //
        Serial.read();
        Serial.print(F("Failure-Bad Command "));
       	Serial.println(command);
        Serial.flush();
      }
    }
  }

  // this is the end of the loop, to calculate the energy spent on this loop
  // take the time substract the time at the beginning of the loop (the now variable defined above)
  // and also substract the seconds spent in powerdownMode
  // finally add the poweredDownInLoopSeconds to the daily total

  //  aPowerManager.endOfLoopProcessing();
}