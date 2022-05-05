#include "WiFi.h"
#include <SPI.h>
#include "MHZ19.h"

#define ID "2"
 
namespace mh {
    int delayTime = 1000;

    struct MhData {
        int ppm = 0;
    };
}    /* namespace mh */

const int rx_pin = 16;
const int tx_pin = 17;

MHZ19 *mhz19_uart = new MHZ19(rx_pin,tx_pin);

namespace wifi {
    const char* ssid     = "AndroidAP9D5C";
//    const char* ssid     =  "Fidget";
    const char* password =  "vfrcbv9121";

    const char* host     = "192.168.170.96";
    uint16_t    port     = 1111;
}    /* namespace wifi */

void setup() {
    Serial.begin(115200);
    mhz19_uart->begin(rx_pin, tx_pin);
    mhz19_uart->setAutoCalibration(false);
    delay(3000);
    while (true) {
      int mhState = mhz19_uart->getStatus();
    Serial.print("MH-Z19 now warming up...  status:");
    Serial.println(mhState);
       int mhWarm = mhz19_uart->isWarming();
    if (mhWarm)
      break;
    delay(5000);
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

String CreateMsg(const mh::MhData& mhData) {
    String msg = "";
    msg += mhData.ppm;
    return msg;
}

void loop() {
    measurement_t m = mhz19_uart->getMeasurement();
    
    mh::MhData mhData;
    mhData.ppm = m.co2_ppm;

    WiFiClient client;
    if (!client.connect(wifi::host, wifi::port)) {
        Serial.println("connection failed");
        return;
    }

    client.print(ID);
    String msg = CreateMsg(mhData);
    Serial.println(msg);
    client.print(msg);
    delay(mh::delayTime);
}
