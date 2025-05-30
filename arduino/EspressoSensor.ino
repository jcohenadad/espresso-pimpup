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
#define DEBUG_MODE true

// Display
#define TFT_CS        10 // chip select
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8 // data/command
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Temperature sensor
#define MAX_CS        6
Adafruit_MAX31855 thermocouple(MAX_CS);

// Pressure sensor
int sensorPin = A1;
float vout = 0.0;
float vin = 0.0;
float pressure = 0.0;

// Capacitive coupling (water level)
#define UPPER_BOUND  0X4000                 // max readout capacitance
#define LOWER_BOUND  (-1 * UPPER_BOUND)
#define CHANNEL 0                          // channel to be read
#define MEASURMENT 0                       // measurment channel
int capdac = 0;
char result[100];
FDC1004 FDC;

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
}

void loop()
{
  // Read Temperature
  float temperature = thermocouple.readCelsius();
  // Error handling
  if (isnan(temperature)) {
    temperature = -1000;
  }

  // Read Pressure
  int pressure_voltage = analogRead(sensorPin);
  vin = (pressure_voltage * 5.0) / 1024.0;
  pressure = (vin < 0.5) ? 0.0 : (vin * 20) - 10;  // Clip pressure at 0 if vin < 0.5

  // Read Water Tank Level
  FDC.configureMeasurementSingle(MEASURMENT, CHANNEL, capdac);
  FDC.triggerSingleMeasurement(MEASURMENT, FDC1004_100HZ);
  delay(500); // wait for completion
  uint16_t value[2];
  if (! FDC.readMeasurement(MEASURMENT, value))
  {
    int16_t msb = (int16_t) value[0];
    int32_t capacitance = ((int32_t)457) * ((int32_t)msb); //in attofarads
    capacitance /= 1000;   //in femtofarads
    capacitance += ((int32_t)3028) * ((int32_t)capdac);
    Serial.print((((float)capacitance/1000)),4);
    Serial.println("  pf, ");
    // TODO: put code below in function
    if (msb > UPPER_BOUND)               // adjust capdac accordingly
	{
      if (capdac < FDC1004_CAPDAC_MAX)
	  capdac++;
    }
	else if (msb < LOWER_BOUND)
	{
      if (capdac > 0)
	  capdac--;
    }
  }
  float water_level = 0;  // TODO: fix later


  // Update Display
  // tft.fillRect(0, 0, 240, 40, ST77XX_BLACK);  // Clear temperature section (adjust height for visibility)
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.print("Temp: ");
  tft.print(temperature, 1);
  tft.print(" C");

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 50);
  tft.setTextSize(3);
  tft.print("Pressure: ");
  tft.print(pressure, 1);
  tft.print(" Bar");

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 90);
  tft.setTextSize(3);
  tft.print("Water: ");
  tft.print((int)water_level, 1);
  tft.print(" %");

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
