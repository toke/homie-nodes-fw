#include <Arduino.h>
#include <Homie.h>
#include "DHT.h"
#include "DHT_U.h"


#define FW_NAME "dht-node"
#define FW_VERSION "2.0.0"

#define DHTPIN D4

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const int TEMPERATURE_INTERVAL = 60;

unsigned long lastTemperatureSent = 0;


void setupHandler();
void loopHandler();


HomieNode temperatureNode("temperature", "temperature");
HomieNode heatindexNode("heatindex", "temperature");
HomieNode humidityNode("humidity", "humidity");
DHT dht(DHTPIN, DHTTYPE);

void setupHandler() {
  dht.begin();
  temperatureNode.setProperty("unit").send("°C");
  heatindexNode.setProperty("unit").send("°C");
  humidityNode.setProperty("unit").send("%");
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(humidity) || isnan(temperature)) {
      Serial << "Invalid DHT data." << endl;
      return;
    }

    float hic = dht.computeHeatIndex(temperature, humidity, false);

    Serial << "Temperature: " << temperature << " °C" << endl;
    Serial << "Heatindex: " << hic << " °C" << endl;
    Serial << "Humidity: " << humidity << " %" << endl;

    if (temperatureNode.setProperty("degrees").send(String(temperature))) {
      lastTemperatureSent = millis();
    } else {
      Serial << "Temperature sending failed" << endl;
    }
    if (humidityNode.setProperty("percent").send(String(humidity))) {
      lastTemperatureSent = millis();
    } else {
      Serial << "Humidity sending failed" << endl;
    }
    if (heatindexNode.setProperty("degrees").send(String(hic))) {
      lastTemperatureSent = millis();
    } else {
      Serial << "Heatindex sending failed" << endl;
    }

  }
}


void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  heatindexNode.advertise("unit");
  heatindexNode.advertise("degrees");
  humidityNode.advertise("unit");
  humidityNode.advertise("percent");

  Homie.setup();
}

void loop() {
  Homie.loop();
}
