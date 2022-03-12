#include "WiFi.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define ID "1"
#define TCA_ADDR 0x70
#define BME_ADDR 0x76

namespace bme {
    int delayTime = 1000;
    int bmePin[3] = {3, 5, 7};

    struct BmeData {
        float temp      = 0;
        float pressure  = 0;
        float humidity  = 0;
    };
}    /* namespace bme */

Adafruit_BME280 bme280;

namespace wifi {
    // const char* ssid     = "AndroidAP9D5C";
    const char* ssid     =  "Fidget";
    const char* password =  "vfrcbv9121";

    const char* host     = "192.168.1.76";
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
        int res = TcaSelect(bme::bmePin[i]);
        if (res == -1)
            Serial.println("TCA setup error");
        res = (int)bme280.begin();
        if (res == 0)
            Serial.println("BME setup error");
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

String CreateMsg(const bme::BmeData& bmeData) {
    String msg = "";
    msg += bmeData.temp;
    msg += '_';
    msg += bmeData.pressure;
    msg += '_';
    msg += bmeData.humidity;
    return msg;
}

void loop() {
    bme::BmeData bmeData;
    for (int i = 0; i < 3; ++i) {
        int res = TcaSelect(bme::bmePin[i]);
        if (res == -1)
            Serial.println("TCA setup error");

        bmeData.temp     += bme280.readTemperature();
        bmeData.pressure += (bme280.readPressure() / 100.0F);
        bmeData.humidity += bme280.readHumidity();
    }

    bmeData.temp     /= 3;
    bmeData.pressure /= 3;
    bmeData.humidity /= 3;

    Serial.print("Temperature = ");
    Serial.println(bmeData.temp);
    Serial.print("Pressure = ");
    Serial.println(bmeData.pressure);
    Serial.print("Humidity = ");
    Serial.println(bmeData.humidity);

    delay(bme::delayTime);

    WiFiClient client;
    if (!client.connect(wifi::host, wifi::port)) {
        Serial.println("connection failed");
        return;
    }

    client.print(ID);
    String msg = CreateMsg(bmeData);
    Serial.println(msg);
    client.print(msg);
}
