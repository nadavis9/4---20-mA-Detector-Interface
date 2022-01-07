//Arduino Bluetooth Imported directories
//#include <Arduino.h>
//#include <SPI.h>
//#include "Adafruit_BLE.h"
//#include "Adafruit_BluefruitLE_SPI.h"
//#include "Adafruit_BluefruitLE_UART.h"
//#include "BluefruitConfig.h"
//
//#define FACTORYRESET_ENABLE         1
//#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
//#define MODE_LED_BEHAVIOUR          "MODE"
  #define RESETLINE     4
//#define BLUEFRUIT_HWSERIAL_NAME      Serial1


#define DisplaySerial Serial

#include "DiabloGraphicsMainConst.h"

#include "Diablo_Serial_4DLib.h"
#include "Diablo_LedDigitsDisplay.h"
#include "Diablo_Const4D.h"

Diablo_Serial_4DLib Display(&DisplaySerial);

//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
///Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

// routine to handle Serial errors
void mycallback(int ErrCode, unsigned char Errorbyte)
{
#ifdef LOG_MESSAGES
  const char *Error4DText[] = {"OK\0", "Timeout\0", "NAK\0", "Length\0", "Invalid\0"} ;
  LOG_MESSAGES.print(F("Serial 4D Library reports error ")) ;
  LOG_MESSAGES.print(Error4DText[ErrCode]) ;
  if (ErrCode == Err4D_NAK)
  {
    LOG_MESSAGES.print(F(" returned data= ")) ;
    LOG_MESSAGES.println(Errorbyte) ;
  }
  else
    LOG_MESSAGES.println(F("")) ;
  while (1) ; // you can return here, or you can loop
#else
  // Pin 13 has an LED connected on most Arduino boards. Just give it a name
#define led 13
  while (1)
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);                // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(200);                // wait for a second
  }
#endif
}
// end of routine to handle Serial errors
//DEFINE VARIABLES HERE
word hndl ;
int state,SystemStatus = 0, PumpPower = 1, PumpPowerStatus = 1, sleep_timer = 0;
int form = 1,TempUnitStatus = 1,TempLogStatus = 1,LevelLogStatus = 1,PresLogStatus = 1,FlowLogStatus = 1;
float TempData = 0,LevelData = 0,FlowDataGraphic = 0,LevelTarget = 1, LevelRange = 1,PresData = 0, FlowData = 0, ValvePower = 0;
int x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6;
int i2=1,i22=1,i222=1,i3=1,i33=1,i5=1,i55=1,i6=1,i66=1;
int f1=0,b2=0,f2=0,b3=0,f3=0,b4=0,f4=0,b5=0,f5=0,b6=0;
//Arduino Control---------------------------------------------
float runningFlow = 0;
float flowpersec;
//int setLevel = 1.5;
int level_TX = A0, pressure_TX = A1, flow_TX = A2, temp_TX = A3;
int level_sample, pressure_sample, flow_sample, temp_sample;
int button1;
int On = 0, Off = 0;
boolean toggle4 = LOW;
float temp_voltage, level_voltage, flow_voltage, pressure_voltage, temp, level, flow, pressure, voltage;
float ultraBuffer[20], ultraSample, filteredSample;
//---------------------------------------------------------------

void setup()
{
// Ucomment to use the Serial link to the PC for debugging
  pinMode(40, OUTPUT);
// Note! The next statement will stop the sketch from running until the serial monitor is started
//       If it is not present the monitor will be missing the initial writes
    //while (!Serial) ;             // wait for serial to be established

  pinMode(RESETLINE, OUTPUT);       // Display reset pin
  digitalWrite(RESETLINE, 1);       // Reset Display, using shield
  delay(100);                       // wait for it to be recognised
  digitalWrite(RESETLINE, 0);       // Release Display Reset, using shield
  delay(3000) ;                     // give display time to startup

  // now start display as Serial lines should have 'stabilised'

    DisplaySerial.begin(115200) ;     // Hardware serial to Display, same as SPE on display is set to (initially 9600)
    Display.TimeLimit4D = 5000 ;      // 5 second timeout on all commands
    Display.Callback4D = mycallback ;

  //Control System Hardware Timer Configuration
   //Set Timer4 Periodic Interrupt for 1Hz
   TCCR4A = 0;
   TCCR4B = 0;
   TCNT4  = 0; // Initialize Count Register
   OCR4A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
   TCCR4B |= (1 << WGM12); // Set for CTC Mode
   TCCR4B |= (1 << CS12) | (1 << CS10); // Set Prescaler to 1024
   TIMSK4 |= (1 << OCIE4A); // Set output compare interrupt flag to jump into ISR

//Initialize Screen
  Display.gfx_ScreenMode(LANDSCAPE_R) ; // change manually if orientation change
  Display.putstr("Mounting...\n");
  if (!(Display.file_Mount()))
  {
    while(!(Display.file_Mount()))
    {
      Display.putstr("Drive not mounted...");
      delay(200);
      Display.gfx_Cls();
      delay(200);
    }
  }
  //hFontn = Display.file_LoadImageControl("NoName2.dnn", "NoName2.gnn", 1); // Open handle to access uSD fonts, uncomment if required and change nn to font number
  //hstrings = Display.file_Open("DiabloGr.txf", 'r') ;// Open handle to access uSD strings, uncomment if required
    Display.gfx_Cls();
    hndl = Display.file_LoadImageControl("DiabloGr.dat", "DiabloGr.gci", 1);

  //SETUP CODE FOR INITIALIZATION
    Display.img_ClearAttributes(hndl, ALL, I_TOUCH_DISABLE); // 4Dbutton1 set to enable touch, only need to do this once
    Display.touch_Set(TOUCH_ENABLE);

    //Initialize Bluetooth
//    Serial1.begin(115200) ;        // serial to USB port
//    ble.echo(false);
//    ble.info();
//    ble.verbose(false);
//    // Wait for connection - Currently Bipass
//    //while (! ble.isConnected()) {
//        //delay(500);
//    //}
//    // LED Activity command is only supported from 0.6.6
//    if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
//    {
//      // Change Mode LED Activity
//      ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
//    }
//
//    // Set module to DATA mode
//    ble.setMode(BLUEFRUIT_MODE_DATA);
//    ble.reset();

} // end Setup **do not alter, remove or duplicate this line**

  //Timer4 Interrupt Service Routine - Jump into this routine once per second (1Hz)
  ISR(TIMER4_COMPA_vect){

  flowpersec = flow / 60; // Convert pump flow rate to liters per second
  runningFlow = runningFlow + flowpersec; // update tank volume counter
  }

//-------------------------------------------------------------------------------------------------------------------
//Loop to check for screen interaction
void loop()
{
  //Arduino Sensor Control Here
      // Pump on / off for control testing
       On = digitalRead(35);
       Off = digitalRead(37);

        if (On == LOW){
          digitalWrite(47, HIGH);
        }
        else if (Off == LOW){
         digitalWrite(47, LOW);
        }
        digitalWrite(40, LOW);
    //CONTROL SYSTEM
    if(PumpPower == 0){
      digitalWrite(40, HIGH);
    }
    else if(PumpPower == 1){
      digitalWrite(40, LOW);
    }

    if (runningFlow >= LevelTarget) { // check if target level has been reached
      digitalWrite(47, LOW); // Turn pump off if target has been reached
      runningFlow = 0;
      PumpPower = 1;
    }
      //Output to Bluetooth File
      //ble.write("Temperature(F), Water Height(Liters), Pressure(PSI), FlowRate(Liters/Min) /n");

      // Sample temperature sensor voltage
        temp_sample = analogRead(temp_TX);
        temp_voltage = temp_sample * (5.0/1023.0);
      // Process temperature sensor sample
        temp = gimmeTemp(temp_voltage);                     //Fahrenheit 50-100
        TempData = round(temp*10);
        //ble.print("99");

      // Sample level sensor voltage
        level_sample = analogRead(level_TX);
        level_voltage = (level_sample * (5.0/1023.0));
      // Process level sensor sample
        level = gimmeLevel(level_voltage);
        LevelData = level;
        LevelData = floor(((level*(pow(5.375,2)*3.14159)*0.016)));                     //Mapping Level Data to Screen (WITH Inches3 to Liters)
        //ble.write("1.4");

      // Sample pressure sensor voltage
        pressure_sample = analogRead(pressure_TX);
        pressure_voltage = pressure_sample * (5.0/1023.0);
      // Process pressure sensor sample
        pressure = gimmePressure(pressure_voltage);           //0-0.5
        PresData = round((pressure)*100);                               //Mapping Pressure Data to Screen (PSI)
        //ble.write("0.33");

      // Sample flow sensor voltage
        flow_sample = analogRead(flow_TX);
        flow_voltage = flow_sample * (5.0/1023.0);
      // Process flow sensor sample
        flow = gimmeFlow(flow_voltage);                       //0-3
        if(flow < 0.5)
          {flow = 0;}
        FlowData = (flow*100);
        FlowDataGraphic = round(flow*100);                                      //Mapping Flow Rate Data to Screen (Liters/Minute)
        //ble.write("1.3");
        //delay(200);

  //---------------------------------------------------------------
  //SETUP HERE
  if(form == 1)      //Form 1 - Statup Screen
  {
    Display.img_Show(hndl,iStatictext1) ;  // Statictext1
    Display.img_Show(hndl,iStatictext2) ;  // Statictext2
    Display.img_Show(hndl,iStatictext3) ;  // Statictext3
    Display.img_Show(hndl,iStatictext4) ;  // Statictext4
    Display.img_Show(hndl,iStatictext5) ;  // Statictext5
    Display.img_SetAttributes(hndl, i4Dbutton1, I_ENABLED);
    Display.img_Enable(hndl,i4Dbutton1);
    Display.img_SetWord(hndl, i4Dbutton1, IMAGE_INDEX, 0); // 4Dbutton1 where state is 0 for up and 1 for down
    Display.img_Show(hndl,i4Dbutton1) ;  // 4Dbutton1
  }
  //---------------------------------------------------------------
  if(form == 2)       //Form 2 - Temperature Sensor
  {
    //Display static text
      Display.img_Show(hndl,iStatictext6) ;
      Display.img_Show(hndl,iStatictext7) ;
      Display.img_Show(hndl,iStatictext9) ;
    //Poll for OPERATIONAL STATUS
      if((i2 % 2) == 0){         //Unoperational
          Display.img_Enable(hndl, iStatictext14);
          Display.img_Show(hndl,iStatictext14) ;
      }
      if((i2 % 2) == 1){         //Operational
          Display.img_Enable(hndl, iStatictext8);
          Display.img_Show(hndl,iStatictext8) ;
      }
      Display.img_Show(hndl,iStatictext10) ;
    //Poll for Temp Units Text
      //if((i22 % 2) == 1){       //Display Celcius
          //Display.img_Enable(hndl,iStatictext11);
          //Display.img_Show(hndl,iStatictext11) ;
      //}
      //if((i22 % 2) == 0){        //Display Fahrenheit
          Display.img_Enable(hndl,iStatictext15);
          Display.img_Show(hndl,iStatictext15) ;
          //Alternate unit value
      //}
      Display.img_Show(hndl,iStatictext12) ;
    //Poll for Data Logging Text
      if((i222 % 2) == 0){              //Log is Off
          Display.img_Enable(hndl,iStatictext16);
          Display.img_Show(hndl,iStatictext16) ;
      }
      if((i222 % 2) == 1){              //Log is On
          Display.img_Enable(hndl, iStatictext13);
          Display.img_Show(hndl, iStatictext13);
      }
      Display.img_Show(hndl,iStatictext17) ;
    //Init and show the buttons
      Display.img_SetAttributes(hndl, i4Dbutton2, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton2);
      Display.img_SetWord(hndl, i4Dbutton2, IMAGE_INDEX, 0); // where state is 0 for up and 1 for down
      Display.img_Show(hndl,i4Dbutton2) ;
      Display.img_SetAttributes(hndl, i4Dbutton3, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton3);
      Display.img_SetWord(hndl, i4Dbutton3, IMAGE_INDEX, 0); // where state is 0 for up and 1 for down
      Display.img_Show(hndl,i4Dbutton3) ;
      //Display.img_SetAttributes(hndl, i4Dbutton4, I_ENABLED);
      //Display.img_Enable(hndl,i4Dbutton4);
      //Display.img_SetWord(hndl, i4Dbutton4, IMAGE_INDEX, TempUnitStatus); // where state is 0 for up and 1 for down
      //Display.img_Show(hndl,i4Dbutton4) ;
      Display.img_SetAttributes(hndl, i4Dbutton5, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton5);
      Display.img_SetWord(hndl, i4Dbutton5, IMAGE_INDEX, TempLogStatus); // where state is 0 for up and 1 for down
      Display.img_Show(hndl,i4Dbutton5) ;
    //Init and show DATA digits
      LedDigitsDisplay(Display, hndl, TempData, iLeddigits1+1, 464, 3, 1, 40, 0) ;  // Leddigits1
      Display.gfx_CircleFilled(541, 275, 2, LIME) ;  // Circle1
    //Init and show STATUS led
      Display.img_SetWord(hndl, iUserled1, IMAGE_INDEX, SystemStatus) ;      // where state is 0 (Off) or 1 (On)
      Display.img_Show(hndl,iUserled1) ;
    //Init and show Thermometer, value set with TempData
      Display.img_SetWord(hndl, iThermometer1, IMAGE_INDEX, round(TempData/10)) ; // where frame is 0 to 60 (for a displayed -1 to -1)
      Display.img_Show(hndl,iThermometer1) ;                         // Thermometer1
  }
  //---------------------------------------------------------------
  if(form == 3)                //Form 3 - Water Level Sensor
  {
    //Display Static Text
      Display.img_Show(hndl,iStatictext18) ;
      Display.img_Show(hndl,iStatictext19) ;
      Display.img_Show(hndl,iStatictext20) ;
    //Poll for OPERATIONAL Status
      if((i3 % 2) == 0){         //Unoperational
          Display.img_Enable(hndl,iStatictext26) ;
          Display.img_Show(hndl,iStatictext26) ;
      }
      if((i3 % 2) == 1){         //Operational
          Display.img_Enable(hndl,iStatictext21) ;
          Display.img_Show(hndl,iStatictext21) ;
      }
      Display.img_Show(hndl,iStatictext22) ;
      Display.img_Show(hndl,iStatictext23) ;
    //Poll for Data Logging Text
      if((i33 % 2) == 0){          //Disabled
          Display.img_Enable(hndl,iStatictext27) ;
          Display.img_Show(hndl,iStatictext27) ;
      }
      if((i33 % 2) == 1){          //Enabled
          Display.img_Enable(hndl,iStatictext24) ;
          Display.img_Show(hndl,iStatictext24) ;
      }
      Display.img_Show(hndl,iStatictext25) ;
    //Init and show the buttons
      Display.img_SetAttributes(hndl, i4Dbutton6, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton6);
      Display.img_SetWord(hndl, i4Dbutton6, IMAGE_INDEX, LevelLogStatus); // where state1 is 0 for up and 1 for down
      Display.img_Show(hndl, i4Dbutton6);
      Display.img_SetAttributes(hndl, i4Dbutton7, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton7);
      Display.img_SetWord(hndl, i4Dbutton7, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton7);
      Display.img_SetAttributes(hndl, i4Dbutton8, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton8);
      Display.img_SetWord(hndl, i4Dbutton8, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton8);
    //Water Level Buttons
      Display.img_SetAttributes(hndl, i4Dbutton9, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton9);
      Display.img_SetWord(hndl, i4Dbutton9, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton9);
      Display.img_SetAttributes(hndl, i4Dbutton10, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton10);
      Display.img_SetWord(hndl, i4Dbutton10, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton10);
      Display.img_SetAttributes(hndl, i4Dbutton19, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton19);
      //Display.img_SetWord(hndl, i4Dbutton19, IMAGE_INDEX, 1);
      //Display.img_Show(hndl,i4Dbutton19);

      //Pump Power Control
      if(PumpPower == 0){           //POWER ON
        Display.img_SetWord(hndl, i4Dbutton19, IMAGE_INDEX, PumpPower); // 4Dbutton19 where state is 0 for up and 1 for down
        Display.img_Show(hndl,i4Dbutton19) ;  // 4Dbutton19
        LevelRange = (LevelTarget - round(LevelData));
      }
      if(PumpPower == 1){           //POWER OFF
        Display.img_SetWord(hndl, i4Dbutton19, IMAGE_INDEX, PumpPower); // 4Dbutton19 where state is 0 for up and 1 for down(OFF)
        Display.img_Show(hndl,i4Dbutton19) ;  // 4Dbutton19
        LevelRange = 0;
      }
    //Init and show DATA digits
      LedDigitsDisplay(Display, hndl, round(LevelData), iLeddigits2+1, 484, 2, 1, 35, 0) ;  // Leddigits2
      //Display.gfx_CircleFilled(516, 260, 2, LIME) ;  // Circle2
      LedDigitsDisplay(Display, hndl, LevelTarget, iLeddigits3+1, 620, 2, 1, 37, 0) ;  // Leddigits3
      //Display.gfx_CircleFilled(653, 385, 2, LIME) ;  // Circle3
    //Init and show STATUS led
      Display.img_SetWord(hndl, iUserled2, IMAGE_INDEX, SystemStatus) ;      // where state is 0 (Off) or 1 (On)
      Display.img_Show(hndl,iUserled2) ;
    //Init and show Water Tank
      Display.img_SetWord(hndl, iTank1, IMAGE_INDEX, round(LevelData)) ; // where frame is 0 to 12 (for a displayed 0 to 12)
      Display.img_Show(hndl,iTank1) ;
  }
  //---------------------------------------------------------------
  if(form == 4)                     //Form 4 - Full System Readings
  {
    //Display Static text
      Display.img_Show(hndl,iStatictext28) ;
      Display.img_Show(hndl,iStatictext29) ;
    //Poll for Change in Temperature Units
      //if((i22 % 2) == 0){                     //Fahrenheit
          //Display.gfx_OutlineColour(BLACK) ;
          //Display.gfx_LinePattern(LPFINE) ;
          //Display.gfx_RectangleFilled(268, 202, 392, 231, BLACK) ;  // Rectangle10
          //Display.gfx_OutlineColour(BLACK) ;
          //Display.gfx_LinePattern(LPSOLID) ;
          Display.img_Enable(hndl, iStatictext38) ;
          Display.img_Show(hndl,iStatictext38) ;
      //}
      //if((i22 % 2) == 1) {                    //Celcius
         // Display.gfx_OutlineColour(BLACK) ;
          //Display.gfx_LinePattern(LPFINE) ;
          //Display.gfx_RectangleFilled(268, 202, 392, 231, BLACK) ;  // Rectangle10
          //Display.gfx_OutlineColour(BLACK) ;
          //Display.gfx_LinePattern(LPSOLID) ;
          //Display.img_Enable(hndl, iStatictext31) ;
          //Display.img_Show(hndl,iStatictext31) ;
      //}
      Display.img_Show(hndl,iStatictext30) ;
      Display.img_Show(hndl,iStatictext32) ;
      Display.img_Show(hndl,iStatictext33) ;
      Display.img_Show(hndl,iStatictext34) ;
      Display.img_Show(hndl,iStatictext35) ;
      Display.img_Show(hndl,iStatictext36) ;
      Display.img_Show(hndl,iStatictext37) ;
    //Init and show buttons
      Display.img_SetAttributes(hndl, i4Dbutton11, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton11);
      Display.img_SetWord(hndl, i4Dbutton11, IMAGE_INDEX, 0); // where state1 is 0 for up and 1 for down
      Display.img_Show(hndl, i4Dbutton11);
      Display.img_SetAttributes(hndl, i4Dbutton12, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton12);
      Display.img_SetWord(hndl, i4Dbutton12, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton12);
    //Init and show DATA digits
      LedDigitsDisplay(Display, hndl, TempData, iLeddigits4+1, 156, 3, 1, 35, 0) ;  // Leddigits4
      Display.gfx_CircleFilled(223, 252, 2, LIME) ;  // Circle4
      LedDigitsDisplay(Display, hndl, LevelData, iLeddigits5+1, 516, 2, 1, 38, 0) ;  // Leddigits5
      //Display.gfx_CircleFilled(551, 252, 2, LIME) ;  // Circle5
      LedDigitsDisplay(Display, hndl, PresData, iLeddigits6+1, 156, 3, 1, 35, 0) ;  // Leddigits6
      Display.gfx_CircleFilled(189, 396, 2, LIME) ;  // Circle6
      LedDigitsDisplay(Display, hndl, FlowData, iLeddigits7+1, 496, 3, 1, 35, 0) ;  // Leddigits7
      Display.gfx_CircleFilled(528, 395, 2, LIME) ;  // Circle7
    //Init and show STATUS led
      Display.img_SetWord(hndl, iUserled3, IMAGE_INDEX, SystemStatus) ;      // where state is 0 (Off) or 1 (On)
      Display.img_Show(hndl,iUserled3) ;
  }
  //---------------------------------------------------------------
  if(form == 5)               //Form 5 - Pressure Sensor Data
  {
    //Display Static text
      Display.img_Show(hndl,iStatictext39) ;
      Display.img_Show(hndl,iStatictext40) ;
      Display.img_Show(hndl,iStatictext41) ;
    //Poll for OPERATIONAL Status
      if((i5 % 2) == 0){                //Unoperational
          Display.img_Enable(hndl, iStatictext47) ;
          Display.img_Show(hndl,iStatictext47) ;
      }
      if((i5 % 2) == 1){                //Operational
          Display.img_Enable(hndl, iStatictext42) ;
          Display.img_Show(hndl,iStatictext42) ;
      }
      Display.img_Show(hndl,iStatictext43) ;
      Display.img_Show(hndl,iStatictext44) ;
    //Poll for Data Logging Text
      if((i55 % 2) == 0){               //Disabled
          Display.img_Enable(hndl, iStatictext48) ;
          Display.img_Show(hndl,iStatictext48) ;
      }
      if((i55 % 2) == 1){               //Enabled
          Display.img_Enable(hndl, iStatictext45) ;
          Display.img_Show(hndl,iStatictext45) ;
      }
      Display.img_Show(hndl,iStatictext46) ;
    //Init and show buttons
      Display.img_SetAttributes(hndl, i4Dbutton13, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton13);
      Display.img_SetWord(hndl, i4Dbutton13, IMAGE_INDEX, 0); // where state1 is 0 for up and 1 for down
      Display.img_Show(hndl, i4Dbutton13);
      Display.img_SetAttributes(hndl, i4Dbutton14, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton14);
      Display.img_SetWord(hndl, i4Dbutton14, IMAGE_INDEX, 0);
      Display.img_Show(hndl, i4Dbutton14);
      Display.img_SetAttributes(hndl, i4Dbutton15, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton15);
      Display.img_SetWord(hndl, i4Dbutton15, IMAGE_INDEX, PresLogStatus);
      Display.img_Show(hndl, i4Dbutton15);
    //Init and show DATA digits
      LedDigitsDisplay(Display, hndl, PresData, iLeddigits8+1, 532, 3, 1, 40, 0) ;  // Leddigits8
      Display.gfx_CircleFilled(568, 355, 2, LIME) ;  // Circle8
    //Init and show STATUS led
      Display.img_SetWord(hndl, iUserled4, IMAGE_INDEX, SystemStatus) ;      // where state is 0 (Off) or 1 (On)
      Display.img_Show(hndl,iUserled4) ;
    // Meter1 1.0 generated 11/10/2021 10:56:17 AM
      Display.img_SetWord(hndl, iMeter1, IMAGE_INDEX, 14) ; // where frame is 0 to 100 (for a displayed 0 to 100)
      Display.img_Show(hndl,iMeter1) ;
  }
  //---------------------------------------------------------------
  if(form == 6)                           //Form 6 - Flow Rate Sensor Data
  {
      //Display static text
      Display.img_Show(hndl,iStatictext49) ;
      Display.img_Show(hndl,iStatictext50) ;
      Display.img_Show(hndl,iStatictext51) ;
      //Poll for OPERATIONAL status
      if((i6 % 2) == 0){          //Unoperational
          Display.img_Enable(hndl,iStatictext57) ;
          Display.img_Show(hndl,iStatictext57) ;
      }
      if((i6 % 2) == 1){          //Operational
          Display.img_Enable(hndl,iStatictext52) ;
          Display.img_Show(hndl,iStatictext52) ;
      }
      Display.img_Show(hndl,iStatictext53) ;
      Display.img_Show(hndl,iStatictext54) ;
      //Poll for Data Logging Text
      if((i66 % 2) == 0){         //Disabled
          Display.img_Enable(hndl, iStatictext58) ;
          Display.img_Show(hndl,iStatictext58) ;
      }
      if((i66 % 2) == 1){         //Enabled
          Display.img_Enable(hndl, iStatictext55) ;
          Display.img_Show(hndl,iStatictext55) ;
      }
      Display.img_Show(hndl,iStatictext56) ;
      //Init and show buttons
      Display.img_SetAttributes(hndl, i4Dbutton16, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton16);
      Display.img_SetWord(hndl, i4Dbutton16, IMAGE_INDEX, 0); // where state1 is 0 for up and 1 for down
      Display.img_Show(hndl, i4Dbutton16);
      Display.img_SetAttributes(hndl, i4Dbutton17, I_ENABLED);
      Display.img_Enable(hndl,i4Dbutton17);
      Display.img_SetWord(hndl, i4Dbutton17, IMAGE_INDEX, FlowLogStatus);
      Display.img_Show(hndl, i4Dbutton17);
      //Init and show DATA digits
      LedDigitsDisplay(Display, hndl, FlowData, iLeddigits9+1, 540, 3, 1, 35, 0) ;  // Leddigits9
      Display.gfx_CircleFilled(572, 355, 2, LIME) ;  // Circle9
      //Init and show STATUS led
      Display.img_SetWord(hndl, iUserled5, IMAGE_INDEX, SystemStatus) ;      // where state is 0 (Off) or 1 (On)
      Display.img_Show(hndl,iUserled5) ;
      // Meter1 1.0 generated 11/10/2021 10:56:17 AM
      Display.img_SetWord(hndl, iMeter2, IMAGE_INDEX, FlowDataGraphic) ; // where frame is 0 to 100 (for a displayed 0 to 350)
      Display.img_Show(hndl,iMeter2) ;
  }
  //---------------------------------------------------------------
    state = Display.touch_Get(TOUCH_STATUS);
  //-------------------------------------------------------------------------------------------------------------------
  //CHECK FOR INPUT
  if(state == TOUCH_PRESSED)
  {
    if(form == 1)   //Controls for Startup Screen
    {
      //Define x and y for touch
        x1 = Display.touch_Get(TOUCH_GETX);
        y1 = Display.touch_Get(TOUCH_GETY);

      if((x1>744 && x1<790) && (y1>424 && y1<470)){
        Display.img_SetWord(hndl, i4Dbutton1, IMAGE_INDEX, 1);
        Display.img_Show(hndl,i4Dbutton1) ;
        //Clear and change screens
        Display.gfx_Cls();
        Display.img_Disable(hndl, i4Dbutton1);
        Display.img_ClearAttributes(hndl, i4Dbutton1, I_TOUCH_DISABLE);
        f1 = 1;
      }
    }
    //---------------------------------------------------------------
    if(form == 2)    //Controls for Temperature Sensor
    {
       //Define x and y for touch
        x2 = Display.touch_Get(TOUCH_GETX);
        y2 = Display.touch_Get(TOUCH_GETY);
       //Check if SYSTEM STATUS CHANGES
       if(SystemStatus == 1){
            i2 = (i2 + 1);
            if((i2 % 2) == 1){
                Display.img_Disable(hndl, iStatictext8);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(272, 183, 427, 211, BLACK) ;  // Rectangle1
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext14);
                Display.img_Show(hndl, iStatictext14);
            }
            if((i2 % 2) == 0){
                Display.img_Disable(hndl, iStatictext14);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(272, 183, 427, 211, BLACK) ;  // Rectangle1
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext8);
                Display.img_Show(hndl, iStatictext8);
            }
        }
        //Check if Units button was pressed
        if((x2>52 && x2<102) && (y2>232 && y2<282)){
            i22 = (i22 + 1);
            if((i22 % 2) == 0){      //Display Celcius
                TempUnitStatus = 0;
                Display.img_SetWord(hndl, i4Dbutton4, IMAGE_INDEX, TempUnitStatus);
                Display.img_Show(hndl,i4Dbutton2) ;
                Display.img_Disable(hndl, iStatictext15);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(260, 241, 377, 263, BLACK) ;  // Rectangle2
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext11);
                Display.img_Show(hndl, iStatictext11);
            }
            if((i22 % 2) == 1){      //Display Fahrenheit
                TempUnitStatus = 1;
                Display.img_SetWord(hndl, i4Dbutton4, IMAGE_INDEX, TempUnitStatus);
                Display.img_Show(hndl,i4Dbutton2) ;
                Display.img_Disable(hndl, iStatictext11);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(260, 241, 377, 263, BLACK) ;  // Rectangle2
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext15);
                Display.img_Show(hndl, iStatictext15);
            }
        }
        //Check if Data Logging button was pressed
        if((x2>52 && x2<102) && (y2>288 && y2<338)){
            i222 = (i222 + 1);
            if((i222 % 2) == 1){
                TempLogStatus = 1;
                Display.img_SetWord(hndl, i4Dbutton5, IMAGE_INDEX, TempLogStatus);
                Display.img_Show(hndl,i4Dbutton5) ;
                Display.img_Disable(hndl, iStatictext16);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(264, 300, 357, 324, BLACK) ;  // Rectangle3
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext13);
                Display.img_Show(hndl, iStatictext13);
            }
            if((i222 % 2) == 0){
                TempLogStatus = 0;
                Display.img_SetWord(hndl, i4Dbutton5, IMAGE_INDEX, TempLogStatus);
                Display.img_Show(hndl,i4Dbutton5) ;
                Display.img_Disable(hndl, iStatictext13);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(264, 300, 357, 324, BLACK) ;  // Rectangle3
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext16);
                Display.img_Show(hndl, iStatictext16);
            }
        }
        //Check if back button was pressed
        if((x2>12 && x2<70) && (y2>424 && y2<470)){
            Display.img_SetWord(hndl, i4Dbutton2, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton2) ;
            //clear the screen and change to STATUS display
            Display.gfx_Cls();
            //disable touch for buttons on previous screen
            Display.img_Disable(hndl,i4Dbutton2);
            Display.img_Disable(hndl,i4Dbutton3);
            Display.img_Disable(hndl,i4Dbutton4);
            Display.img_Disable(hndl,i4Dbutton5);
            Display.img_ClearAttributes(hndl, i4Dbutton2, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton3, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton4, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton5, I_TOUCH_DISABLE);
            b2 = 1;
        }
        //Check if forward button was pressed
        if((x2>744 && x2<790) && (y2>424 && y2<470)){
            Display.img_SetWord(hndl, i4Dbutton3, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton3) ;
            //clear the screen and change to LEVEL display
            Display.gfx_Cls();
            //disable touch for buttons on previous screen
            Display.img_Disable(hndl,i4Dbutton2);
            Display.img_Disable(hndl,i4Dbutton3);
            Display.img_Disable(hndl,i4Dbutton4);
            Display.img_Disable(hndl,i4Dbutton5);
            Display.img_ClearAttributes(hndl, i4Dbutton2, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton3, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton4, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton5, I_TOUCH_DISABLE);
            f2 = 1;
         }
    }
    //---------------------------------------------------------------
    if(form == 3)              //Controls for Water Level Sensor
    {
        //Check for Touch on Display 3
        x3 = Display.touch_Get(TOUCH_GETX);
        y3 = Display.touch_Get(TOUCH_GETY);
        //Check if System Status changes
        if(SystemStatus == 1){
            i3 = (i3 + 1);
            if((i3 % 2) == 0){
                Display.img_Disable(hndl, iStatictext21);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(272, 182, 425, 211, BLACK) ;  // Rectangle4
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext26);
                Display.img_Show(hndl, iStatictext26);
            }
            if((i3 % 2) == 1){
                Display.img_Disable(hndl, iStatictext26);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(272, 182, 425, 211, BLACK) ;  // Rectangle4
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext21);
                Display.img_Show(hndl, iStatictext21);
            }
        }
        //Check is Pump Power is pressed
        if((x3>728 && x3<778) && (y3>208 && y3<258)){
            PumpPowerStatus = (PumpPowerStatus + 1);
            PumpPower = (PumpPowerStatus % 2);
            Display.img_SetWord(hndl, i4Dbutton19, IMAGE_INDEX, PumpPower);     //0 is on / 1 is off
            Display.img_Show(hndl,i4Dbutton19) ;
        }
        //Check if Data Logging is pressed
        if((x3>52 && x3<102) && (y3>288 && y3<340)){
            i33 = (i33 + 1);
            if((i33 % 2) == 1){
                LevelLogStatus = 1;
                Display.img_SetWord(hndl, i4Dbutton6, IMAGE_INDEX, LevelLogStatus);
                Display.img_Show(hndl,i4Dbutton6) ;
                Display.img_Disable(hndl, iStatictext55);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 298, 364, 323, BLACK) ;  // Rectangle5
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext23);
                Display.img_Show(hndl, iStatictext23);
            }
            if((i33 % 2) == 0){
                LevelLogStatus = 0;
                Display.img_SetWord(hndl, i4Dbutton6, IMAGE_INDEX, LevelLogStatus);
                Display.img_Show(hndl,i4Dbutton6) ;
                Display.img_Disable(hndl, iStatictext23);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 298, 364, 323, BLACK) ;  // Rectangle5
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext55);
                Display.img_Show(hndl, iStatictext55);
            }
        }
        //Check if WATER LEVEL down button
        if((x3>560 && x3<610) && (y3>340 && y3<390)){
            Display.img_SetWord(hndl, i4Dbutton9, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton9) ;
            if(LevelTarget == 1){
                 LedDigitsDisplay(Display, hndl, LevelTarget, iLeddigits3+1, 620, 2, 1, 37, 0) ;  // Leddigits3
            }
            if(LevelTarget>1 && LevelTarget<5){
                LevelTarget = LevelTarget - 1;
                LedDigitsDisplay(Display, hndl, LevelTarget, iLeddigits3+1, 620, 2, 1, 37, 0) ;  // Leddigits3
            }
            Display.img_SetWord(hndl, i4Dbutton9, IMAGE_INDEX, 0);
            Display.img_Show(hndl,i4Dbutton9) ;
        }
        //Check if WATER LEVEL up button
        if((x3>704 && x3<754) && (y3>340 && y3<390)){
            Display.img_SetWord(hndl, i4Dbutton10, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton10) ;
            if(LevelTarget == 5){
                 LedDigitsDisplay(Display, hndl, LevelTarget, iLeddigits3+1, 620, 2, 1, 37, 0) ;  // Leddigits3
            }
            if(LevelTarget < 5 && LevelTarget >= 1){
                LevelTarget = LevelTarget + 1;
                LedDigitsDisplay(Display, hndl, LevelTarget, iLeddigits3+1, 620, 2, 1, 37, 0) ;  // Leddigits3
            }
            Display.img_SetWord(hndl, i4Dbutton10, IMAGE_INDEX, 0);
            Display.img_Show(hndl,i4Dbutton10) ;
        }
        //Check if back button is pressed
        if((x3>12 && x3<70) && (y3>424 && y3<470)){
            Display.img_SetWord(hndl, i4Dbutton7, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton7) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton6);
            Display.img_Disable(hndl,i4Dbutton7);
            Display.img_Disable(hndl,i4Dbutton8);
            Display.img_Disable(hndl,i4Dbutton9);
            Display.img_Disable(hndl,i4Dbutton10);
            Display.img_ClearAttributes(hndl, i4Dbutton6, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton7, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton8, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton9, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton10, I_TOUCH_DISABLE);
            b3 = 1;
        }
        //Check if forward button is pressed
        if((x3>744 && x3<790) && (y3>424 && y3<470)){
            Display.img_SetWord(hndl, i4Dbutton8, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton8) ;
            //clear the screen and change to PRESSURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton6);
            Display.img_Disable(hndl,i4Dbutton7);
            Display.img_Disable(hndl,i4Dbutton8);
            Display.img_Disable(hndl,i4Dbutton9);
            Display.img_Disable(hndl,i4Dbutton10);
            Display.img_ClearAttributes(hndl, i4Dbutton6, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton7, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton8, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton9, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton10, I_TOUCH_DISABLE);
            f3 = 1;
        }
    }
    //---------------------------------------------------------------
    if(form == 4)             //Controls for ALL SENSOR READINGS
    {
        //Check for Touch on Form 4
        x4 = Display.touch_Get(TOUCH_GETX);
        y4 = Display.touch_Get(TOUCH_GETY);

        //check if back button is pressed
        if((x4>12 && x4<64) && (y4>424 && y4<470)){
            Display.img_SetWord(hndl, i4Dbutton11, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton11) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton11);
            Display.img_Disable(hndl,i4Dbutton12);
            Display.img_ClearAttributes(hndl, i4Dbutton11, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton12, I_TOUCH_DISABLE);
            b4 = 1;
        }
        //check if forward button is pressed
        if((x4>744 && x4<790) && (y4>424 && y4<470)){
            Display.img_SetWord(hndl, i4Dbutton12, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton12) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton11);
            Display.img_Disable(hndl,i4Dbutton12);
            Display.img_ClearAttributes(hndl, i4Dbutton11, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton12, I_TOUCH_DISABLE);
            f4 = 1;
        }
    }
    //---------------------------------------------------------------
    //Check for Touch on Form 5
    if(form == 5)
    {
       //Check for touch on form 5
        x5 = Display.touch_Get(TOUCH_GETX);
        y5 = Display.touch_Get(TOUCH_GETY);
        //Check SYSTEM STATUS
        if(SystemStatus == 1){
            i5 = (i5 + 1);
            if((i5 % 2) == 0){      //Operational
                Display.img_Disable(hndl, iStatictext47);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 181, 426, 211, BLACK) ;  // Rectangle6
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext42);
                Display.img_Show(hndl, iStatictext42);
            }
            if((i5 % 2) == 1){      //Unoperational
                Display.img_Disable(hndl, iStatictext42);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 181, 426, 211, BLACK) ;  // Rectangle6
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext47);
                Display.img_Show(hndl, iStatictext47);
            }
        }
        //Check if data logging button is pressed
        if((x5>52 && x5<102) && (y5>288 && y5<338)){
            i55 = (i55 + 1);
            if((i55 % 2) == 1){
                PresLogStatus = 1;
                Display.img_SetWord(hndl, i4Dbutton15, IMAGE_INDEX, PresLogStatus);
                Display.img_Show(hndl,i4Dbutton15) ;
                Display.img_Disable(hndl, iStatictext48);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(264, 298, 364, 323, BLACK) ;  // Rectangle7
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext45);
                Display.img_Show(hndl, iStatictext45);
            }
            if((i55 % 2) == 0){
                PresLogStatus = 0;
                Display.img_SetWord(hndl, i4Dbutton15, IMAGE_INDEX, PresLogStatus);
                Display.img_Show(hndl,i4Dbutton15) ;
                Display.img_Disable(hndl, iStatictext45);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(264, 298, 364, 323, BLACK) ;  // Rectangle7
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext48);
                Display.img_Show(hndl, iStatictext48);
            }
        }
        //check if back button is pressed
        if((x5>12 && x5<70) && (y5>424 && y5<470)){
            Display.img_SetWord(hndl, i4Dbutton13, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton13) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton13);
            Display.img_Disable(hndl,i4Dbutton14);
            Display.img_Disable(hndl,i4Dbutton15);
            Display.img_ClearAttributes(hndl, i4Dbutton13, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton14, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton15, I_TOUCH_DISABLE);
            b5 = 1;
        }
        //check if forward button is pressed
        if((x5>744 && x5<790) && (y5>424 && y5<470)){
            Display.img_SetWord(hndl, i4Dbutton14, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton14) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton13);
            Display.img_Disable(hndl,i4Dbutton14);
            Display.img_Disable(hndl,i4Dbutton15);
            Display.img_ClearAttributes(hndl, i4Dbutton13, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton14, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton15, I_TOUCH_DISABLE);
            f5 = 1;
        }
    }
    //---------------------------------------------------------------
    if(form == 6)
    {
        //Check for Touch on Form 4
        x6 = Display.touch_Get(TOUCH_GETX);
        y6 = Display.touch_Get(TOUCH_GETY);
        //CHECK SYSTEM STATUS
        if(SystemStatus == 1){
            i6 = (i6 + 1);
            if((i6 % 2) == 0){      //Unoperational
                Display.img_Disable(hndl, iStatictext52);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 178, 428, 211, BLACK) ;  // Rectangle8
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext57);
                Display.img_Show(hndl, iStatictext57);
            }
            if((i6 % 2) == 1){      //Operational
                Display.img_Disable(hndl, iStatictext57);
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 178, 428, 211, BLACK) ;  // Rectangle8
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext52);
                Display.img_Show(hndl, iStatictext52);
            }
        }
        //Check if Data Logging is pressed
        if((x6>52 && x6<102) && (y6>288 && y6<338)){
            i66 = (i66 + 1);
            if((i66 % 2) == 1){
                FlowLogStatus = 1;
                Display.img_SetWord(hndl, i4Dbutton17, IMAGE_INDEX, FlowLogStatus);
                Display.img_Show(hndl,i4Dbutton17) ;
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 293, 390, 326, BLACK) ;  // Rectangle9
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext58);
                Display.img_Show(hndl, iStatictext58);
            }
            if((i66 % 2) == 0){
                FlowLogStatus = 0;
                Display.img_SetWord(hndl, i4Dbutton17, IMAGE_INDEX, FlowLogStatus);
                Display.img_Show(hndl,i4Dbutton17) ;
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPFINE) ;
                Display.gfx_RectangleFilled(268, 293, 390, 326, BLACK) ;  // Rectangle9
                Display.gfx_OutlineColour(BLACK) ;
                Display.gfx_LinePattern(LPSOLID) ;
                Display.img_Enable(hndl, iStatictext55);
                Display.img_Show(hndl, iStatictext55);
            }
        }
        //check if back button is pressed
        if((x6>12 && x6<70) && (y6>424 && y6<470)){
            Display.img_SetWord(hndl, i4Dbutton16, IMAGE_INDEX, 1);
            Display.img_Show(hndl,i4Dbutton16) ;
            //clear the screen and change to TEMPERATURE display
            Display.gfx_Cls();
            Display.img_Disable(hndl,i4Dbutton16);
            Display.img_Disable(hndl,i4Dbutton17);
            Display.img_ClearAttributes(hndl, i4Dbutton16, I_TOUCH_DISABLE);
            Display.img_ClearAttributes(hndl, i4Dbutton17, I_TOUCH_DISABLE);
            b6 = 1;
        }
    }
  }
  //---------------------------------------------------------------
  if(f1 == 1){
     form = 2;
     f1 = 0;
  }
  if(b2 == 1){
    form = 1;
    b2 = 0;
  }
  if(f2 == 1){
    form = 3;
    f2 = 0;
  }
  if(b3 == 1){
    form = 2;
    b3 = 0;
  }
  if(f3 == 1){
    form = 4;
    f3 = 0;
  }
  if(b4 == 1){
    form = 3;
    b4 = 0;
  }
  if(f4 == 1){
    form = 5;
    f4 = 0;
  }
  if(b5 == 1){
    form = 4;
    b5 = 0;
  }
  if(f5 == 1){
    form = 6;
    f5 = 0;
  }
  if(b6 == 1){
    form = 5;
    b6 = 0;
  }

}
  //Function Definitions
    // Calculate Water Level
    float gimmeLevel(float voltage)
    {
      float waterlevel,processedVoltage;
      processedVoltage = digitalFilter(voltage);
      waterlevel = (3.5305*processedVoltage) - 5.5423;
      return waterlevel;
    }
  // Calculate Water Pressure
    float gimmePressure(float voltage)
    {
      float waterpressure;
      waterpressure = 0.6075*(voltage) - 1.9031;
      return waterpressure;
    }
  // Digital Averaging Filter for Sensor Samples
    float digitalFilter(float voltage)
    {
      int i;
      ultraSample = 0;
      for (i = 0; i < 20; i++)
      {
        ultraSample = ultraSample + voltage;
      }
      filteredSample = ultraSample / 20;
      return filteredSample;
    }
  // Calculate Flow Rate
    float gimmeFlow(float voltage)
    {
      float flowRate;
      flowRate = (0.3352*pow(voltage,2))-(0.9131*(voltage))+1.0696;
//      if (flowRate <= 0.5){
//        flowRate = (0.41*voltage) - 0.474;
//      }
//
//      else if (flowRate > 0.5) {
//        flowRate = (0.9281*voltage) - 1.7649;
//      }
      return flowRate;
    }
  // Calculate Temperature
    float gimmeTemp(float voltage)
    {
      float temperature,processedSample;
      processedSample = digitalFilter(voltage);
      temperature = (189.21)*exp((-0.272)*(processedSample));
      return temperature;
    }
