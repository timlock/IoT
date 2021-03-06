#include <NTPClient.h>
#include <WiFi.h>
#include <M5Stack.h>
#include <WiFiUdp.h>

const char *ssid = "DESKTOP-KT6R0Q7 9768";
const char *password = "123456781";

uint16_t recontimer = 0;


WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP);

String formattedDate;
String timeStamp;
String sec = " ";
String mnt = " ";
String hour = " ";
bool state = true;
bool state2 = false;
bool fon = false;
bool aon = false;
bool beep1 = false;
bool beep2 = false;
bool beep3 = false;
bool con = false;
bool nocon = false;

uint8_t X_OFFSET = 160;
uint8_t Y_OFFSET = 120;
uint8_t R_OUTER = 70;
uint8_t R_INNER = 60;

uint8_t coord[8];
uint8_t count = 0;

float pos_x = 0;
float pos_y = 0;

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;

uint8_t s = 0;
uint8_t m = 0;
uint8_t h = 0;

int execTime = 0;
uint8_t h_sx = 120, h_sy = 120;
uint8_t h_mx = 120, h_my = 120;
uint8_t h_hx = 120, h_hy = 120;

uint8_t H_POINTER_LENGTH = 30;
uint8_t M_POINTER_LENGTH = 50;
uint8_t S_POINTER_LENGTH = 40;
uint8_t RADIUS = 10;

void setup() {


    M5.begin();
    M5.Power.begin();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Initialisiere WiFi...\n");
        delay(1000);
    }
    Serial.print("WiFi initialisiert\n");

    timeClient.begin();                                           // Start der NTPserver Verbindung
    timeClient.setTimeOffset(7200);                               // Anpassen der Zeitzone


    // Interrupts initialisieren
    attachInterrupt(GPIO_NUM_38, swStateR, RISING);
    attachInterrupt(GPIO_NUM_37, battery, FALLING);

    M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_OUTER, 0xF81F);               // Zeichnen der Uhr
    M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_INNER, 0x000F);

    for (int i = 0; i < 360; i++) {
        pos_x = cos((i - 90) * 0.0174532925);
        pos_y = sin((i - 90) * 0.0174532925);
        uint16_t x = pos_x * ((R_OUTER + R_INNER) / 2) + X_OFFSET;
        uint16_t y = pos_y * ((R_OUTER + R_INNER) / 2) + Y_OFFSET;
        if (i % 90 == 0) {
            M5.Lcd.fillCircle(x, y, 5, 0xFFFF);
            coord[count] = x;
            count++;
            coord[count] = y;
            count++;

        }
    }
}


void loop() {
    M5.update();
    uint16_t ret;
    if (beep3) {
        M5.Speaker.tone(300, 50);              //Ton bei dr??cken von Button B
        beep3 = !beep3;
    }
    if (aon) {
        if (beep1) {
            M5.Speaker.tone(900, 100);                  //Ton bei gedr??ckthalten
            beep1 = !beep1;
        }
        ret = analogRead(36);

        //Serial.print(ret);
        //Serial.print(" / ");                        // Ausgabe Sensor Werte

        if (ret < 1200) {
            M5.Lcd.setBrightness(255);
        } else if (ret < 1600) {
            M5.Lcd.setBrightness(200);                    // Abfrage der Helligkeitslevel
        } else if (ret < 2000) {
            M5.Lcd.setBrightness(178);
        } else if (ret < 2400) {
            M5.Lcd.setBrightness(120);
        } else if (ret < 2800) {
            M5.Lcd.setBrightness(70);
        } else if (ret < 3200) {
            M5.Lcd.setBrightness(30);
        }
    } else {
        M5.Lcd.setBrightness(255);
    }
    if (execTime < millis()) {                        // 1s Verz??gerung
        if (WiFi.status() == WL_CONNECTED) {
            con = true;
            while (!timeClient.update()) {             //Falsche Daten vermeiden
                timeClient.forceUpdate();
            }
        } else if ((WiFi.status() != WL_CONNECTED) && recontimer >= 7) {
            Serial.print("Reconnecting...");
            WiFi.disconnect(true);
            WiFi.begin(ssid, password);
            recontimer = 0;
        } else {
            recontimer++;
        }
        formattedDate = timeClient.getFormattedDate();      // Format 2018-05-28T16:00:13Z
        Serial.print(formattedDate);
        Serial.print("\n");
        int splitT = formattedDate.indexOf("T");
        timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1); // Abspalten der Uhrzeit
        if (state) {                      // Digital oder Analog
            if (beep2) {
                M5.Speaker.tone(900, 100);              //Ton bei dr??cken von Button B
                beep2 = !beep2;
            }
            if (state2) {                    // Erste ausf??hrung Analog ?
                
                hour = formattedDate.substring(splitT + 1, splitT + 3);
                mnt = formattedDate.substring(splitT + 4, splitT + 6);           // Aufspalten der Stunden/Minuten/Sekunden
                sec = formattedDate.substring(splitT + 7, splitT + 9);
                hours = hour.toInt();
                minutes = mnt.toInt();
                seconds = sec.toInt();
                double sec_deg = seconds * 6;
                double min_deg = minutes * 6 + sec_deg * 0.01666667;
                double hou_deg = hours * 30 + min_deg * 0.0833333;
                double hou_x = cos((hou_deg - 90) * 0.0174532925); // Koordinaten f??r Zeiger berechnen
                double hou_y = sin((hou_deg - 90) * 0.0174532925);
                double min_x = cos((min_deg - 90) * 0.0174532925);
                double min_y = sin((min_deg - 90) * 0.0174532925);
                double sec_x = cos((sec_deg - 90) * 0.0174532925);
                double sec_y = sin((sec_deg - 90) * 0.0174532925);
                int COLOR = 0xFAFC;

                h_hx = hou_x * H_POINTER_LENGTH + X_OFFSET; // Zeiger Positionen Aktualisieren
                h_hy = hou_y * H_POINTER_LENGTH + Y_OFFSET;
                h_mx = min_x * M_POINTER_LENGTH + X_OFFSET;
                h_my = min_y * M_POINTER_LENGTH + Y_OFFSET;
                h_sx = sec_x * S_POINTER_LENGTH + X_OFFSET;
                h_sy = sec_y * S_POINTER_LENGTH + Y_OFFSET;

                M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_INNER, 0x5AEB);  // Zifferblatt L??schen

                M5.Lcd.drawLine(h_hx, h_hy, X_OFFSET, Y_OFFSET, COLOR);  // Zeiger neu schreiben mit Aktuellen Werten
                M5.Lcd.drawLine(h_mx, h_my, X_OFFSET, Y_OFFSET, COLOR);
                M5.Lcd.drawLine(h_sx, h_sy, X_OFFSET, Y_OFFSET, COLOR);
                M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, RADIUS, COLOR);
            } else {
                M5.Lcd.clear(BLACK);
                M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_OUTER, 0xF81F);               //Neu Zeichnen der Uhr
                M5.Lcd.fillCircle(X_OFFSET, Y_OFFSET, R_INNER, 0x000F);
                state2 = true;
                M5.Lcd.fillCircle(coord[0], coord[1], 5, 0xFFFF);
                M5.Lcd.fillCircle(coord[2], coord[3], 5, 0xFFFF);
                M5.Lcd.fillCircle(coord[4], coord[5], 5, 0xFFFF);
                M5.Lcd.fillCircle(coord[6], coord[7], 5, 0xFFFF);
            }
        } else {
            if (beep2) {
                M5.Speaker.tone(900, 100);                         //Ton bei dr??cken von Button B
                beep2 = !beep2;
            }
            state2 = false;
            M5.Lcd.clear(BLACK);
            M5.Lcd.setTextColor(WHITE);
            M5.Lcd.setTextSize(5);                             // Digital Uhr Modus
            M5.Lcd.setCursor(10, 120);
            M5.Lcd.print(timeStamp);
        }

        //M5.Lcd.clear(BLACK);
        M5.Lcd.setTextSize(2);                             // Anzeigen des Batteriestandes
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.fillRect(0, 0, 320, 20, BLACK);

        if (fon) {
            int8_t battery = M5.Power.getBatteryLevel();
            if (battery >= 75) {               //Batterie Level pr??fen
                M5.Lcd.setTextColor(0x03E0);
                M5.Lcd.print(battery);
            } else if (battery >= 50) {
                M5.Lcd.setTextColor(0xFFE0);
                M5.Lcd.print(battery);
            } else if (battery >= 25) {
                M5.Lcd.setTextColor(0xFFE0);
                M5.Lcd.print(battery);
            } else if (battery >= 0) {
                M5.Lcd.setTextColor(0xFDA0);
                M5.Lcd.print(battery);
            } else if (battery == -1) {
                Serial.print("Fehler");
            }
            M5.Lcd.print("%");
        }
        execTime = millis() + 1000;
    }
}


void swStateR() {                    // Umschalten zwischen Digital und Analog

    if (M5.BtnB.pressedFor(2000)) {
        aon = !aon;
        beep1 = !beep1;
    } else {
        state = !state;
        beep2 = !beep2;
    }

}

void battery() {
    fon = !fon;
    beep3 = !beep3;
}
