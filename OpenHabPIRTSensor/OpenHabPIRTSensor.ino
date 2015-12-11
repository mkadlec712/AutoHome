/*
Author: Michal Kadlec

Based on Eric Tsai UberHomeAutomation project https://github.com/tsaitsai/OpenHab-RFM69
Using Arduino Tasker Library by Petr Stehlík https://github.com/joysfera/arduino-tasker
Using RFM69 Library by  Felix Rusu (LowPowerLab) https://github.com/LowPowerLab/RFM69

License: CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
Date: 9-1-2014
File: OpenHABPIRTSensor.ino
This sketch is for a wired Arduino w/ RFM69 wireless transceiver
Sends sensor data (PIR presence) back
to gateway.  See OpenHAB configuration file.
1) Update encryption string "ENCRYPTKEY"
*/


/* sensor
node = 12
device ID
4 = 1242 = human motion present or not
*/

#include <Tasker.h>


//RFM69  --------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <RFM69.h>
// #include <RFM69registers.h>
#define TASKER_MAX_TASKS 4
#define NODEID        12    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "xxxxxxxxxxxxxxxx" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define LED           13  // Moteinos have LEDs on D9
#define SERIAL_BAUD   115200  //must be 9600 for GPS, use whatever if no GPS
#define DEBUG


//struct for wireless data transmission
typedef struct {    
  int       nodeID;     //node ID (1xx, 2xx, 3xx);  1xx = basement, 2xx = main floor, 3xx = outside
  int       deviceID;   //sensor ID (2, 3, 4, 5)
  unsigned long   var1_usl;     //uptime in ms
  float     var2_float;     //sensor data?
  float     var3_float;   //battery condition?
} Payload;
Payload theData;

char buff[20];
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;

Tasker tasker;

//end RFM69 ------------------------------------------

// PIR sensor ================================================
int PirInput = 5;
int PIR_status = 0;
int PIR_reading = 0;
int PIR_reading_previous = 0;

// timings
unsigned long pir_time;
//unsigned long pir_time_send;

void Read_PIR(int /* unused */)
{
  //device #4
  //PIR
  
  //1 mean presence detected?
  PIR_reading = digitalRead(PirInput);
  #ifdef DEBUG
  if (PIR_reading == 1)
  Serial.println("PIR = 1");
  else
  Serial.println("PIR = 0");
  //send PIR sensor value only if presence is detected and the last time
  //presence was detected is over x miniutes ago.  Avoid excessive RFM sends
  #endif 
  if ((PIR_reading == 1) && ( ((millis() - pir_time)>60000)||( (millis() - pir_time)< 0)) ) //meaning there was sound
  {
    pir_time = millis();  //update gas_time_send with when sensor value last transmitted
    theData.deviceID = 4;
    theData.var1_usl = millis();
    theData.var2_float = 1111;
    theData.var3_float = 1112;    //null value;
    radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData));
                Serial.println("PIR detected RFM");

    }  
}

void setup() 
{
  Serial.begin(SERIAL_BAUD);          //  setup serial

  //RFM69-------------------------------------------
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  theData.nodeID = NODEID;  //this node id should be the same for all devices in this node
  //end RFM--------------------------------------------

//PIR sensor
  pinMode(PirInput, INPUT);

tasker.setTimeout(Read_PIR, 2000);

tasker.run(); // never returns
}

void loop() {
  // put your main code here, to run repeatedly:

}
