#include <Homie.h>
#include "DHT.h"

#define DHTPIN 14

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const int TEMPERATURE_INTERVAL = 300;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "temperature");
HomieNode heatindexNode("heatindex", "heatindex");
HomieNode humidityNode("humidity", "humidity");
DHT dht(DHTPIN, DHTTYPE);

void setupHandler() {
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
    } else {
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
}

void setup() {
  dht.begin();
  Homie.setFirmware("dht-node", "1.0.1");
  Homie.registerNode(temperatureNode);
  Homie.registerNode(heatindexNode);
  Homie.registerNode(humidityNode);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
