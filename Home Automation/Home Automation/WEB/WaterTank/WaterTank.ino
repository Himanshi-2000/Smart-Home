// Include the library files
#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>

#define LED1 2
#define LED2 4
#define LED3 5
#define LED4 18
#define trig 12
#define echo 13
#define relay 14
#define buzzerPin 15  // Change to your buzzer pin

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Enter your tank max value (CM)
int MaxLevel = 13;
int Level1 = (MaxLevel * 85) / 100;
int Level2 = (MaxLevel * 75) / 100;
int Level3 = (MaxLevel * 55) / 100;
int Level4 = (MaxLevel * 25) / 100;

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

BlynkTimer timer;

// Enter your Auth token
char auth[] = BLYNK_AUTH_TOKEN;

// Enter your WIFI SSID and password
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

// Insert Firebase project API Key
#define API_KEY "AIzaSyAzctiicoFA7lBxnJ9GaLU3Y8SItgxoQQE"

// Insert RTDB URL
#define DATABASE_URL "https://real-time-data-b7ba8-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth fauth;
FirebaseConfig config;

// Motor state
bool motorOn = false;
bool buzzerOn = false;
bool signupOK = false;
unsigned long sendDataPrevMillis = 0;

void setup() {
  // Debug console
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  lcd.init();
  lcd.backlight();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(relay, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("System");
  lcd.setCursor(4, 1);
  lcd.print("Loading..");
  delay(4000);
  lcd.clear();

  // Assign the API key
  config.api_key = API_KEY;

  // Assign the RTDB URL
  config.database_url = DATABASE_URL;

  // Sign up
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

// Get the ultrasonic sensor values
void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  Serial.println(distance);

  int waterLevelPercentage = map(distance, 4, 12, 100, 0);  // Map the distance to a percentage
  waterLevelPercentage = constrain(waterLevelPercentage, 0, 100);

  Blynk.virtualWrite(V6, waterLevelPercentage);  // Send water level percentage to Blynk
  if (!isnan(waterLevelPercentage)) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.setInt(&fbdo, "Ultrasonic/WaterLevel", waterLevelPercentage)) {
        Serial.print("Water Level Percentage: ");
        Serial.print(waterLevelPercentage);
        Serial.println("%");
      } else {
        Serial.println("Failed to write water level to Firebase");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    } else {
      Serial.println("Failed to read water level from the sensor!");
      delay(2000);
    }
  }

  // Rest of the code remains the same
  lcd.setCursor(0, 0);
  lcd.print("WLevel:");

  if (Level1 <= distance) {
    lcd.setCursor(8, 0);
    lcd.print("Very Low");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);

    // Automatically turn on the motor and buzzer when the water level is "Very Low"
    if (!motorOn) {
      digitalWrite(relay, LOW);
      motorOn = true;
      lcd.setCursor(0, 1);
      lcd.print("Motor is ON ");
      Firebase.RTDB.setString(&fbdo, "Ultrasonic/Motor_Status", "ON");
    }

    if (!buzzerOn) {
      digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
      buzzerOn = true;
    }
  } else if (Level2 <= distance && Level1 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Low");
    lcd.print("      ");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);

    // Turn off the buzzer when the water level is "Low"
    if (buzzerOn) {
      digitalWrite(buzzerPin, LOW);  // Turn off the buzzer
      buzzerOn = false;
    }
  } else if (Level3 <= distance && Level2 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Medium");
    lcd.print("      ");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, LOW);
  } else if (Level4 <= distance && Level3 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Full");
    lcd.print("      ");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
  } else {
    // Turn off the motor and buzzer if the water level is not "Very Low"
    if (motorOn) {
      digitalWrite(relay, HIGH);
      motorOn = false;
      lcd.setCursor(0, 1);
      lcd.print("Motor is OFF");
      Firebase.RTDB.setString(&fbdo, "Ultrasonic/Motor_Status", "OFF");
    }

    if (buzzerOn) {
      digitalWrite(buzzerPin, LOW);  // Turn off the buzzer
      buzzerOn = false;
    }
  }
}

// Get the button value
BLYNK_WRITE(V7) {
  bool Relay = param.asInt();
  if (Relay == 1) {
    digitalWrite(relay, LOW);
    motorOn = true;
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
    Firebase.RTDB.setString(&fbdo, "Ultrasonic/Motor_Status", "ON");
  } else {
    digitalWrite(relay, HIGH);
    motorOn = false;
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");+
    Firebase.RTDB.setString(&fbdo, "Ultrasonic/Motor_Status", "OFF");
  }
}

void loop() {
  ultrasonic();
  Blynk.run();  // Run the Blynk library
}