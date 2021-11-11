#include "WiFi.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

#define ID "0"
#define TCA_ADDR 0x70
#define BMP_ADDR 0x76

namespace bmp {
    int delayTime = 1000;
    int bmpPin[3] = {3, 5, 7};

    struct BmpData {
        float temp      = 0;
        float pressure  = 0;
    };
}    /* namespace bmp */

Adafruit_BMP280 bmp280;

namespace wifi {
    const char* ssid     = "Fidget";
    const char* password =  "vfrcbv9121";

    const char* host     = "192.168.1.89";
    uint16_t    port     = 1111;
}    /* namespace wifi */

int TcaSelect(uint8_t i2cBus) {
    if (i2cBus > 7)
        return -1;
    Wire.beginTransmission(TCA_ADDR);
    Wire.write(1 << i2cBus);
    Wire.endTransmission();
    return 0;
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    for (int i = 0; i < 3; ++i) {
        int res = TcaSelect(bmp::bmpPin[i]);
        if (res == -1)
            Serial.println("TCA setup error");
        res = (int)bmp280.begin();
        if (res == 0)
            Serial.println("BMP setup error");
    }

    WiFi.begin(wifi::ssid, wifi::password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
  
    Serial.println("Connected to the WiFi network");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

String CreateMsg(const bmp::BmpData& bmpData) {
    String msg = "";
    msg += bmpData.temp;
    msg += '_';
    msg += bmpData.pressure;
    return msg;
}

void loop() {
    bmp::BmpData bmpData;
    for (int i = 0; i < 3; ++i) {
        int res = TcaSelect(bmp::bmpPin[i]);
        if (res == -1)
            Serial.println("TCA setup error");

        bmpData.temp += bmp280.readTemperature();
        bmpData.pressure += (bmp280.readPressure() / 100.0F);
    }

    bmpData.temp     /= 3;
    bmpData.pressure /= 3;

    Serial.print("Temperature = ");
    Serial.println(bmpData.temp);
    Serial.print("Pressure = ");
    Serial.println(bmpData.pressure);

    delay(bmp::delayTime);

    WiFiClient client;
    if (!client.connect(wifi::host, wifi::port)) {
        Serial.println("connection failed");
        return;
    }

    client.print(ID);
    String msg = CreateMsg(bmpData);
    Serial.println(msg);
    client.print(msg);
}
