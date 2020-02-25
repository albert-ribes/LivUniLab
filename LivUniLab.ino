
#include <Wire.h> // I2C

//BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//BME280 T/H/P sensor
Adafruit_BME280 bme;
#define BME280_ADDRESS (0x76)
//#define SEALEVELPRESSURE_HPA (1013.25)
float temperature, humidity, pressure, altitude;


//Display OLED
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET D5
/* Object named display, of the class Adafruit_SSD1306 */
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//RTC DS3231
#include <RtcDS3231.h> //RTC library
RtcDS3231<TwoWire> rtcObject(Wire);

//SD card
#include <SD.h>
#define CS_PIN  D8
boolean sd_enabled = false;

//Wifi
#include <ESP8266WiFi.h>
const char* ssid     = "XXXXXXXX";                // your network SSID (name)
const char* password = "xxxxxxxx";                    // your network password
#define NB_TRYWIFI        120  //tentatives per connectar a WiFi
boolean wifi_con = true;
String MAC ="";

//Time
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

//Time alarms
#include "Schedule.h"
#include <ESP8266TimeAlarms.h>
int time_read_sensors = 20; //secons
int time_write_sd = 60; //seconds
#include <Ticker.h>
Ticker reader;
Ticker writter;


//NTP
#include <NTPtimeESP.h>
#define DEBUG_ON
NTPtime NTPch("ch.pool.ntp.org");

/*
 * The structure contains following fields:
 * struct strDateTime
{
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
};
 */
strDateTime dateTime;
/*
  %a Abbreviated weekday name
  %A Full weekday name
  %b Abbreviated month name
  %B Full month name
  %c Date and time representation for your locale
  %d Day of month as a decimal number (01-31)
  %H Hour in 24-hour format (00-23)
  %I Hour in 12-hour format (01-12)
  %j Day of year as decimal number (001-366)
  %m Month as decimal number (01-12)
  %M Minute as decimal number (00-59)
  %p Current locale's A.M./P.M. indicator for 12-hour clock
  %S Second as decimal number (00-59)
  %U Week of year as decimal number,  Sunday as first day of week (00-51)
  %w Weekday as decimal number (0-6; Sunday is 0)
  %W Week of year as decimal number, Monday as first day of week (00-51)
  %x Date representation for current locale
  %X Time representation for current locale
  %y Year without century, as decimal number (00-99)
  %Y Year with century, as decimal number
  %z %Z Time-zone name or abbreviation, (no characters if time zone is unknown)
  %% Percent sign
  You can include text literals (such as spaces and colons) to make a neater display or for padding between adjoining columns.
  You can suppress the display of leading zeroes  by using the "#" character  (%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y)
*/
char buffer[80];

String inputString="";
boolean write_to_storage=true;

// https://diyusthad.com/image2cpp
// 'uGreen', 128x64px
const unsigned char myBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0xbf, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xcf, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe7, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xff, 0xe3, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xe1, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xe1, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xe1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xa0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x20, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x10, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/**************************************/
/* Control Version */
/**************************************/
String Code = "LivUniLab";
String Version = "01.00";

////////////////////////////////////////
/* 
 *  SETUP
 */
////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  inputString.reserve(200);

  Serial.println("");
  Serial.println("##############################################");
  Serial.println("Code: " + Code + " , Version: " + Version);
  Serial.println("##############################################");
  Serial.println("Initializing device ...");

  //OLED display
  Serial.println("- Starting the 128x64 OLED Display");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); /* Initialize display with address 0x3C */
  display.setTextSize(1);
  display.clearDisplay();  /* Clear display */
  display.setTextColor(WHITE);  /* Color of text*/
  display.setCursor(0, 06);
  display.println(Code + ", v:" +Version);
  display.setCursor(0, 16);
  display.println("------------------");
  display.setCursor(0, 26);
  display.println("Intializing device");
  display.display();

  //RTC3231
  rtcObject.Begin();    //Starts I2C
  
  //BME280
  Serial.println("- Starting the BME280 T/H/P sensor");
  bme.begin(BME280_ADDRESS);

  //SD card
  if (!SD.begin(CS_PIN)) {
    Serial.println("- SD card not present");
    sd_enabled = false;
  }
  else{ 
    Serial.println("- SD card present");
    sd_enabled = true;
  }

  //Wifi
  Serial.printf("- Connecting to WiFi SSID %s ", ssid);
  MAC = String(WiFi.macAddress());
  
  WiFi.mode(WIFI_STA);   
  WiFi.begin (ssid, password);
  //WiFi.config(myIP, myGW, myNM);
  //check wi-fi is connected to wi-fi network
  int _try = 0;
  wifi_con=true;
  int num=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    drawDot(num);
    num++;
    _try++;
    
    if ( _try >= NB_TRYWIFI ) {
        Serial.println(" KO!");
        Serial.println("- Not connected to the WiFi network");
        //Serial.println("Continuing ahead");
        wifi_con=false;
        break;
        //ESP.deepSleep(10e6);
    }
    
  }
  if (wifi_con == true){
    Serial.println(" OK!");
    Serial.println("- Network details:");
    Serial.print("    MAC:\t");  Serial.println(MAC);
    Serial.print("    IP adress:\t");  Serial.println(WiFi.localIP());
    Serial.print("    Netmask:\t");  Serial.println(WiFi.subnetMask());
    Serial.print("    Gateway:\t");  Serial.println(WiFi.gatewayIP());
  }

  /*
  //NTP
  if (wifi_con == true){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("\nConfiguring NTP " + String(ntpServer));
    unsigned timeout = 5000;
    unsigned start = millis();
    while (!time(nullptr))
    {
      Serial.print(".");
      delay(1000);
    }
    delay(1000);
    //Serial.println("Time...");
    
    }
    else{
      Serial.println("dateTime NOT valid");
    }
  */
      
  RtcDateTime currentTime = rtcObject.GetDateTime();
  String HH = String(currentTime.Hour());
  String MM = String(currentTime.Minute());
  String SS = String(currentTime.Second());
  String dd = String(currentTime.Day());
  String mm = String(currentTime.Month());
  String yyyy = String(currentTime.Year());
  Serial.println("- RTC time: " + yyyy+"/"+mm+"/"+dd+"_"+HH+":"+MM+":"+SS);

  //Alarms
  Serial.println("- Configuring alarms ...");
  Serial.println("    read_sensors:\t" + String(time_read_sensors) + " sec");
  Serial.println("    write_sd:\t\t" + String(time_write_sd) + " sec");
  reader.attach(time_read_sensors, read_sensors);
  writter.attach(time_write_sd, write_sd);
    
  Serial.println("Initalization complete!");
  Serial.println("##############################################");
  delay(2000);  
  display.clearDisplay(); //for Clearing the display
  display.drawBitmap(0, 0, myBitmap, 128, 64, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.display();
  //help();
  delay(5000);
}

////////////////////////////////////////
/* 
 *  LOOP
 */
////////////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:
  /*
  String newStr = Serial.readStringUntil('\n');
  if (newStr!="") {
    str = newStr;
    Serial.println("Input from serial interface received: " + str);
  }
  //"set clock HH:MM:SS DD-MM-YYYY"
  if (str == "clock"){
    str="";
    help();
  }
  */
  if (wifi_con==true){
    sync_rtc(); /*<----------------- descomentar per sincronitzar NTP->RTC*/
  }
  //read_sensor();
  /*
  if (sd_enabled == true){
    write_sd();
  }
  */
  writeOLED();
  //print_time();
  //sprintLocalTime(); 
  display.println("------------------");
  delay(500);
}

////////////////////////////////////////
/* 
 *  Functions
 */
////////////////////////////////////////

void print_time(){
  RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC
 
  char str[20];   //declare a string as an array of chars
 
  sprintf(str, "%d/%d/%d %d:%d:%d",     //%d allows to print an integer to the string
          currentTime.Year(),   //get year method
          currentTime.Month(),  //get month method
          currentTime.Day(),    //get day method
          currentTime.Hour(),   //get hour method
          currentTime.Minute(), //get minute method
          currentTime.Second()  //get second method
         );
 
  Serial.println(str);
}
void read_sensors(){
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  //altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature) || isnan(pressure) || isnan(altitude)) {
    Serial.println("Failed to read from BME sensor!");
    //return;
  }
  
  Serial.println("Temperature: " + String(temperature) + " (C), Humidity: " + String(humidity) + " (%), Pressure: " + String(pressure) + " (mBar)");
}

void writeOLED(){
  //Serial.println("Writing OLED display ...");
  display.clearDisplay();  /* Clear display */
  char temp[5];
  char hum[5];

  dtostrf(temperature,5,2,temp);
  dtostrf(humidity,5,2,hum);

  display.setTextSize(2);
  drawStr(10, 10, "T");
  display.setTextSize(1);
  drawStr(24, 16, "=");
  display.setTextSize(2);
  //drawStr(34, 10, temp);
  display.setCursor(34, 10);  /* Set x,y coordinates */
  display.println(temperature,2);
  display.setTextSize(1);
  drawStr(98, 16, "(C)");
  
  display.setTextSize(2);
  drawStr(10, 30, "H");
  display.setTextSize(1);
  drawStr(24, 36, "=");
  display.setTextSize(2);
  drawStr(34, 30, hum);
  display.setTextSize(1);
  drawStr(98, 36, "(%)");

  RtcDateTime currentTime = rtcObject.GetDateTime();
    String HH;
    String MM;
    String SS;
    
  if (currentTime.Hour()<10){
    HH = "0" + String(currentTime.Hour());
  }
  else{
    HH = String(currentTime.Hour());
  }
  if (currentTime.Minute()<10){
    MM = "0" + String(currentTime.Minute());
  }
  else{
    MM = String(currentTime.Minute());
  }
  if (currentTime.Second()<10){
    SS ="0" + String(currentTime.Second());
  }
  else{
    SS = String(currentTime.Second());
  }

  display.setTextSize(1);
  //drawStr(10, 54, "HH:MM:SS");
  display.setCursor(10, 54);
  display.println(HH+":"+MM+":"+SS);

  if (wifi_con==true){
    display.setTextSize(1);
    drawStr(68, 54, "Wifi");
  }
  if (wifi_con==true && sd_enabled==true && write_to_storage==true){
    display.setTextSize(1);
    drawStr(92, 54, "/"); 
  }
  if (sd_enabled==true && write_to_storage==true){
    display.setTextSize(1);
    drawStr(92, 54, " SD");
  }

  display.display();
}

void drawStr(uint8_t x, uint8_t y, char* str){
  display.setCursor(x, y);  /* Set x,y coordinates */
  display.println(str);
}

void help(){
  Serial.println("----------------------------------------------");
  Serial.println("Accepted commands:");
  Serial.println("  set clock HH:MM:SS DD-MM-YYYY");
  Serial.println("----------------------------------------------");
}

void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,80," %d %B %Y %H:%M:%S ",timeinfo);
  //struct tm timeinfo;
  //time_t now = time(nullptr);
  Serial.println(buffer);
  //Serial.print(ctime(&now));
  //Serial.print(&timeinfo, " %d %B %Y %H:%M:%S ");
}

void sync_rtc(){
  /**************************************/
  /* Time synchronization NTP --> RTC   */
  /**************************************/
  if (wifi_con == true){
    Serial.println("Synchronizing NTP time to RTC DS3212");
    // first parameter: Time zone in floating point (for India); second parameter: 1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment; (contributed by viewwer, not tested by me)
    dateTime = NTPch.getNTPtime(1.0, 1);
  
    // check dateTime.valid before using the returned time
    // Use "setSendInterval" or "setRecvTimeout" if required
    if(dateTime.valid){
      NTPch.printDateTime(dateTime);
  
      byte actualHour = dateTime.hour;
      byte actualMinute = dateTime.minute;
      byte actualSecond = dateTime.second;
      int actualYear = dateTime.year;
      byte actualMonth = dateTime.month;
      byte actualDay =dateTime.day;
      byte actualdayofWeek = dateTime.dayofWeek;
    
      Serial.println("NTP time: "+ String(actualHour)+":"+String(actualMinute)+":"+String(actualSecond)+" "+String(actualDay)+"-"+String(actualMonth)+"-"+String(actualYear));
      RtcDateTime currentTime = RtcDateTime(actualYear,actualMonth,actualDay,actualHour,actualMinute,actualSecond); //define date and time object
      //RtcDateTime currentTime = RtcDateTime(actualDay,String(actualYear).substring(2,4).toInt(),actualMonth,actualHour,actualMinute,actualSecond); //define date and time object
      rtcObject.SetDateTime(currentTime);
    }
  }
}

void write_sd(){
  if (sd_enabled == true){
    File dataFile = SD.open("LOG.txt", FILE_WRITE);
    if (dataFile) {
      //Serial.println("LOG.txt file opened successfully");
      RtcDateTime currentTime = rtcObject.GetDateTime();
      
  
      String HH;
      String MM;
      String SS;
      String dd;
      String mm;
      String yyyy;
      
      if (currentTime.Hour()<10){
        HH = "0" + String(currentTime.Hour());
      }
      else{
        HH = String(currentTime.Hour());
      }
      if (currentTime.Minute()<10){
        MM = "0" + String(currentTime.Minute());
      }
      else{
        MM = String(currentTime.Minute());
      }
      if (currentTime.Second()<10){
        SS ="0" + String(currentTime.Second());
      }
      else{
        SS = String(currentTime.Second());
      }
      if (currentTime.Day()<10){
        dd = "0" + String(currentTime.Day());
      }
      else{
       dd = String(currentTime.Day());
      }
      if (currentTime.Month()<10){
        mm = "0" + String(currentTime.Month());
      }
      else{
        mm = String(currentTime.Month());
      }
      yyyy = String(currentTime.Year());
      
          
      /*
      String HH = String(currentTime.Hour());
      String MM = String(currentTime.Minute());
      String SS = String(currentTime.Second());
      String dd = String(currentTime.Day());
      String mm = String(currentTime.Month());
      String yyyy = String(currentTime.Year());
      */
  	  //Serial.println(yyyy+"/"+mm+"/"+dd+"_"+HH+":"+MM+":"+SS);
      dataFile.print(yyyy+"/"+mm+"/"+dd+"_"+HH+":"+MM+":"+SS+",");
      dataFile.println(String(temperature)+","+String(humidity)+","+String(pressure));
      dataFile.close();
      Serial.println("Data written to LOG.txt file successfully");
    }
    else {
      Serial.println("ERROR opening file LOG.txt");
    }
  }
}
void drawDot(int num){
  display.setCursor(0 + num, 46);  /* Set x,y coordinates */
  display.print(".");
  display.display();
}
