//Work

#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BlynkSimpleEsp32.h>

#define LED_PIN 23
#define FAN_PIN 22
#define DHT_SENSOR_PIN 13
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

int threshhold_temperature = 25; // Set your threshold temperature here

void onThreshholdStepperChange(int threshold_stepper) {
    if (threshold_stepper < dht_sensor.readTemperature()) {
      digitalWrite(FAN_PIN, HIGH);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }
}
int sliderValue = 0;  // Initialize with a default value

BLYNK_WRITE(V2) {
  int fanStatus = param.asInt();
  if (fanStatus == 1) {
    onThreshholdStepperChange(sliderValue);
  } else {
    digitalWrite(FAN_PIN, HIGH); // Turn off the fan
  }
}



BLYNK_WRITE(V3) {
  // This function is called when the LedSwitch widget value changes in the Blynk app
  bool ledStatus = param.asInt();
    if (ledStatus == 1) {
    // LedSwitch is ON
    digitalWrite(LED_PIN, HIGH);
  } else {
    // LedSwitch is OFF
    digitalWrite(LED_PIN, LOW);
  }
}

BLYNK_WRITE(V4) {
  int newValue = param.asInt();
  sliderValue = newValue;
  onThreshholdStepperChange(sliderValue);
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  dht_sensor.begin();
  Blynk.begin(auth, ssid, pass);

  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  Blynk.run();
  float temp = dht_sensor.readTemperature();
  float humi = dht_sensor.readHumidity();
  if (!isnan(temp) || !isnan(humi)) {
    // Use Blynk.virtualWrite to update the temperature display widget
    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V0, humi);

    Serial.print("Humidity: "); Serial.print(humi); Serial.print("%"); Serial.print("  |  ");
    Serial.print("Temperature: "); Serial.print(temp); Serial.println("°C");
    delay(2000);

    if (sliderValue < temp) {
      // Turn on the fan when sliderValue is greater than the temperature
      digitalWrite(FAN_PIN, HIGH);
    } else {
      // Turn off the fan when sliderValue is less than or equal to the temperature
      digitalWrite(FAN_PIN, LOW);
    }
  } else {
    Serial.println("Failed to read temperature from DHT Sensor!");
    delay(2000);
  }

}
