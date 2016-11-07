#include <Homie.h>
#include "DHT.h"
#include "DHT_U.h"


#define FW_NAME "dht-node"
#define FW_VERSION "1.0.7"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

#define DHTPIN D4

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const int TEMPERATURE_INTERVAL = 60;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "temperature");
HomieNode heatindexNode("heatindex", "temperature");
HomieNode humidityNode("humidity", "humidity");
DHT dht(DHTPIN, DHTTYPE);

void setupHandler() {
  dht.begin();
  Homie.setNodeProperty(temperatureNode, "unit", "c", true);
  Homie.setNodeProperty(heatindexNode, "unit", "c", true);
  Homie.setNodeProperty(humidityNode, "unit", "%", true);
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.print("Invalid DHT data.");
      return;
    }

    float hic = dht.computeHeatIndex(temperature, humidity, false);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    if (Homie.setNodeProperty(temperatureNode, "degrees", String(temperature), true)) {
      lastTemperatureSent = millis();
    } else {
      Serial.println("Temperature sending failed");
    }
    if (Homie.setNodeProperty(humidityNode, "percent", String(humidity), true)) {
      lastTemperatureSent = millis();
    } else {
      Serial.println("Humidity sending failed");
    }
    if (Homie.setNodeProperty(heatindexNode, "degrees", String(hic), true)) {
      lastTemperatureSent = millis();
    } else {
      Serial.println("Heatindex sending failed");
    }

  }
}


void setup() {
  //Homie.setBrand("MyIoTSystem");
  Homie.setFirmware(FW_NAME, FW_VERSION);
  Homie.registerNode(temperatureNode);
  Homie.registerNode(heatindexNode);
  Homie.registerNode(humidityNode);
  Homie.setSetupFunction(setupHandler);
  //Homie.setResetFunction(resetFunction);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
