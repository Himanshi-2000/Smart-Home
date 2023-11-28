#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>

#define PIR_PIN 5     // PIR sensor input pin
#define BUZZER_PIN 2  // Buzzer control pin

#include "addons/TokenHelper.h"  // Provide the token generation process info.
#include "addons/RTDBHelper.h"   // Provide the RTDB payload printing info and other helper functions.

#define API_KEY "AIzaSyAzctiicoFA7lBxnJ9GaLU3Y8SItgxoQQE"
#define DATABASE_URL "https://real-time-data-b7ba8-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth fauth;
FirebaseConfig config;
BlynkTimer timer;

bool signupOK = false;
unsigned long sendDataPrevMillis = 0;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

int motionDetected = 0;
// LDR pin (analog input)
int ldrPin = A0;  //vp
int ldrValue = 0;

// LED pin
int ledPin1 = 18;  // Replace with the pin where your LED is connected
int ledPin2 = 15;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);

  Blynk.begin(auth, ssid, pass);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);


  lcd.init();  // Initialize the LCD
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System");
  lcd.setCursor(4, 1);
  lcd.print("Loading..");
  delay(4000);
  lcd.clear();

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer initially

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
}

void loop() {
  Blynk.run();

  // Read the PIR sensor
  motionDetected = digitalRead(PIR_PIN);
  // Read the LDR value
  ldrValue = analogRead(ldrPin);

  // Adjust the threshold as needed
  int threshold = 500;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

        // If gas concentration is above the threshold, turn on the fan
        if (motionDetected == HIGH) {
          Firebase.RTDB.setString(&fbdo, "Motion/motionStatus", "Motion Detected");
          lcd.clear();
          Serial.println("Motion Detected");
          lcd.setCursor(0, 0);
          lcd.print("Motion Detected");
          digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer
          digitalWrite(BUZZER_PIN, LOW);   // Turn off the buzzer
          delay(1000);
          digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer
          delay(2000);                     // Buzzer on for 3 seconds (adjust as needed)
          digitalWrite(BUZZER_PIN, LOW);   // Turn off the buzzer
          delay(1000);
        } else {
          Firebase.RTDB.setString(&fbdo, "Motion/motionStatus", "No Motion");
          Serial.println("No Motion");
          digitalWrite(BUZZER_PIN, LOW);
          delay(1000);
        }
    } else {
      Serial.println("Failed to read Motion status from the sensor!");
      delay(2000);
    }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

        // If gas concentration is above the threshold, turn on the fan
        if (ldrValue < threshold) {
          Firebase.RTDB.setString(&fbdo, "LDR/LED_Status", "OFF");
          digitalWrite(ledPin1, LOW);
          digitalWrite(ledPin2, LOW);
        } else {
          Firebase.RTDB.setString(&fbdo, "LDR/LED_Status", "ON");
          digitalWrite(ledPin1, HIGH);
          digitalWrite(ledPin2, HIGH);
        }
    } else {
      Serial.println("Failed to read Motion status from the sensor!");
      delay(2000);
    }  

    lcd.clear();
}
