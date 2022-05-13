#include <M5Stack.h>
#include <TinyGPS++.h>
//#include "utility/MPU9250.h"

using namespace std;

String createTimeStamp();

TinyGPSPlus gps;
HardwareSerial serialStream(2);
//MPU9250 IMU;

void setup() {
  M5.begin();
  M5.Power.begin();
  serialStream.begin(9600);

  Wire.begin();
  //IMU.initMPU9250(); // this line must be after Wire.begin()
  M5.Lcd.println("Road Condition Monitoring\n");
}

void loop() {

  M5.Lcd.setCursor(0, 20);
    uint32_t satelliteCount = gps.satellites.value();
    int32_t hdopValue = gps.hdop.value();
    double lattitude = gps.location.lat();
    double longitude = gps.location.lng();
    uint8_t days = gps.date.day();
    uint8_t months = gps.date.month();
    uint16_t years = gps.date.year();
    String timeStamp = createTimeStamp();
    M5.Lcd.println("Sats|HDOP|Latitude  |Longitude  |Date     |Time\n");
    M5.Lcd.printf("%u   |%d|%f  |%f   |%u/%u/%u|%s\n\n",satelliteCount, hdopValue, lattitude, longitude, days, months, years, timeStamp.c_str());

    M5.Lcd.println("x-Acceleration y-Acceleration z-Acceleration");
  if(serialStream.available()){
   // M5.Lcd.clear(BLACK);
  }
    while(serialStream.available()>0){
      char c = serialStream.read();
      if(c =='$'){
        Serial.printf("\n");
      }
      gps.encode(c);
      Serial.printf("%s",&c);
    }
    //delay(1000);
}

String createTimeStamp(){
    String timeStamp = "";
    uint8_t hours = (gps.time.hour() + 2) % 24;
    uint8_t minutes = gps.time.minute();
    uint8_t seconds = gps.time.second();
    //if(hours < 10) timeStamp += "0";
    timeStamp += String(hours) + ":";
    //if(minutes < 10) timeStamp += "0";
    timeStamp += String(minutes) + ":";
    //if(seconds < 10) timeStamp += "0";
    timeStamp += String(seconds);
    return timeStamp;
}
