/* PixMobIRForwarder, 

   Modifed by Dave A. July 16 2025:
    
   Hardware: M5StickC Plus:  
   

   #########################################################################################################################################################
   Source and credits to:
   https://github.com/danielweidman/pixmob-ir-reverse-engineering/blob/main/arduino_sender/PixMob_Transmitter_ESP32/PixMob_Transmitter_ESP32.ino
   https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
   https://ivanr3d.com/tools/led-wristband/
   IR Definitions: https://github.com/danielweidman/pixmob-ir-reverse-engineering/blob/main/www/js/effects_definitions.js
   U8G2:  https://github.com/olikraus/u8g2/blob/master/sys/arduino/u8g2_full_buffer/HelloWorld/HelloWorld.ino

   
   Version 1.0, August 2022
   Daniel Weidman, danielweidman.com
   
   Based on Ken Shirriff's IrsendDemo
   Copyright 2009 Ken Shirriff, http://arcfn.com

   This script takes IR codes for PixMob bracelets from a connected computer over Serial or 
   other device over Bluetooth and transmits those codes to PixMob bracelets.

    ------------------------------------------------------------------------------
   An IR LED circuit *MUST* be connected to the Arduino on a pin
   as specified by kIrLed below.

   TL;DR: The IR LED needs to be driven by a transistor for a good result.

   Suggested circuit:
       https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending

   Common mistakes & tips:
 *   * Don't just connect the IR LED directly to the pin, it won't
       have enough current to drive the IR LED effectively.
 *   * Make sure you have the IR LED polarity correct.
       See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
 *   * Typical digital camera/phones can be used to see if the IR LED is flashed.
       Replace the IR LED with a normal LED if you don't have a digital camera
       when debugging.
 *   * Avoid using the following pins unless you really know what you are doing:
 *     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
 *     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
 *     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.
*/
#include <Arduino.h>
#include <M5StickCPlus.h>
#include "AXP192.h"         //M5Stick-C Plus
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Wire.h>
#include "fonts/Orbitron_ExtraBold9pt7b.h"
#include "fonts/Orbitron_ExtraBold12pt7b.h" 
#include "fonts/Orbitron_ExtraBold14pt7b.h" 


//##############################################################################
// D E B U G  - set True to turn on Serial Debugging
#define debug false
//##############################################################################

//#define LDRSensorPIN 33                // 26 causes issues, so dont use it
//#define CAMERA HERO7                   // Change here for your camera

// SET THIS TO THE DATA PIN USED FOR THE IR TRANSMITTER
const uint16_t IrLed = 9;

boolean whichTab = 0;                  // TabSelection
int seqSelection = 1;                 // For menuing system
int seqToPlay = 1;
boolean option2 = 0;               // Using GoPro Camera motorized rail?
long prevUpdateMillis = 0;
long updateIntervalMS = 2000;          // Update the display status (bottom area) every 2 seconds
char battStats[40] = "" ; 
int option1 = 0;     
boolean repeatSeq = false;

//LED Sequences
const uint16_t yellow3Fade6[] = {1400,1400,700,700,700,700,1400,2800,700,2100,700,700,700,1400,700,1400,1400,2800,1400,2800,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  //WORKS
const uint16_t redFade6[] = {1400,1400,700,700,700,1400,700,2800,700,2100,1400,700,700,700,700,1400,1400,2800,1400,2800,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  
const uint16_t magentaFade6[] = {700,700,700,700,1400,1400,1400,2800,700,2100,1400,2100,700,700,700,700,1400,2100,700,700,700,2100,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  
const uint16_t greenFade6[] = {1400,1400,700,700,700,700,1400,2800,700,1400,700,1400,700,1400,700,1400,1400,2800,1400,2800,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  
const uint16_t blueFade6[] = {700,700,700,2100,1400,1400,700,2100,700,1400,700,700,700,1400,1400,700,700,1400,700,700,700,700,700,700,700,2100,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  
const uint16_t redOrangeFade6[] = {700,700,700,700,1400,1400,1400,2800,700,2800,1400,1400,700,700,700,1400,700,2100,1400,2800,700,1400,700,1400,1400,700,700,1400,1400,2800,1400,2800,700};  
const uint16_t specialRandomFade[] = {700,700,700,2100,1400,700,700,2800,700,700,700,1400,700,2100,700,700,700,1400,700,1400,700,1400,700,2100,700};  


const int yellow3Fade6_len = sizeof(yellow3Fade6) / sizeof(yellow3Fade6[0]);
const int redFade6_len = sizeof(redFade6) / sizeof(redFade6[0]);
const int magentaFade6_len = sizeof(magentaFade6) / sizeof(magentaFade6[0]);
const int greenFade6_len = sizeof(greenFade6) / sizeof(greenFade6[0]);
const int blueFade6_len = sizeof(blueFade6) / sizeof(blueFade6[0]);
const int redOrangeFade6_len = sizeof(redOrangeFade6) / sizeof(redOrangeFade6[0]);
const int specialRandomFade_len = sizeof(specialRandomFade) / sizeof(specialRandomFade[0]);

//FUNCTION DECLARATIONS
void lcdClear();
void updateStats();
void updateStatsMsg(String textToShow, int xVal, uint16_t clr);
const char *formatM5BattInfo(float battV, float battPwr, float usbV, char * battStats);
void doHMI(int tab, int menuItem, int settingChange);


void sendyellow3Fade6();
void sendredFade6();
void sendmagentaFade6();
void sendgreenFade6();
void sendblueFade6();
void sendredOrangeFade6();
void sendspecialRandomFade();



IRsend irsend(IrLed);

void setup() 
{

  Serial.begin(115200);
  irsend.begin();
    
  M5.begin();                             // Initialize M5StickC Plus.
    
  M5.Lcd.setRotation(3);                  // Rotate the screen.
  doHMI(whichTab, seqSelection, 0);         // Show initial HMI interface

  /*
  sendyellow3Fade6();delay(1000);  //delays are needed, else some dont fire.
  sendredFade6();delay(1000);
  sendmagentaFade6();delay(1000);
  sendgreenFade6();delay(1000);
  sendblueFade6();delay(1000);
  sendredOrangeFade6();delay(1000);
  sendspecialRandomFade();delay(1000);
  */
    
}

void loop() 
{
  unsigned long currUpdateMillis = millis();
  M5.update();                                        // Read the press state of the key.

  //Navigate the options 
  if(M5.BtnB.wasPressed())
  {
    seqSelection++;
    //seqToPlay = seqSelection;
    if (seqSelection > 7) seqSelection = 1;
    doHMI(whichTab, seqSelection, 0);
  }

  //Save and act on choices
  //if (M5.BtnB.pressedFor(1000))
  if (M5.BtnB.pressedFor(1000))
  {
    /*
    if (seqSelection >=1 && seqSelection <=7)  //LED sequences
    {
      seqToPlay = seqSelection;
    }
    else
    {
      repeatSeq = !repeatSeq;
    }
    */
    repeatSeq = !repeatSeq;
    doHMI(whichTab, seqSelection, 1);  //change the selected setting
    delay(2000);
  }
  
  
  /*
  if (M5.BtnB.pressedFor(1000) && seqSelection ==8)
  {
    repeatSeq = !repeatSeq;
    doHMI(whichTab, seqSelection, 0); 
    delay(2000);
  }
  */

     
  //M5.update(); 
  //if(M5.BtnA.pressedFor(1000))
  if(M5.BtnA.isPressed())
  {
   //Execute the sequence.
     switch (seqSelection) 
     {
      case 1: sendblueFade6();delay(1000); break;
      case 2: sendredFade6();delay(1000); break;
      case 3: sendmagentaFade6();delay(1000); break;
      case 4: sendyellow3Fade6();delay(1000);break;
      case 5: sendredOrangeFade6();delay(1000); break;
      case 6: sendgreenFade6();delay(1000); break;
      case 7: sendblueFade6();delay(1000);sendredFade6();delay(1000);sendmagentaFade6();delay(1000);sendyellow3Fade6();delay(1000);sendgreenFade6();delay(1000);sendredOrangeFade6();delay(1000); break;
      default:break;
     }
    
    if (repeatSeq)
    {
      while (true) 
      {
        switch (seqSelection) 
        {
          case 1: sendblueFade6();delay(1000); break;
          case 2: sendredFade6();delay(1000); break;
          case 3: sendmagentaFade6();delay(1000); break;
          case 4: sendyellow3Fade6();delay(1000);break;
          case 5: sendgreenFade6();delay(1000); break;
          case 6: sendredOrangeFade6();delay(1000); break;
          case 7: sendspecialRandomFade();delay(1000); break;
          default:break;
        }
        M5.update();
        if(M5.BtnB.isPressed()) //break out of the while
        //if(M5.BtnB.isReleased()) //break out of the while
        {
          //repeatSeq = !repeatSeq; 
          //doHMI(whichTab, seqSelection, 0);
          break;
        }  
      }

    }
  }

  
  

  //regular updates to the HMI - only show if on Run tab
  if (currUpdateMillis - prevUpdateMillis > updateIntervalMS && whichTab==1)
  {
    //updateStats(); 
    //if (debug) {Serial.println("Update Stats:  Should only be every 2 seconds..");}

     // updateStatsMsg(formatM5BattInfo(M5.Axp.GetBatVoltage(), M5.Axp.GetBatPower(), M5.Axp.GetVBusVoltage(), battStats), 8, TFT_GREEN);
      //Serial.print("Batt Stuff:  ");Serial.println(formatM5BattInfo(M5.Axp.GetBatVoltage(), M5.Axp.GetBatPower(), M5.Axp.GetVBusVoltage(), battStats));  
    prevUpdateMillis = currUpdateMillis;
  }

 
  
}  //END LOOP
//########################################################################################
//########################################################################################

void doHMI(int tab, int menuItem, int settingChange)
{
  //fillRect(x, y, w, h, color);
  //drawRoundRect(x, y, w, h, radius, colour)
  //drawLine(x0, y0, x1, y1, color);
  //drawCircle(x0, y0, r, color);
  //fillCircle(x0, y0, r, color);
  //drawTriangle(x0, y0, x1, y1, x2, y2, color);
  //fillTriangle(x0, y0, x1, y1, x2, y2, color);
  //screen is 135H x 240W

  lcdClear();

  if (!tab)  //MAIN SCREEN - PIXMOB
  {
    //M5.Lcd.drawRoundRect(119, 5, 116, 24, 4, TFT_DARKGREY);    //RUN Tab
    //M5.Lcd.drawRoundRect(127, 5, 109, 24, 4, TFT_GREEN);         //RUN Tab
    M5.Lcd.fillRect(120, 26, 114, 4, TFT_BLACK);
  
    M5.Lcd.drawRoundRect(4, 5, 146, 24, 4, TFT_BLUE);    //PIXMOB Tab box
    M5.Lcd.drawRoundRect(4, 26, 232, 106, 4, TFT_BLUE);  //box around rest of screen
    M5.Lcd.fillRect(5, 26, 144, 4, TFT_BLACK);           //blank out line below setup text
    M5.Lcd.drawLine(4, 15, 4, 30, TFT_BLUE);             //Tiny line connecting upper tab to lower box
    //S E T U P tab
    M5.Lcd.setFreeFont(&Orbitron_ExtraBold9pt7b);
    M5.Lcd.setCursor(20, 25);
    M5.Lcd.setTextColor(TFT_GREEN); 
    M5.Lcd.print("P I X M O B");   
    //R U N  tab
    //M5.Lcd.setCursor(150, 23);
    //M5.Lcd.setTextColor(TFT_GREEN); 
    //M5.Lcd.print("R U N");   

    //SETUP SEQUENCE BALLS USING COLOURS
    M5.Lcd.fillCircle(22, 60, 14, TFT_DARKGREY);
    M5.Lcd.fillCircle(22, 60, 11, TFT_BLUE);

    M5.Lcd.fillCircle(60, 60, 14, TFT_DARKGREY);
    M5.Lcd.fillCircle(60, 60, 11, TFT_RED);

    M5.Lcd.fillCircle(98, 60, 14, TFT_DARKGREY);
    M5.Lcd.fillCircle(98, 60, 11, TFT_MAGENTA);

    M5.Lcd.fillCircle(136, 60, 14, TFT_DARKGREY);
    M5.Lcd.fillCircle(136, 60, 11, TFT_YELLOW);

    M5.Lcd.fillCircle(172, 60, 14, TFT_DARKGREY);
    M5.Lcd.fillCircle(172, 60, 11, TFT_RED);
    
    M5.Lcd.fillCircle(210, 60, 14, TFT_CYAN);
    M5.Lcd.fillCircle(210, 60, 11, TFT_GREEN);
    
    M5.Lcd.fillCircle(22, 100, 14, TFT_PINK);
    M5.Lcd.fillCircle(22, 100, 11, TFT_BLACK);


    //Option 1
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setCursor(130, 120); 
    M5.Lcd.print("RPT");  
   
    /*
    if (!option1)
    {
      M5.Lcd.drawRoundRect(185, 107, 38, 15, 6, TFT_BLUE); 
      M5.Lcd.fillCircle(190, 114, 11, TFT_BLUE);
      M5.Lcd.fillCircle(190, 114, 10, TFT_DARKGREY);
    } else {
      M5.Lcd.drawRoundRect(185, 207, 38, 15, 6, TFT_BLUE); 
      M5.Lcd.fillCircle(214, 114, 11, TFT_BLUE);
      M5.Lcd.fillCircle(214, 114, 10, TFT_GREEN);
    }
    */  



    //M5.Lcd.setCursor(100, 120); //x,y
    //M5.Lcd.setTextColor(TFT_BLUE);
    //M5.Lcd.print(seqSelection);
    
    /*
    //Option 2
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setCursor(25, 90); 
    M5.Lcd.print("OPT 2");  
    if (!option2)
    {
      M5.Lcd.drawRoundRect(185, 79, 38, 15, 6, TFT_BLUE); 
      M5.Lcd.fillCircle(190, 86, 11, TFT_BLUE);
      M5.Lcd.fillCircle(190, 86, 10, TFT_DARKGREY);
    } else {
      M5.Lcd.drawRoundRect(185, 79, 38, 15, 6, TFT_BLUE); 
      M5.Lcd.fillCircle(214, 86, 11, TFT_BLUE);
      M5.Lcd.fillCircle(214, 86, 10, TFT_GREEN);
    }
    */
     
    //fillTriangle(x0, y0, x1, y1, x2, y2, color);  spacing is 38 px between circles
    switch (menuItem) 
    {
      case 1: M5.Lcd.fillTriangle(15,90,29,90,22,80, TFT_RED); break;
      case 2: M5.Lcd.fillTriangle(53,90,67,90,60,80, TFT_RED); break;
      case 3: M5.Lcd.fillTriangle(91,90,105,90,98,80, TFT_RED); break;
      case 4: M5.Lcd.fillTriangle(129,90,143,90,136,80, TFT_RED); break;
      case 5: M5.Lcd.fillTriangle(167,90,181,90,174,80, TFT_RED); break;
      case 6: M5.Lcd.fillTriangle(205,90,219,90,212,80, TFT_RED); break;
      case 7: M5.Lcd.fillTriangle(15,130,29,130,22,120, TFT_RED); break;
      //case 8: M5.Lcd.fillTriangle(205,90,219,90,212,100, TFT_RED); break;
      default:break;
    }
    //if (menuItem <=7) seqSelection = menuItem;  //only change if one of the 7 LED sequences
      
   
    

    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setCursor(50, 120); 
    M5.Lcd.print("SEQ: " + (String)menuItem);  

    if (!repeatSeq)
     {
        M5.Lcd.drawRoundRect(185, 107, 38, 15, 6, TFT_BLUE); 
        M5.Lcd.fillCircle(190, 114, 11, TFT_BLUE);
        M5.Lcd.fillCircle(190, 114, 10, TFT_DARKGREY);
     } else {
        M5.Lcd.drawRoundRect(185, 207, 38, 15, 6, TFT_BLUE); 
        M5.Lcd.fillCircle(214, 114, 11, TFT_BLUE);
        M5.Lcd.fillCircle(214, 114, 10, TFT_GREEN);
    }      
    

    //if (debug) {Serial.println("Seq to play:");}
    //if (debug) {Serial.print(seqToPlay);}
    //if (debug) {Serial.print(" | repeat: ");}
    //if (debug) {Serial.print(repeatSeq);}


    /* 
    switch (menuItem) 
    {
      case 1:
        //FRAME SKIP SETTING
        M5.Lcd.fillTriangle(10, 62, 10, 48, 20, 55, TFT_RED); //0 set is bottom, 1st is top, 2nd is right point
        if (settingChange)
        {
          option1 = !option1;
          doHMI(whichTab, menuItem, 0);   //refresh the menu
        }
      break;
      case 2:
        //USE SLIDER SETTING
        M5.Lcd.fillTriangle(10, 91, 10, 77, 20, 84, TFT_RED); //0 set is bottom, 1st is top, 2nd is right point
        if (settingChange)
        {
          option2 = !option2;
          doHMI(whichTab, menuItem, 0);   //refresh the menu
        }
      break;
      default:
        // statements
      break;
    }
    */
  
  
  }
  /*
  else
  {  //RUN TAB whichTab 1 - showing RUN tab section
    
    //R U N  tab box
    M5.Lcd.drawRoundRect(127, 5, 109, 24, 4, TFT_GREEN);         //RUN Tab
    M5.Lcd.fillRect(120, 26, 114, 4, TFT_BLACK);
  
    M5.Lcd.drawRoundRect(4, 26, 232, 109, 4, TFT_GREEN);  //box around rest of screen
    M5.Lcd.fillRect(128, 26, 108, 4, TFT_BLACK);         //blank out line below setup text
    M5.Lcd.drawLine(235, 15, 235, 30, TFT_GREEN);         //Tiny line connecting upper tab to lower box

    //Run tab menu
    M5.Lcd.setCursor(150, 23);
    M5.Lcd.setTextColor(TFT_GREEN); 
    M5.Lcd.print("R U N");   

    //Items
    M5.Lcd.setCursor(10, 55); 
    M5.Lcd.setTextColor(TFT_BLUE); 
    M5.Lcd.print("R E C O R D"); 
    M5.Lcd.setCursor(155, 55); 
    M5.Lcd.setTextColor(TFT_RED); 
    M5.Lcd.print("O F F");    

    M5.Lcd.setCursor(10, 80); 
    M5.Lcd.setTextColor(TFT_BLUE); 
    M5.Lcd.print("F R A M E #"); 
    M5.Lcd.setCursor(155, 80); 
    M5.Lcd.setTextColor(TFT_GREEN); 
    M5.Lcd.print("0");     

    //Status Box
    //M5.Lcd.fillRect(2, 91, 234, 109, TFT_LIGHTGREY); 
    M5.Lcd.fillRoundRect(4, 89, 232, 46, 4, TFT_BLACK); 
    M5.Lcd.drawRoundRect(4, 89, 232, 46, 4, TFT_GREEN);  //drawRoundRect(x, y, w, h, radius, colour)
  }
  */  

}

//########################################################################################
void sendspecialRandomFade()
{
  irsend.sendRaw(specialRandomFade, specialRandomFade_len, 38);delay(3);
}
//########################################################################################

void sendredOrangeFade6()
{
  irsend.sendRaw(redOrangeFade6, redOrangeFade6_len, 38);delay(3);
}
//########################################################################################

void sendblueFade6()
{
  irsend.sendRaw(blueFade6, blueFade6_len, 38);delay(3);
}
//########################################################################################


void sendgreenFade6()
{
  irsend.sendRaw(greenFade6, greenFade6_len, 38);delay(3);
}
//########################################################################################


void sendmagentaFade6()
{
  irsend.sendRaw(magentaFade6, magentaFade6_len, 38);delay(3);
}
//########################################################################################

void sendredFade6()
{
  irsend.sendRaw(redFade6, redFade6_len, 38);delay(3);
}
//########################################################################################

void sendyellow3Fade6() {
  irsend.sendRaw(yellow3Fade6, yellow3Fade6_len, 38);delay(3);
}
//########################################################################################


void lcdClear()
{
  M5.Lcd.fillScreen(BLACK);  
}
//########################################################################################



void updateStats()
{
  M5.Lcd.setFreeFont(&Orbitron_ExtraBold9pt7b);

  //Update Record State
  M5.Lcd.fillRect(150, 35, 70, 25, TFT_BLACK);  //screen is 135H x 240W  //fillRect(x, y, w, h, color);
  M5.Lcd.setCursor(160, 55); 
    M5.Lcd.setTextColor(TFT_GREEN); 
    M5.Lcd.print("O N");   
  
  //Update Frame Count
  M5.Lcd.fillRect(150, 65, 70, 20, TFT_BLACK);  //screen is 135H x 240W  //fillRect(x, y, w, h, color);
  M5.Lcd.setCursor(160, 80); 
  M5.Lcd.setTextColor(TFT_GREEN); 
  M5.Lcd.print("XXX");   
}
//########################################################################################

void updateStatsMsg(String textToShow, int xVal, uint16_t clr)
{
  //M5.Lcd.drawRoundRect(4, 89, 232, 50, 4, TFT_PURPLE);  // see further up for this box
  //M5.Lcd.fillRect(11, 88, 230, 40, TFT_OLIVE);  //clear old values.  screen is 135H x 240W  //fillRect(x, y, w, h, color);
  M5.Lcd.fillRoundRect(4, 89, 232, 46, 4, TFT_BLACK);  // Also created above in doHMI()
  M5.Lcd.drawRoundRect(4, 89, 232, 46, 4, TFT_GREEN);  //drawRoundRect(x, y, w, h, radius, colour)  
  M5.Lcd.setCursor(xVal, 108); //x,y
  M5.Lcd.setTextColor(clr);
  M5.Lcd.println(textToShow);
}
//##################################################################################
const char *formatM5BattInfo(float battV, float battPwr, float usbV, char * battStats)
{
  //char buffer[40] = "";
  //char bV[6];
  //char bP[6];
  //char uV[6];

  //dtostrf(battV, 6, 1, bV);
  //dtostrf(battPwr, 6, 1, bP);
  //dtostrf(usbV, 6, 1, uV);

  //sprintf(buffer, "BattV: %2.1f V BattP: %2.1f V USBV: %2.1f V", bV, bP, uV);
  //sprintf(battStats, "BV: %2.1fV %2.1fmW \n            USBV: %2.1fV", battV, battPwr, usbV);
  sprintf(battStats, "BV: %2.1fV %2.1fmW  %2.1fV", battV, battPwr);
  //Serial.println(buffer);
  //Serial.print(battV);Serial.print("   ");
  //Serial.print(battPwr);Serial.print("   ");
  //Serial.println(usbV);
  
  return battStats;
  
  //sprintf(displayString, "value:%7d.%d%d", (int)num, int(num*10)%10, int(num*100)%10);
  //char buffer[40];
  //sprintf(buffer, "The %d burritos are %s degrees F", numBurritos, tempStr);
  //Serial.println(buffer);
  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
}
//########################################################################################
