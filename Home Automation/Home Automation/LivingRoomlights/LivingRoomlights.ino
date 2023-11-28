#define BLYNK_TEMPLATE_ID "TMPL6q5ye6d0-"
#define BLYNK_TEMPLATE_NAME "Fan Temperature and Humidity"
#define BLYNK_AUTH_TOKEN "a_oRZO3QFnlxWXY6PHRH3NS0zkZn53Ui"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define LED_PIN1 19

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "IHPHONE";
char pass[] = "bbbbbbba";

BLYNK_WRITE(V8) {
  // This function is called when the LedSwitch widget value changes in the Blynk app
  bool ledStatus = param.asInt();
  if (ledStatus == 1) {
    // LedSwitch is ON
    digitalWrite(LED_PIN1, HIGH);
  } else {
    // LedSwitch is OFF
    digitalWrite(LED_PIN1, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  Blynk.begin(auth, ssid, pass);

  pinMode(LED_PIN1, OUTPUT);
}

void loop() {
  Blynk.run();
}
