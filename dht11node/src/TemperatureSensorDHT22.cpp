#include <Arduino.h>
#include <Homie.h>
#include "DHT.h"
#include "DHT_U.h"


#define FW_NAME "dht-node"
#define FW_VERSION "2.0.1"

#define DHTPIN D4
#define PIN_BUTTON1 0

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const int TEMPERATURE_INTERVAL = 60;

unsigned long lastTemperatureSent = 0;


void setupHandler();
void loopHandler();
bool broadcastHandler(const String& level, const String& value);

Bounce debouncer = Bounce(); // Bounce is built into Homie, so you can use it without including it first


int lastButton1Value = -1;

HomieNode temperatureNode("temperature", "temperature");
HomieNode heatindexNode("heatindex", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode buttonNode1("button1", "button");

DHT dht(DHTPIN, DHTTYPE);

void setupHandler() {
  dht.begin();
  temperatureNode.setProperty("unit").send("°C");
  heatindexNode.setProperty("unit").send("°C");
  humidityNode.setProperty("unit").send("%");
}

void loopHandler() {
  int buttonValue = debouncer.read();

  if (buttonValue != lastButton1Value) {
     Serial << "Button is now " << (buttonValue ? "open" : "close") << endl;

     buttonNode1.setProperty("open").send(buttonValue ? "true" : "false");
     lastButton1Value = buttonValue;
  }


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

bool broadcastHandler(const String& level, const String& value) {
  Serial << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  pinMode(PIN_BUTTON1, INPUT);
  digitalWrite(PIN_BUTTON1, HIGH);
  debouncer.attach(PIN_BUTTON1);
  debouncer.interval(50);


  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  heatindexNode.advertise("unit");
  heatindexNode.advertise("degrees");
  humidityNode.advertise("unit");
  humidityNode.advertise("percent");
  buttonNode1.advertise("open");

  Homie.setBroadcastHandler(broadcastHandler); // before Homie.setup()
  Homie.setup();
}

void loop() {
  Homie.loop();
  debouncer.update();
}
