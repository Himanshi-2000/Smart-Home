#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
// Include the necessary library files
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiClient.h>
// #include <BlynkSimpleEsp32.h>
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Define pins for LEDs, ultrasonic sensor, and relay
#define LED1 2
#define LED2 4
#define LED3 5
#define LED4 18
#define trig 12
#define echo 13
#define relay 14

// Set the maximum water tank level (in centimeters)
int MaxLevel = 13;
int Level1 = (MaxLevel * 85) / 100;
int Level2 = (MaxLevel * 75) / 100;
int Level3 = (MaxLevel * 55) / 100;
int Level4 = (MaxLevel * 25) / 100;

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create a Blynk timer
BlynkTimer timer;

// Set your Blynk authentication token
char auth[] = BLYNK_AUTH_TOKEN;

// Set your WiFi credentials
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

// Motor state flags
bool motorOn = false;

void setup() {
  // Initialize the serial console for debugging
  Serial.begin(115200);

  // Connect to the Blynk server with your credentials
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Initialize the LCD display and other pins
  lcd.init();
  lcd.backlight();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  // Display a loading message on the LCD
  lcd.setCursor(0, 0);
  lcd.print("System");
  lcd.setCursor(4, 1);
  lcd.print("Loading...");
  delay(4000);
  lcd.clear();
}

// Function to get ultrasonic sensor values and update the display
void ultrasonic() {
  // Trigger the ultrasonic sensor and measure distance
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  // Calculate the water level percentage based on distance
  int waterLevelPercentage = map(distance, 4, 12, 100, 0);

  // Send water level percentage to Blynk
  Blynk.virtualWrite(V6, waterLevelPercentage);

  // Update the LCD display
  lcd.setCursor(0, 0);
  lcd.print("WLevel:");

  if (Level1 <= distance) {
    lcd.setCursor(8, 0);
    lcd.print("Very Low");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);

    // Automatically turn on the motor and update the display
    if (!motorOn) {
      digitalWrite(relay, LOW);
      motorOn = true;
      lcd.setCursor(0, 1);
      lcd.print("Motor is ON ");
    }
  } else if (Level2 <= distance && Level1 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Low");
    lcd.print("      ");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
  } else if (Level3 <= distance && Level2 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Medium");
    lcd.print("      ");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, LOW);
  } else if (Level4 <= distance && Level3 > distance) {
    lcd.setCursor(8, 0);
    lcd.print("Full");
    lcd.print("      ");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
  } else {
    // Turn off the motor and update the display
    if (motorOn) {
      digitalWrite(relay, HIGH);
      motorOn = false;
      lcd.setCursor(0, 1);
      lcd.print("Motor is OFF");
    }
  }
}

// Blynk button widget handler to control the motor
BLYNK_WRITE(V1) {
  bool Relay = param.asInt();
  if (Relay == 1) {
    digitalWrite(relay, LOW);
    motorOn = true;
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
  } else {
    digitalWrite(relay, HIGH);
    motorOn = false;
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");
  }
}

void loop() {
  // Continuously monitor the water level and run Blynk
  ultrasonic();
  Blynk.run();
}
