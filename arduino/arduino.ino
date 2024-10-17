#include <RTClib.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h> // for ultrasonic-- might not be needed

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Temperature sensor
#define ONE_WIRE_BUS 2 // PIN number on Arduino for data (yellow wire)
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Pressure sensor
int sensorPin = A1;
float vout = 0.0;
float vin = 0.0;
float pressure = 0.0;

// Ultrasonic distance sensor (water tank level)
const int trigPin = 9;
const int echoPin = 10;
long duration;
float distance;
// Values below need to be calibrated
const float distance_full = 5;  // cm
const float distance_empty = 20;  // cm

/* Create an rtc object */
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  delay(1000);  // Give some time for Serial communication to stabilize

  // OLED
  Serial.println("Starting OLED initialization...");
  // Initialize the 128x64 OLED display at the correct I2C address (check from scanner)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println("OLED initialized successfully!");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Hello :)");
  display.display(); // Update the display with the new content
  delay(2000);

  // Temperature sensor
  sensors.begin();

  // Pressure sensor
  pinMode(sensorPin, INPUT);

  // Ultrasound
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop()
{
  // Serial.println("Hello world!");
  // delay(1000);

  // Clear the display buffer
  display.clearDisplay();
  display.setCursor(0, 0);

  // Temperature
  // ==========================================================================
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  Serial.print("Temp [°C]: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  // Display on OLED
  display.print(sensors.getTempCByIndex(0), 2);
  display.print(" C");

  // Pressure sensor
  // ==========================================================================
  int pressure_voltage = analogRead(sensorPin);
  vin = (pressure_voltage * 5.0) / 1024.0;
  pressure = (vin * 20) - 10;
  // Clip pressure at 0 if voltage is below 0.5V
  if (vin < 0.5) {
    pressure = 0.0;
  }
  // print out the value you read:
  Serial.print("Pressure [Bar]: ");
  Serial.print(pressure,2);
  Serial.print("  Voltage: ");
  Serial.println(vin);
  // Display on OLED
  display.setCursor(0, 24);
  display.print(pressure, 2);
  display.print(" Bar");

  // Water tank level
  // ==========================================================================
  digitalWrite(trigPin, LOW);  // Clears the trigPin
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Convert distance to percentage of water tank level in percentage
  float water_level = 100 * (distance - distance_full) / (distance_empty - distance_full);
  // Prints the distance on the Serial Monitor
  Serial.print("Distance [cm]: ");
  Serial.println(distance);
  Serial.print("  Water tank [%]: ");
  Serial.println((int)water_level);
  // Display on OLED
  display.setCursor(0, 48);
  display.print((int)water_level);
  display.print(" %");

  // display.setCursor(0, 20);
  // display.print("Hello!");

  // DateTime now = rtc.now(); // Get current date/time

  // Serial.println("Day: " + String(now.day()));

  // display.clearDisplay(); // Clears display
  // display.setTextColor(SSD1306_WHITE); // Sets color to white
  // display.setTextSize(2); // Sets text size to 2
  // display.setCursor(0, 0); // X, Y starting coordinates

  // print2digits(now.day());   // Retrieve day 
  // display.print("/");
  // print2digits(now.month()); // Retrieve month
  // display.print("/");
  // print2digits(now.year() - 2000); // Retrieve year (last two digits)

  // // Serial.println("Day: " + String(now.day()));

  // display.setCursor(0, 18); // Change cursor to second row
  // print2digits(now.hour());   // Retrieve hours
  // display.print(":");
  // print2digits(now.minute()); // Retrieve minutes
  // display.print(":");
  // print2digits(now.second()); // Retrieve seconds

  display.display(); // Print to display

  delay(500);
}

void print2digits(int number) {
  if (number < 10) {
    display.print("0"); // Print a 0 before if the number is < 10
  }
  display.print(number);
}
