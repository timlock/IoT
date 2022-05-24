#include <M5Stack.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

using namespace std;

const char* ssid = "DESKTOP-RNGNR2Q 0447";
const char* password = "k3Q491*1";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

uint16_t X_OFFSET = 160;
uint16_t Y_OFFSET = 120;
uint16_t R_OUTER = 70;
uint16_t R_INNER = 60;

float pos_x = 0;
float pos_y = 0;

int execTime = 0;
int startTime = 0;
int endTime = 0;

uint16_t H_POINTER_LENGTH = 30;
uint16_t M_POINTER_LENGTH = 50;
uint16_t S_POINTER_LENGTH = 40;
uint16_t RADIUS = 10;

bool analog = true;
bool clockDrawn = false;
void setup() {
  M5.begin();
  M5.Power.begin();
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 1000 );
    Serial.print ( "." );
  }
  Serial.print("\nConnected to WiFi\n");
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  attachInterrupt(GPIO_NUM_37, setAnalog, HIGH);
  attachInterrupt(GPIO_NUM_38, setDigital, HIGH);
}

void loop() {
  timeClient.update();
  String timeStamp = timeClient.getFormattedTime();
  Serial.printf("Time: %s\n", timeStamp.c_str());
  if (analog) {
    if (!clockDrawn)
      drawAnalogClock();
    int hours, minutes, seconds;
    sscanf(timeStamp.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
    drawTimeHands(hours, minutes, seconds);
  } else {
    if (clockDrawn) {
      M5.Lcd.clear(BLACK);
      clockDrawn = false;
    }
    drawDigital(timeStamp);
  }

  smartDelay(1000);
}

void smartDelay(unsigned long ms)
{
  bool dataRead = false;
  unsigned long start = millis();
  do
  {
    while (!timeClient.update()) {
      timeClient.forceUpdate();
    }
  } while (millis() - start < ms);
}

void setAnalog() {
  analog = true;
  Serial.println("Analog");
}
void setDigital() {
  analog = false;
  Serial.println("Digital");
}

void drawAnalogClock() {
  M5.Lcd.clear(BLACK);
  M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_OUTER, 0x5AFF);
  M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_INNER, 0x5AEB);
  float pos_x, pos_y, x, y;
  for (int i = 0; i < 360; i++) {
    pos_x = cos((i - 90) * 0.0174532925);
    pos_y = sin((i - 90) * 0.0174532925);
    x = pos_x * ((R_OUTER + R_INNER) / 2) + X_OFFSET;
    y = pos_y * ((R_OUTER + R_INNER) / 2) + Y_OFFSET;
    if (i % 90 == 0) {
      M5.Lcd.fillCircle(x, y, 5, 0xFFFF);
    }
  }
  clockDrawn = true;
}

void drawTimeHands(int hours, int minutes, int seconds) {
  Serial.printf("%d %d %d\n", hours, minutes, seconds);
  uint16_t h_sx = 120, h_sy = 120;
  uint16_t h_mx = 120, h_my = 120;
  uint16_t h_hx = 120, h_hy = 120;

  float sec_deg = seconds * 6;
  float min_deg = minutes * 6 + sec_deg * 0.01666667;
  float hou_deg = hours * 30 + min_deg * 0.0833333;
  float hou_x = cos((hou_deg - 90) * 0.0174532925);
  float hou_y = sin((hou_deg - 90) * 0.0174532925);
  float min_x = cos((min_deg - 90) * 0.0174532925);
  float min_y = sin((min_deg - 90) * 0.0174532925);
  float sec_x = cos((sec_deg - 90) * 0.0174532925);
  float sec_y = sin((sec_deg - 90) * 0.0174532925);
  int white = 0xFFFFFF;
  int red = 0xFF0000;
  h_hx = hou_x * H_POINTER_LENGTH + X_OFFSET;
  h_hy = hou_y * H_POINTER_LENGTH + Y_OFFSET;
  h_mx = min_x * M_POINTER_LENGTH + X_OFFSET;
  h_my = min_y * M_POINTER_LENGTH + Y_OFFSET;
  h_sx = sec_x * S_POINTER_LENGTH + X_OFFSET;
  h_sy = sec_y * S_POINTER_LENGTH + Y_OFFSET;
  M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_INNER, 0x5AEB);

  M5.Lcd.drawLine(h_hx, h_hy, X_OFFSET, Y_OFFSET, white);
  M5.Lcd.drawLine(h_mx, h_my, X_OFFSET, Y_OFFSET, white);
  M5.Lcd.drawLine(h_sx, h_sy, X_OFFSET, Y_OFFSET, red);
  M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, RADIUS, white);
}

void drawDigital(String timeStamp) {
  M5.Lcd.setTextSize(5);
  M5.Lcd.setCursor(20, 100);
  M5.Lcd.print(timeStamp);
}
