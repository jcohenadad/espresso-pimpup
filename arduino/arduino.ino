#include <RTClib.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h> // for ultrasonic-- might not be needed
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// Debugging
#define DEBUG_MODE true

// Display
#define TFT_CS        10 // chip select
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8 // data/command
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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
const int trigPin = 3;
const int echoPin = 4;
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

  // Initialize display
  // Serial.print(F("Hello!"));
  tft.init(240, 280);  // Init ST7789 280x240
  tft.setRotation(3);  // rotate clockwise by 270°
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);

  // Display welcome message
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(30, 90);
  tft.print("Hello Julien!");
  delay(1000);
  tft.setCursor(15, 130);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("ready for some coffee?");
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);

  // display.clearDisplay();
  // display.setTextColor(SSD1306_WHITE);
  // display.setTextSize(2);
  // display.setCursor(0, 0);
  // display.print("Hello :)");
  // display.display(); // Update the display with the new content

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
  // Read Temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Read Pressure
  int pressure_voltage = analogRead(sensorPin);
  vin = (pressure_voltage * 5.0) / 1024.0;
  pressure = (vin < 0.5) ? 0.0 : (vin * 20) - 10;  // Clip pressure at 0 if vin < 0.5

  // Read Water Tank Level
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  float water_level = 100 - 100 * (distance - distance_full) / (distance_empty - distance_full);


  // tft.init(240, 280);  // Init ST7789 280x240
  // tft.setRotation(3);  // rotate clockwise by 270°
  // tft.setTextWrap(false);
  tft.setCursor(15, 130);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("blablabla");
  delay(2000);

  // Update Display
  // tft.fillRect(0, 0, 240, 40, ST77XX_BLACK);  // Clear temperature section (adjust height for visibility)
  // tft.setTextColor(ST77XX_WHITE);
  // tft.setCursor(10, 10);
  // tft.setTextSize(2);
  // tft.print("Temp: ");
  // tft.print(temperature, 1);
  // tft.print(" C");

  // display.fillRect(0, 0, 128, 16, SSD1306_BLACK);  // Clear temperature section
  // display.setCursor(0, 0);
  // display.print(temperature, 1);
  // display.print(" C");

  // display.fillRect(0, 24, 128, 16, SSD1306_BLACK);  // Clear pressure section
  // display.setCursor(0, 24);
  // display.print(pressure, 1);
  // display.print(" Bar");

  // display.fillRect(0, 48, 128, 16, SSD1306_BLACK);  // Clear water level section
  // display.setCursor(0, 48);
  // display.print((int)water_level);
  // display.print(" %");

  // display.display();

  // Debug output
  if (DEBUG_MODE) {
    Serial.print("Temp [°C]: ");
    Serial.println(temperature);
    Serial.print("Pressure [Bar]: ");
    Serial.println(pressure);
    Serial.print("Water Tank [%]: ");
    Serial.println(water_level);
  }

  delay(500);
}

void print2digits(int number) {
  if (number < 10) {
    tft.print("0"); // Print a 0 before if the number is < 10
  }
  tft.print(number);
}
