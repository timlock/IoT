#include <HTTP_Method.h>
#include <Uri.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>

#define MAX_MOISTURE 4095
#define MIN_MOISTURE 1450

int pin         =  15;
int numPixels   = 10;
int pixelFormat = NEO_GRB + NEO_KHZ800;

Adafruit_NeoPixel pixels(numPixels, pin, pixelFormat);

const char* ssid = "MoistureServer";
const char* password = "12341234";
//
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

void setup() {
  M5.begin();
  M5.Power.begin();
  M5.lcd.setTextSize(2);
  M5.Lcd.println("Earth Moisture");
  pinMode(26, INPUT);
  pixels.begin();
    if (true) {
      WiFi.softAP(ssid, password);
      WiFi.softAPConfig(local_ip, gateway, subnet);
      Serial.print("Sensor AP ready! Use 'http://");
      Serial.print(WiFi.softAPIP());
      Serial.println("' to connect");
  
    }
    server.on("/", handle_OnConnect);
    server.begin();
  //WiFi.mode(WIFI_AP);
//  WiFi.softAP(ssid, password);
//  Serial.println(WiFi.softAPIP());
//  Serial.println(WiFi.localIP());
//  Serial.print("\n");
//  server.on("/", handleRoot);
//
//  server.on("/inline", []() {
//    server.send(200, "text/plain", "this works as well");
//  });
//
//  server.onNotFound(handleNotFound);
//
//  server.begin();
//  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  pixels.clear();
  M5.update();
  M5.Lcd.setCursor(0, 80);
  float moisture = printMoistureLevel();
  int battery = printBattery();
  activateLeds(moisture, battery);

  delay(1000);

  //  uint16_t ret;
  //  ret = analogRead(36);
  //
  //  //Serial.print(ret);
  //  //Serial.print(" / ");                        // Ausgabe Sensor Werte
  //  server.handleClient();

}

void handleRoot() {
  
  server.send(200, "text/plain", "hello from esp32!");
  
}

void handleNotFound() {
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  
}

float printMoistureLevel() {
  int analog = analogRead(36);
  float moisture = 100 - ((analog - MIN_MOISTURE) * 100.f) / (MAX_MOISTURE - MIN_MOISTURE);
  Serial.printf("Analog: %d relativer Wert: %f\n", analog, moisture);
  M5.Lcd.printf("Moisutre level: %3.2f%%  \n", moisture);
  return moisture;
}

int printBattery() {
  int battery = M5.Power.getBatteryLevel();
  M5.Lcd.printf("Batterry: %d%%  \n", battery);
  return battery;
}
void activateLeds(float moisture, int battery) {
  static bool blinked = false;
  battery /= 20;
  for (int i = 0; i < 5; i++) {
    if ( i < battery) {
      pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    } else {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
  }
  if (moisture < 20.f && !blinked) {
    blinked = true;
    for (int i = 5; i < numPixels; i++) { // For each pixel...
      pixels.setPixelColor(i, pixels.Color(250, 0, 0));
    }
  }
  else {
    blinked = false;
  }

  pixels.show();
}
  void handle_OnConnect() {
    String ptr = "<!DOCTYPE html>";
    ptr += "<html>";
    ptr += "<head><title>HelloWorld -Title</title></head>";
    ptr += "<body><p>Hello World</p></body>";
    ptr += "<meta http-equiv='refresh' content='10'>>";
    ptr += "</html>";
    server.send(200, "text/html", ptr);
  }
