#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define GSM_RX 8
#define GSM_TX 7
#define DAM_SENSOR_PIN 2
#define FLOW_SENSOR_PIN 3
#define BUZZER_PIN 9
#define LED_SCREEN_ADDRESS 0x27
#define PHONE_NUMBER "+9779814000128"

SoftwareSerial mySerial(GSM_RX, GSM_TX);
LiquidCrystal_I2C lcd(LED_SCREEN_ADDRESS, 16, 2);

unsigned long lastAlertTime = 0;
unsigned long alertInterval = 60000;  // Set the alert interval to 1 minute
unsigned long buzzerStartTime = 0;
unsigned long buzzerDuration = 5000;  // Set the buzzer duration to 5 seconds
unsigned long buzzerInterval = 2000;  // Set the interval between buzzer activations to 2 seconds

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Wire.begin();

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();

  pinMode(DAM_SENSOR_PIN, INPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.print("Water Monitoring");
}

void loop() {
  int damWaterLevel = digitalRead(DAM_SENSOR_PIN);
  int flowDetected = digitalRead(FLOW_SENSOR_PIN);

  Serial.print("Dam Water Level: ");
  Serial.println(damWaterLevel ? "HIGH" : "LOW");

  Serial.print("Flow Detected: ");
  Serial.println(flowDetected ? "YES" : "NO");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dam Level: ");
  lcd.print(damWaterLevel ? "HIGH" : "LOW");

  lcd.setCursor(0, 1);
  lcd.print("Flow Detected: ");
  lcd.print(flowDetected ? "YES" : "NO");

  // Check if the dam water level is high
  if (damWaterLevel == HIGH) {
    // Check the time since the last alert
    if (millis() - lastAlertTime >= alertInterval) {
      // Water level is high, send SMS and make a call
      if (sendSMS("High water level at dam! Take action immediately.")) {
        Serial.println("SMS sent successfully.");
      } else {
        Serial.println("Error sending SMS.");
      }

      if (callPhoneNumber(PHONE_NUMBER)) {
        Serial.println("Call initiated successfully.");
        delay(5000); // Wait for 5 seconds before ending the call
        endCall();   // End the call
      } else {
        Serial.println("Error initiating call.");
      }

      // Activate the buzzer only when flood is detected
      if (flowDetected == HIGH) {
        activateBuzzer();
        lcd.clear();
        lcd.print("Flood at Dam!");
      }

      lastAlertTime = millis();  // Update last alert time
    }
  }

  delay(5000);
}

bool sendSMS(String message) {
  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.print("AT+CMGS=\"");
  mySerial.print(PHONE_NUMBER);
  mySerial.println("\"");
  delay(1000);
  mySerial.print(message);
  delay(1000);
  mySerial.write(26);
  delay(1000);

  delay(5000);

  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }

  return mySerial.find("OK") != NULL;
}

bool callPhoneNumber(String phoneNumber) {
  mySerial.print("ATD");
  mySerial.print(phoneNumber);
  mySerial.println(";");
  delay(1000);

  delay(5000);

  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }

  return mySerial.find("OK") != NULL;
}

void endCall() {
  mySerial.println("ATH0"); // Command to end the ongoing call

  delay(3000);

  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}

void activateBuzzer() {
  // Check if it's time to activate the buzzer
  if (millis() - buzzerStartTime >= buzzerInterval) {
    digitalWrite(BUZZER_PIN, HIGH);  // Activate the buzzer
    delay(buzzerDuration);  // Buzzer active for 5 seconds
    digitalWrite(BUZZER_PIN, LOW);  // Deactivate the buzzer
    buzzerStartTime = millis();  // Reset the buzzer start time
  }
}
