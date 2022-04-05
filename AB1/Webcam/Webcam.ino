#include <WiFi.h>
#include <M5Stack.h>
#include <HTTPClient.h>
#include <stdlib.h>


const char* ssid = "M5Stack-Kamera";
const char* password = "12341234";
const char* url = "http://192.168.4.1/capture";

void setup() {
  M5.begin();
  M5.Power.begin();
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
      Serial.print("Initialisiere WiFi...\n");
      delay(1000);
  }
  Serial.print("WiFi initialisiert\n");
}

void loop() {
  while(WiFi.status() != WL_CONNECTED){
      Serial.print("Verbindung zu WiFi verloren...\n");
      delay(1000);
  }
    HTTPClient http;
    http.begin(url);
    int response = http.GET();
    if(response <= 0){
      Serial.printf("GET() fehlgeschlagen %s\n", http.errorToString(response).c_str());
    }else {
      int fileSize = http.getSize(); // Gesamtgröße des Bildes
      Serial.printf("File size: %d\n",fileSize);
      WiFiClient * stream = http.getStreamPtr(); // Pointer auf den Stream zum auslesen des Bildes
      uint8_t * buffer = (uint8_t*)malloc(fileSize); // Dynamischer Speicher zum speichern des Bildes
      uint8_t * p = buffer; // Pointer auf die Stelle im Buffer auf die als nächstes beschrieben wird
      size_t bytesRead = 0; 
      size_t totalBytesRead = 0;
      while(totalBytesRead < fileSize){
        size_t availableBytes = stream->available(); // Anzahl an Bytes die momentan aus dem Stream gelesen werden können
        Serial.printf("Verfuegbare Bytes %d\n",availableBytes);
        bytesRead = stream->readBytes(p, availableBytes);
        if(bytesRead > 0){
          p += bytesRead; 
          totalBytesRead += bytesRead;
        }
        Serial.printf("Zueltzt gelesene Bytes: %d, insgesamt gelesene Bytes: %d\n",bytesRead, totalBytesRead);
      }
      Serial.print("Zeichne Jpeg\n");  
      M5.Lcd.drawJpg(buffer,fileSize);
      free(buffer);
    }
      //delay(500);
}
