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

   History/ Status
   =======
   2014/MAR/12  - initial sketch
   2014/MAR/15  - not getting GPS fix
   2014/MAR/16  - found GPS fix issue memory contraint


Required hardware:
  -Arduino (I used Micro)
  -Adafruit Ultimate GPS v3
  -Adafruit SSD1306 128x64 OLED display

Wiring:

  Connections for Adafruit Ultimate GPS v3
  ============
   Connect GPS power to 5V
   Connect GPS ground to ground
   Connect the GPS TX (transmit) pin to Arduino RX1 (Digital 0)
   Connect the GPS RX (receive) pin to matching TX1 (Digital 1)   
  

   Connections for the Adafruit 128x64 SPI OLED
   ===========
   Connect OLED_MOSI(DATA) 9
   Connect OLED_CLS(Clock) 10
   Connect OLED_DC(SAO) 11
   Connect OLED_RESET 13
   Connect OLED_CS 12
   Connect OLED_VDD(Vin) to 3.3V DC (5V tolerant)
   Connect OLED_GROUND to common ground


Special Notes: This sketch is very close to exceeding the OLED buffer
and when you do, then the serial data from the GPS will not be received
and the symptom is displaying NO GPS Fix!! Frustrating to debug.

This sketch uses a modified Adafruit's GPS library to allow reading the
status of the GPS antenna to determine if the GPS is using the onboard ceramic
antenna, the external active antenna, or if there is an antenna short creating
a fault. 

*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>          // OLED/LCD Display grapshics library 
#include <Adafruit_SSD1306.h>      // OLED Graphics display              
//#include <Adafruit_GPS.h>        // Standard Adafruit GPS library
#include <Adafruit_GPSmod.h>       // Modified Adafruit GPS library providing antenna status  
#include <SoftwareSerial.h>

// Forward declarations
void GPS_Setup();
void GPS_Tick();
void OLED_Setup();


// mySerial Object
SoftwareSerial mySerial(3, 2);

// GPS Object
Adafruit_GPS GPS(&Serial1);

// OLED Object
#define OLED_DC 11
#define OLED_CS 12
#define OLED_CLK 10
#define OLED_MOSI 9
#define OLED_RESET 13
Adafruit_SSD1306 OLED(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Timer
uint32_t timer = millis();

void OLED_Setup()
{
   // by default, we'll generate the high voltage from the 3.3v line internally! 
  OLED.begin(SSD1306_SWITCHCAPVCC);
  OLED.display();        // Display Adafruit Splash Screen :-)
  delay(1000);;
  OLED.clearDisplay();   // clears the screen and buffer
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE);
}



void GPS_Setup()
{
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  // required for number of SATS and altitude.
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  // ************************************************************
  //GPS.sendCommand(PGCMD_ANTENNA);  // required to get PGTOP working
  // Commnets: brycej date: 02/18/2014
  // I think this is incorrect and needs to be updated in the header files
  // FORMAT: $PGTOP,11,value*checksum
  //    1. Active Antenna Shorted
  //    2. Using Internal Antenna $PGTOP,11,2*6E
  //    3. Using Active Antenna   $PGTOP,11,3*6F
  //  
  GPS.sendCommand(PGTOP_ANTENNA); 
  delay(1000);
  
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}



// Read in and tick the GPS class
void GPS_Tick()
{
  
  // 'hand query' the GPS for new data  
  // read data from the GPS in the 'main loop'
  char c = GPS.read(); 
  
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) 
  {
      if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
       return;  // we can fail to parse a sentence in which case we should just wait for another
  }
}



// Main Setup
void setup()  
{ 
  
  //
  OLED_Setup();
  OLED.setCursor(0,0);
  OLED.println("Waking up...");
  OLED.display();
  
  //
  GPS_Setup();
  
  //
  Serial.begin(115200);
 
  //
  OLED.clearDisplay(); 
  OLED.display();

}
// run over and over again
void loop()                     
{

  // Tick the GPS
  GPS_Tick();


  // 1Hz update rate
  if (millis() - timer > 1000) 
  { 
    //
    Serial.print(".");
    
    
    // Reset the timer
    timer = millis(); 
  
    // If GPS FIX then display data. NOTE: this very close to exceeding display buffer!!
    if (GPS.fix > 0) 
    {
        OLED.setTextSize(1);
        OLED.setTextColor(WHITE);
        OLED.clearDisplay();        
        OLED.setCursor(0,0);
        OLED.print("GPS DATA   ");
        OLED.print(GPS.month, DEC); OLED.print('/');
        OLED.print(GPS.day, DEC); OLED.print("/20");
        OLED.print(GPS.year, DEC);
        OLED.print("\nUTC Time: ");        
        OLED.print(GPS.hour, DEC); OLED.print(':');
        OLED.print(GPS.minute, DEC); OLED.print(':');
        OLED.print(GPS.seconds, DEC); OLED.print('.');   
        OLED.print("\n");
        OLED.print("Lat: ");
        OLED.print(GPS.latitude, 4); OLED.print(GPS.lat);
        OLED.print("\n");
        OLED.print("Lon: "); 
        OLED.print(GPS.longitude, 4); OLED.print(GPS.lon);
        OLED.print("\n");
        
        OLED.print("Speed(mph): "); OLED.print(GPS.speed * 1.15078);OLED.print("\n");  // Convert Knots to MPH
        //OLED.print("Angle: "); OLED.print(GPS.angle);OLED.print("\n");
        OLED.print("Altd: "); OLED.print(GPS.altitude);OLED.print("\n");
        OLED.print("Sats: "); OLED.print((int)GPS.satellites);
        OLED.print("\nAntenna: ");
        OLED.print(GPS.antennastatus, DEC);
        
        
        switch ( int(GPS.antennastatus) ) {
           case 1:
              OLED.print(" Bad");        // Fault with Antenna, shorted
           break;
           case 2:
              OLED.print(" Int");        // GPS using internal ceramic antenna
           break;
           case 3:
              OLED.print(" Active");     // GPS using active antenna
           break;     
           default:
           // Get out of here
           break;
           }
         
        OLED.print("\n");
        
        OLED.display();
    }
    // No GPS fix yet so 
    else 
    {
        OLED.setTextSize(2);
        OLED.setTextColor(WHITE); 
        OLED.clearDisplay();
        OLED.setCursor(0,20);
        OLED.print("NO FIX");
        OLED.display();
    }  
  }
}
