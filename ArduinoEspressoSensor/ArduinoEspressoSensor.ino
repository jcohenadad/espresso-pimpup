#include <RTClib.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MAX31855.h> // for thermal sensor
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h> // for capacitive coupling sensor
#include <Protocentral_FDC1004.h> // for capacitive coupling sensor

// Debugging
// #define DEBUG_MODE true
#define DEBUG_TEMPERATURE false
#define DEBUG_PRESSURE false
#define DEBUG_CAPACITANCE true

// Display
#define TFT_CS        10 // chip select
#define TFT_RST       -1 // set to -1 means the pin 7 (RST) should not be connected
#define TFT_DC         8 // data/command
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Temperature sensor
#define MAX_CS        9 // chip select pin for MAX31855
Adafruit_MAX31855 thermocouple(MAX_CS);

// Pressure sensor
int sensorPin = A0;
float vout = 0.0;
float vin = 0.0;
float pressure = 0.0;

// Capacitive coupling (water level)
// FDC1004 communicates over I²C, so you must connect:
// - SCL (clock) of FDC1004 → A5 on Arduino Uno, or SCL pin on Nano
// - SDA (data) of FDC1004 → A4 on Uno, or SDA on Nano
#define CHANNEL 0  // channel of the FDC1004 to be read
#define MEASUREMENT 0  // measurEment channel
#define LOWER_BOUND  9  // min readout capacitance in pF (find experimentally, and set DEBUG_CAPACITANCE=true)
#define UPPER_BOUND  9.65  // max readout capacitance in pF
int capdac = 0;
int16_t msb;
int32_t capacitance;
float capacitance_pF;
char result[100];
FDC1004 fdc; // Create an FDC1004 object

// Global variables for brewing state
bool isBrewing = false;
unsigned long brewStartTime = 0;
float brewDuration = 0.0;
const float pressureThreshold = 0.5; // Set your threshold (in Bar)
int blinkCount = 0;
unsigned long lastBlinkTime = 0;
bool showTimer = true;

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

  // Pressure sensor
  pinMode(sensorPin, INPUT);

  // Initialize the FDC1004
  Wire.begin();  // Initialize I2C communication  
}

void loop()
{
  // Read Water Tank Level
  fdc.configureMeasurementSingle(MEASUREMENT, CHANNEL, capdac);
  fdc.triggerSingleMeasurement(MEASUREMENT, FDC1004_100HZ);
  delay(100); // Must wait at least 9ms for conversion. We use this as our loop delay.
  uint16_t value[2];
  // If readMeasurement returns false, it means the reading was successful
  if (! fdc.readMeasurement(MEASUREMENT, value))
  {
    msb = (int16_t)value[0];  // Most significant byte
    capacitance = 457L * msb + 3028L * capdac * 1000L;  // Gain factor: 457 (datasheet says 16-bit range = ±16.384pF → gain ≈ 16.384pF / 2^15 ≈ 500 af/LSB; here it's 457 af/LSB). there is also an offset factor of 3028.
    capacitance_pF = capacitance / 1e6;
    if (DEBUG_CAPACITANCE) {
      Serial.print("msb: ");
      Serial.println((msb),4);
      Serial.print("capacitance_pF: ");
      Serial.print((capacitance_pF),4);  // Prints the capacitance
      Serial.println("  pf, ");
    }
  }
  // Convert to water level based on calibration
  float water_level = (capacitance_pF - LOWER_BOUND) / (UPPER_BOUND - LOWER_BOUND) * 100;
  // Here we assume a linear mapping for demonstration purposes

  // Read Temperature
  float temperature = thermocouple.readCelsius();
  // Error handling
  if (isnan(temperature)) {
    temperature = -1000;
  }

  // Read Pressure
  int pressure_voltage = analogRead(sensorPin);
  if (DEBUG_PRESSURE) {
    Serial.print("Pressure ADC value: ");
    Serial.println(pressure_voltage);
  }
  vin = (pressure_voltage * 5.0) / 1024.0;
  pressure = (vin < 0.5) ? 0.0 : (vin * 20) - 10;  // Clip pressure at 0 if vin < 0.5

  // Brew timer logic
  // if (!isBrewing && pressure > pressureThreshold) {
  //   isBrewing = true;
  //   brewStartTime = millis();
  // }

  // if (isBrewing && pressure < pressureThreshold) {
  //   isBrewing = false;
  //   brewDuration = (millis() - brewStartTime) / 1000.0; // Save in seconds (float)
  // }
  
  // Update Display
  // tft.fillRect(0, 0, 240, 40, ST77XX_BLACK);  // Clear temperature section (adjust height for visibility)
  
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.setCursor(10, 20);
  tft.setTextSize(3);
  tft.print("Water: ");
  tft.print((int)water_level, 1);
  tft.print(" %");
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.setCursor(10, 60);
  tft.setTextSize(3);
  tft.print("Temp: ");
  tft.print(temperature, 1);
  tft.print(" C");

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 100);
  tft.setTextSize(3);
  tft.print("Pressure: ");
  tft.print(pressure, 1);
  tft.print(" Bar");


  // tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  // tft.setCursor(10, 140);
  // tft.setTextSize(3);
  // tft.print("Timer: ");
  // if (isBrewing) {
  // float elapsed = (millis() - brewStartTime) / 1000.0;
  // tft.print(elapsed, 1);  // one decimal
  // tft.print(" s");
  // } 
  // else if (brewDuration > 0) {
  //   // Blink a few times after brewing stops
  //   if (blinkCount < 6) { // 6 toggles = 3 blinks
  //     if (millis() - lastBlinkTime > 500) {
  //       showTimer = !showTimer;
  //       lastBlinkTime = millis();
  //       blinkCount++;
  //     }
  //     if (showTimer) {
  //       tft.print(brewDuration, 1);
  //       tft.print(" s");
  //     } else {
  //       tft.print("        ");
  //     }
  //   } else {
  //     // After 3 blinks, always show final duration
  //     tft.print(brewDuration, 1);
  //     tft.print(" s");
  //   }
  // }

  // Debug output
  if (DEBUG_CAPACITANCE) {
    Serial.print("Water Tank [%]: ");
    Serial.println(water_level);
  }
  if (DEBUG_TEMPERATURE) {
    Serial.print("Temperature [°C]: ");
    Serial.println(temperature);
  }
  if (DEBUG_PRESSURE) {
    Serial.print("Pressure [Bar]: ");
    Serial.println(pressure);
  }
  Serial.println("------------------------");
}

void print2digits(int number) {
  if (number < 10) {
    tft.print("0"); // Print a 0 before if the number is < 10
  }
  tft.print(number);
}
