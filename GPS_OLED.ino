/*********************************************************************
This sketch interfaces a 128x64 Monochrome OLED based on SSD1306 drivers
to interface Adafruit's Ultimate GPS and displaying location information
to the tiny OLED display.

  Pick up an OLED display in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

  Pick up the high performing Ultimate v3 GPS in the adafruit shop!
  ------> http://www.adafruit.com/products/746

This example is for a 128x64 size display using SPI to communicate.
Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

Author: Bryce Jones
Date: 12 MAR 2014

Required hardware:
  -Arduino (I used Micro)
  -Adafruit Ultimate GPS v3
  -Adafruit SSD1306 128x64 OLED display

   History
   =======
   2013/MAR/12  - initial sketch

*********************************************************************/

#include <Wire.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>          /* OLED/LCD Gisplay grapshics library */
#include <Adafruit_SSD1306.h>      /* OLED Graphics display              */

/* 
   To use this OLED driver you will need to download the Adafruit
   OLED library and include it in your libraries folder. I used Adafruit's
   128x64 OLED. It uses the SSD1306 driver and this sketch uses an SPI
   interface to the OLED for a speedy refresh!
   
   Connections for the Adafruit 128x64 SPI OLED
   ===========
   Connect OLED_MOSI(DATA) 9
   Connect OLED_CLS(Clock) 10
   Connect OLED_DC(SAO) 11
   Connect OLED_RESET 13
   Connect OLED_CS 12
   Connect OLED_VDD(Vin) to 3.3V DC (5V tolerant)
   Connect OLED_GROUND to common ground

*/

#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


/*  
    GPS Software Serial GPS Connections
    ===============================
    Connect GPS Power pin to 5V
    Connect GPS Ground pin to ground
    Connect GPS TX (transmit) pin to Digital 8
    Connect GPS RX (receive) pin to Digital 7
    (You can change the software serial pin numbers however
     make sure following matches your wiring)
*/
SoftwareSerial mySerial(8, 7);


/*             Set Up GPS Recevier                  */
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
// turn on GPRMC and GGA
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn off output
#define PMTK_SET_NMEA_OUTPUT_OFF "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

#define PMTK_Q_RELEASE "$PMTK605*31"

/*          End Setting Up GPS Echo                */  

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()  {
  while (!Serial); // wait for Leonardo/ Micro serial port to be ready

  Serial.begin(57600); // this baud rate doesn't actually matter!
  mySerial.begin(9600);
  delay(2000);
  Serial.println("Get version!");
  mySerial.println(PMTK_Q_RELEASE);
  
  // you can send various commands to get it started
  //mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  mySerial.println(PMTK_SET_NMEA_OUTPUT_ALLDATA);

  mySerial.println(PMTK_SET_NMEA_UPDATE_1HZ);
 
   // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done
    display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("\nHello GPS");
  display.display();
  delay(2000);
  display.clearDisplay();
   
}


void loop() {

  if (Serial.available()) {
   char c = Serial.read();
   Serial.write(c);
   mySerial.write(c);
  }
  if (mySerial.available()) {
    char c = mySerial.read();
    Serial.write(c);
  } 
  
}




