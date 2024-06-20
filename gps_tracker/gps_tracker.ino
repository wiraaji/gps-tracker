//This code is based on https://iotdesignpro.com/projects/nodemcu-esp8266-gps-module-interfacing-to-display-latitude-and-longitude
//Hardware serial port is using insted of software serial port

// --------Wiring-----------------
//ESP32S 		 		  GPS6MV2
//GND 			 <---> GND
//GPIO16(RX2) <---> TX
//GPIO17(TX2) <---> RX
//3V3				 <---> VCC

//Ver 0.2
//2019-11-11
#include <TinyGPS++.h>  // library for GPS module
#include <WiFi.h>
#include <HardwareSerial.h>
TinyGPSPlus gps;  // The TinyGPS++ object
HardwareSerial ss(1);
const char* ssid = "Tes";           //ssid of your wifi
const char* password = "1234567890";  //password of your wifi
float latitude, longitude;
int year, month, date, hour, minute, second;
String date_str, time_str, lat_str, lng_str;
int pm;
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);            //connecting to wifi
  while (WiFi.status() != WL_CONNECTED)  // while wifi not connected
  {
    delay(500);
    Serial.print(".");  //print "...."
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());  // Print the IP address
}


void loop() {
  while (ss.available() > 0)  //while data is available
  {
    yield();                    //to prevent Soft WDT reset in esp8266 while long loop
    if (gps.encode(ss.read()))  //read gps data
    {
      yield();                     //to prevent Soft WDT reset in esp8266 while long loop
      if (gps.location.isValid())  //check whether gps location is valid
      {
        latitude = gps.location.lat();
        lat_str = String(latitude, 6);  // latitude location is stored in a string
        longitude = gps.location.lng();
        lng_str = String(longitude, 6);  //longitude location is stored in a string
      }
      yield();                 //to prevent Soft WDT reset in esp8266 while long loop
      if (gps.date.isValid())  //check whether gps date is valid
      {
        date_str = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        if (date < 10)
          date_str = '0';
        date_str += String(date);  // values of date,month and year are stored in a string
        date_str += " / ";

        if (month < 10)
          date_str += '0';
        date_str += String(month);  // values of date,month and year are stored in a string
        date_str += " / ";

        if (year < 10)
          date_str += '0';
        date_str += String(year);  // values of date,month and year are stored in a string
      }
      yield();                 //to prevent Soft WDT reset in esp8266 while long loop
      if (gps.time.isValid())  //check whether gps time is valid
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();

        minute = (minute + 0);  // converting to JST +0 minutes
        if (minute > 59) {
          minute = minute - 60;
          hour = hour + 1;
        }

        hour = (hour + 9);  // converting to JST +9 hours
        if (hour > 23)
          hour = hour - 24;

        if (hour >= 12)  // checking whether AM or PM
          pm = 1;
        else
          pm = 0;

        hour = hour % 12;
        if (hour < 10)
          time_str = '0';
        time_str += String(hour);  //values of hour,minute and time are stored in a string
        time_str += " : ";

        if (minute < 10)
          time_str += '0';
        time_str += String(minute);  //values of hour,minute and time are stored in a string
        time_str += " : ";

        if (second < 10)
          time_str += '0';
        time_str += String(second);  //values of hour,minute and time are stored in a string

        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
      }
    }
  }
  yield();                                 //to prevent Soft WDT reset in esp8266 while long loop
  WiFiClient client = server.available();  // Check if a client has connected
  yield();                                 //to prevent Soft WDT reset in esp8266 while long loop
  if (!client) {
    return;
  }
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>GPS DATA</title> <style>";
  s += "a:link {background-color: YELLOW;text-decoration: none;}";
  s += "table, th, td </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER> GPS DATA</h1>";
  s += "<p ALIGN=CENTER style="
       "font-size:150%;"
       "";
  s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER >";
  s += lat_str;
  s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
  s += lng_str;
  s += "</td> </tr> <tr>  <th>Date</th> <td ALIGN=CENTER >";
  s += date_str;
  s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
  s += time_str;
  s += "</td></tr> <tr> <th>OPEN GMAPS</th> <td ALIGN=CENTER >";
  s += "<a href=\"https://maps.google.com/maps?q=" + lat_str + "," + lng_str + "\">HERE</a>";
  s += "</td>  </tr> </table> ";

  s += "</body> </html>";
  yield();          //to prevent Soft WDT reset in esp8266 while long loop
  client.print(s);  // all the values are send to the webpage
  delay(100);
}