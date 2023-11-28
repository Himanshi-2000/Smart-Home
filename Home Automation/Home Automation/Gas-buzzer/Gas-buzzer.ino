// Set your Blynk template information and authentication token
#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"

// Enable serial debugging
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

BlynkTimer timer;

// Set Wi-Fi credentials
char auth[] = BLYNK_AUTH_TOKEN; // Replace with your Blynk authorization token
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

bool fanOn = false;

// Pin assignments
#define MQ2 34
#define FAN_PIN 18   // Exhaust fan control pin
#define VIRTUAL_PIN 1
#define GAS_THRESHOLD 1700 // Set your gas concentration threshold here
#define buzzer 5

int sensorValue = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(MQ2, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(FAN_PIN, LOW);
  digitalWrite(buzzer, LOW);

  // Set up a timer to read the gas sensor periodically
  timer.setInterval(1000L, readGasSensor);
}

void readGasSensor() {
  sensorValue = analogRead(MQ2);

  Serial.print("Gas Sensor Value: ");
  Serial.println(sensorValue);

  // If gas concentration is above the threshold, turn on the fan
  if (sensorValue > GAS_THRESHOLD) {
    Serial.println("Gas Detected!");
      digitalWrite(FAN_PIN, HIGH); // Turn on the fan when gas is detected
      digitalWrite(buzzer, HIGH);
  } else {
    Serial.println("No Gas Detected");
      digitalWrite(FAN_PIN, LOW);// Turn off the fan when no gas is detected
      digitalWrite(buzzer, LOW);
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
