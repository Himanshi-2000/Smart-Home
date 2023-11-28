#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>

#define LED_PIN 23
#define FAN_PIN 22
#define DHT_SENSOR_PIN 13
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

// Insert Firebase project API Key
#define API_KEY "AIzaSyAzctiicoFA7lBxnJ9GaLU3Y8SItgxoQQE"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://real-time-data-b7ba8-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth fauth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;    

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
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

    /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &fauth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &fauth);
  Firebase.reconnectWiFi(true);

}

void loop() {
  Blynk.run();
  float temp = dht_sensor.readTemperature();
  float humi = dht_sensor.readHumidity();
  if (!isnan(temp) || !isnan(humi)) {
    // Use Blynk.virtualWrite to update the temperature display widget
    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V0, humi);

      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    //since we want the data to be updated every second
    sendDataPrevMillis = millis();
    // Enter Temperature in to the DHT_11 Table
      if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temp)){
      // This command will be executed even if you dont serial print but we will make sure its working
      Serial.print("Temperature : ");
      Serial.print(temp); Serial.print("°C"); Serial.println("  |  ");
    }
      else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }
        // Enter Humidity in to the DHT_11 Table
    if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humi)){
      Serial.print("Humidity : " );
      Serial.print(humi);Serial.println("%");
      delay(2000);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
  }
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
