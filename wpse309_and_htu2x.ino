#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HTU21DF.h>

// Create sensor object to HTU (temperature and humidity)
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

// Create pin to WPSE (microphone)
int sensorPin = A0;  // A0 pin of sensor connected to A0 pin on ESP8266
int sensorValue;

void setup() {
  Serial.begin(115200); // Initialize serial communication
  // Initialize I2C with new pins (SDA = GPIO2 (D4), SCL = GPIO14 (D5))
  Wire.begin(2, 14);

  if (!htu.begin()) { // Check if the sensor is connected
    Serial.println("Couldn't find HTU21D sensor!");
    while (1);
  }
}

void loop() {
  float temp = htu.readTemperature();  // Read temperature in Celsius
  float humidity = htu.readHumidity(); // Read relative humidity

  // Print the results
  Serial.print("(HUT2X) Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
  
  Serial.print("(HUT2X) Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  sensorValue = analogRead(sensorPin);  // Read analog value (0-1023)
  Serial.print("(WPSE309) Microphone sensor value: ");
  Serial.println(sensorValue);

  Serial.println("-----------------------------------");

  delay(1000);  // Wait 1 second before next reading
}