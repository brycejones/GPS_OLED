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
#include <Adafruit_GPSmod.h>
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
    Connect GPS Ground pin to Ground
    Connect GPS TX (transmit) pin to Digital 8
    Connect GPS RX (receive) pin to Digital 7
    (You can change the software serial pin numbers however
     make sure the following matches your wiring)
*/


// If using software serial (sketch example default):
//   Connect the GPS TX (transmit) pin to Digital 8
//   Connect the GPS RX (receive) pin to Digital 7
// If using hardware serial:
//   Connect the GPS TX (transmit) pin to Arduino RX1 (Digital 0)
//   Connect the GPS RX (receive) pin to matching TX1 (Digital 1)

// If using software serial, keep these lines enabled
// (you can change the pin numbers to match your wiring):
//SoftwareSerial mySerial(6, 5);    //[GPS TX,GPS RX]
//Adafruit_GPS GPS(&mySerial);

// If using hardware serial, comment
// out the above two lines and enable these two lines instead:
Adafruit_GPS GPS(&Serial1);
HardwareSerial mySerial = Serial1;

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
//#define GPSECHO  true
#define GPSECHO false

int led = 3;     // GPS fix indicator LED

// Set up the OLED Display
#if (SSD1306_LCDHEIGHT != 64)     // check to see if correct display provided
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = true;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

int count = 1;  // loop counter to check GPS query

void setup()  
{
  //while (!Serial); // wait for Leonardo/ Micro serial port to be ready

  Serial.begin(115200);  // this hdwe serial interface commuicates with PC via USB
  delay(15000);
  //mySerial.begin(9600); // this sftwe serial interface communicates with the GPS receiver
  //delay(15000);
  Serial.println("GPS hardware serial sketch for Arduino MICRO");

  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done
  
  //display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();
  display.clearDisplay();
  delay(2000);
  display.println("\nHello GPS");
  display.display();
  delay(2000);
  display.clearDisplay();
 
 // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz


  // ************************************************************
  GPS.sendCommand(PGCMD_ANTENNA);  // required to get PGTOP working
  // Commnets: brycej date: 02/18/2014
  // I think this is incorrect and needs to be updated in the header files
  // FORMAT: $PGTOP,11,value*checksum
  //    1. Active Antenna Shorted
  //    2. Using Internal Antenna $PGTOP,11,2*6E
  //    3. Using Active Antenna   $PGTOP,11,3*6F
  //  
  //GPS.sendCommand(PGTOP_ANTENNA);

  
  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE); 


  //useInterrupt(false);
  //delay(1000);


}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}



uint32_t timer = millis();

void loop()                     // run over and over again
{
 
  // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
 
  }
 
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
 // Visual indicator for GPS fix, light LED
   if (GPS.fixquality>0 ){
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      //delay(1000);               // wait for a second
    } 
    else {
       digitalWrite(led, LOW); 
    }
    
    display.clearDisplay();
    display.setCursor(0,0);   
    display.print("Time: ");
    display.print(GPS.hour, DEC); display.print(':'); 
    display.print(GPS.minute, DEC); display.print(':');
    display.print(GPS.seconds, DEC); display.print('.');
    display.print(GPS.milliseconds);
    display.print("\nPass: ");; display.print(count);    
    if (GPS.fix) { 
         display.print("\nFIX YES");
    }
    else {   
         display.print("\nFIX NO!!!");
    }     
    display.display();
    delay(2000);
    count = (count + 1) ;
 
    
/*    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.month, DEC); Serial.print('/');
    Serial.print(GPS.day, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      //Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Speed (mph): "); Serial.println(GPS.speed * 1.15078);  // Speed in miles per hour
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Antenna Status: "); Serial.print(GPS.antennastatus, DEC);    //printing antenna status
  
      switch ( GPS.antennastatus ) {
           case 1:
              Serial.print(" -Antenna Shorted!!\n");        // Fault with Antenna
           break;
           case 2:
              Serial.print(" -Using Internal Antenna\n");   // GPS using internal ceramic antenna
           break;
           case 3:
              Serial.print(" -Using Active Antenna\n");     // GPS using active antenna
           break;
           
           default:
           // Get out of here
           break;
           }
  
      Serial.print("\n");
      
    }
*/    
    
  }
}
