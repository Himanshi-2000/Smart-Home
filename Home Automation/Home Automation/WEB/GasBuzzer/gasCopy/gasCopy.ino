// Set your Blynk template information and authentication token
#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"

// Enable serial debugging
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyAzctiicoFA7lBxnJ9GaLU3Y8SItgxoQQE"
#define DATABASE_URL "https://real-time-data-b7ba8-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth fauth;
FirebaseConfig config;
BlynkTimer timer;

// Set Wi-Fi credentials
char auth[] = BLYNK_AUTH_TOKEN;  // Replace with your Blynk authorization token
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

bool fanOn = false;

// Pin assignments
#define MQ2 34
#define FAN_PIN 18  // Exhaust fan control pin
#define VIRTUAL_PIN 1
#define GAS_THRESHOLD 1700  // Set your gas concentration threshold here
#define buzzer 5

int sensorValue = 0;
bool signupOK = false;
unsigned long sendDataPrevMillis = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(MQ2, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(FAN_PIN, LOW);
  digitalWrite(buzzer, LOW);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &fauth, "", "")) {
    Serial.println("Signup successful");
    signupOK = true;
  } else {
    Serial.printf("Signup error: %s\n", config.signer.signupError.message.c_str());
  }

  // Initialize Firebase
  Firebase.begin(&config, &fauth);
  Firebase.reconnectWiFi(true);

  // Set up a timer to read the gas sensor periodically
  timer.setInterval(1000L, readGasSensor);
}

void readGasSensor() {
  sensorValue = analogRead(MQ2);

  if (!isnan(sensorValue)) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.setInt(&fbdo, "MQ2/GasValue", sensorValue)) {
        Serial.print("Gas Value : ");
        Serial.print(sensorValue);

        // If gas concentration is above the threshold, turn on the fan
        if (sensorValue > GAS_THRESHOLD) {
          Firebase.RTDB.setString(&fbdo, "MQ2/FanStatus", "ON");
          Serial.println("Gas Detected!");
          digitalWrite(FAN_PIN, HIGH);  // Turn on the fan when gas is detected
          digitalWrite(buzzer, HIGH);
        } else {
          Firebase.RTDB.setString(&fbdo, "MQ2/FanStatus", "OFF");
          Serial.println("No Gas Detected");
          digitalWrite(FAN_PIN, LOW);  // Turn off the fan when no gas is detected
          digitalWrite(buzzer, LOW);
        }

      } else {
        Serial.println("Failed to write Gas level to Firebase");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    } else {
      Serial.println("Failed to read Gas level from the sensor!");
      delay(2000);
    }
  }

  // If gas concentration is above the threshold, turn on the fan
  if (sensorValue > GAS_THRESHOLD) {
    Serial.println("Gas Detected!");
    digitalWrite(FAN_PIN, HIGH);  // Turn on the fan when gas is detected
    digitalWrite(buzzer, HIGH);
  } else {
    Serial.println("No Gas Detected");
    digitalWrite(FAN_PIN, LOW);  // Turn off the fan when no gas is detected
    digitalWrite(buzzer, LOW);
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
