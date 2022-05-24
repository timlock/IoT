#include <M5Stack.h>
#include <TinyGPS++.h>
#include "utility/MPU6886.h"
#include <InfluxDbCloud.h>
#include <InfluxDbClient.h>
#include <WiFi.h>

using namespace std;

TinyGPSPlus gps;
HardwareSerial serialStream(2);
MPU6886 mpu;
char* blankLine = "                    ";
char* trennlinie = "-----------------------------------------------------";

struct PositionStamp {
  volatile uint8_t hours;
  volatile uint8_t minutes;
  volatile uint8_t seconds;
  volatile double lattitude;
  volatile double longitude;
};


struct PositionStamp stamp;
volatile bool positionSet = false;

char * ssid = "hotspotTim";
char * password = "k3Q491*1";
char * url = "https://eu-central-1-1.aws.cloud2.influxdata.com";
char * organisation = "hsos";
char * bucket = "iot-bucket";
char * token = "FESIyzQ8punR1imo7K6NvofJQ8q5Gy6fDOWzC8UThTNYCoGHCHKxkGNplFT1vXgAFHAK9a1-9f5_7A23rhnO4g==";
char * measurement = "SchluchLock";

InfluxDBClient  influxClient(url, organisation, bucket, token, InfluxDbCloud2CACert);

Point sensor(measurement);
bool influxConnected = false;

int dots = 0;

unsigned long lastConnectTime;

void setup() {
  M5.begin();
  M5.Power.begin();
  serialStream.begin(9600);
  Wire.begin();
  mpu.Init();
  attachInterrupt(GPIO_NUM_38, updatePosStamp, HIGH);
  WiFi.begin(ssid, password);
  lastConnectTime = millis();
  //sensor.addTag("SSID", WiFi.SSID());
}

void loop() {
  sensor.clearFields();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Road Condition Monitoring\n");
  int battery = M5.Power.getBatteryLevel();
  M5.Lcd.printf("Batterry: %d%s\n", battery, blankLine);
  printWiFiStatus();
  printInfluxStatus();
  M5.Lcd.println(trennlinie);
  printeTimeDate();
  M5.Lcd.println(trennlinie);
  printGps();
  M5.Lcd.println(trennlinie);
  printAccel();
  M5.Lcd.println(trennlinie);
  if (positionSet) {
    printPosStamp();
    M5.Lcd.println(trennlinie);
  }
  if (influxConnected) {
    Serial.println(influxClient.pointToLineProtocol(sensor));
    if (!influxClient.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(influxClient.getLastErrorMessage());
    }
  }
  smartDelay(1000);

}

void printGps() {
  int updated = gps.location.isUpdated();
  int valid = gps.location.isValid();
  M5.Lcd.printf("Location updated (%d) valid (%d)\n", updated, valid);
  uint32_t satelliteCount = gps.satellites.value();
  M5.Lcd.printf("Sats: %u%s\n", satelliteCount, blankLine);
  int32_t hdopValue = gps.hdop.value();
  M5.Lcd.printf("HDOP: %d%s\n", hdopValue, blankLine);
  double lattitude = 0;
  double longitude = 0;
  if (updated && valid) {
    lattitude = gps.location.lat();
    longitude = gps.location.lng();
    if (influxConnected) {
      sensor.addField("lattitude", gps.location.lat());
      sensor.addField("longitude", gps.location.lng());
    }
  }
  M5.Lcd.printf("Lattitude: %f%s\nLongitude: %f%s\n", lattitude, blankLine, longitude, blankLine);
}

void printeTimeDate() {
  uint8_t days = gps.date.day();
  uint8_t months = gps.date.month();
  uint16_t years = gps.date.year();
  M5.Lcd.printf("Date: %u/%u/%u%s\n", days, months, years, blankLine);
  String timeStamp = "";
  uint8_t hours = (gps.time.hour() + 2) % 24;
  uint8_t minutes = gps.time.minute();
  uint8_t seconds = gps.time.second();
  timeStamp += String(hours) + ":";
  timeStamp += String(minutes) + ":";
  timeStamp += String(seconds);
  M5.Lcd.printf("Time: %s%s\n", timeStamp.c_str(), blankLine);
}


void printAccel() {
  float accX;
  float accY;
  float accZ;
  mpu.getAccelData(&accX, &accY, &accZ);
  if (influxConnected) {
    sensor.addField("X-acceleration", accX);
    sensor.addField("Y-acceleration", accY);
    sensor.addField("Z-acceleration", accZ);
  }
  M5.Lcd.printf("X-acceleration: %5.2f%s\n", accX, blankLine);
  M5.Lcd.printf("Y-acceleration: %5.2f%s\n", accY, blankLine);
  M5.Lcd.printf("Z-acceleration: %5.2f%s\n", accZ, blankLine);
}


void updatePosStamp() {
  if (gps.location.isValid()) {
    stamp.hours = gps.time.hour();
    stamp.minutes = gps.time.minute();
    stamp.seconds = gps.time.second();
    stamp.lattitude = gps.location.lat();
    stamp.longitude = gps.location.lng();
    positionSet = true;
  } else {
    Serial.println("Kann keinen Positions Stempel setzen, die gelesen Koordinaten sind ungültig");
  }
}

String passedTimeSince() {
  String passedTime = String(gps.time.hour() - stamp.hours);
  passedTime += (":");
  passedTime += (gps.time.minute() - stamp.minutes);
  passedTime += (":");
  passedTime += (gps.time.second() - stamp.seconds);
  return passedTime;
}

void printPosStamp() {
  M5.Lcd.printf("Startposition:%s\nLattitude: %f%s\nLongitude: %f%s\n", blankLine, stamp.lattitude, blankLine, stamp.longitude, blankLine);
  String timePassed = passedTimeSince();
  M5.Lcd.printf("Dauer: %s%s\n", timePassed.c_str(), blankLine);
  unsigned long distance = TinyGPSPlus::distanceBetween(stamp.lattitude, stamp.longitude, gps.location.lat(), gps.location.lng());
  M5.Lcd.printf("Distanz zum Start: %d%s\n", distance, blankLine);
}

void printWiFiStatus() {
  bool wifiStatus = WiFi.status() == WL_CONNECTED;
  M5.Lcd.printf("WiFi Status: (%d)", wifiStatus);
  unsigned long currentTime = millis();
  if (!wifiStatus && (currentTime - lastConnectTime >= 10000)) { // Versucht alle 10 Sekunden eine Verbinung aufzubauen
    Serial.println("WiFi Verbindung verloren");
    WiFi.reconnect();
    int dots = 0;
    lastConnectTime = currentTime;
  }
  if (!wifiStatus ) {
    influxConnected = false;
    if (dots == 3) {
      dots = 0;
    }
    else {
      for (int i = 0; i <= dots; i++) {
        M5.Lcd.print(".");
      }
      dots++;
    }
  }
  M5.Lcd.print("     \n");
}

void printInfluxStatus() {
  if (WiFi.status() == WL_CONNECTED && !influxConnected) {
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nis.gov");
    if (influxClient.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(influxClient.getServerUrl());
      influxConnected = true;
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(influxClient.getLastErrorMessage());
    }
  }
  M5.Lcd.printf("InfluxDB Status: (%d)\n", influxConnected);
}


void smartDelay(unsigned long ms)
{
  bool dataRead = false;
  unsigned long start = millis();
  do
  {
    char c;
    while (serialStream.available()) {
      dataRead = true;
      c = serialStream.read();
      if (c == '$')
        Serial.println();
      Serial.print(c);
      gps.encode(c);
    }
  } while (millis() - start < ms);
  M5.Lcd.printf("Lese GPS-Daten (%d)", dataRead);
}
