#include <RTClib.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MAX31855.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Wire.h>
#include <Protocentral_FDC1004.h>

// ─── Debugging ───────────────────────────────────────────────────────────────
#define DEBUG_TEMPERATURE false
#define DEBUG_PRESSURE    false
#define DEBUG_CAPACITANCE true

// ─── Loop duration ───────────────────────────────────────────────────────────
#define LOOP_DURATION 500  // ms

// ─── Display ─────────────────────────────────────────────────────────────────
#define TFT_CS   10
#define TFT_RST  -1
#define TFT_DC    8
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ─── Temperature sensor ──────────────────────────────────────────────────────
#define MAX_CS 9
Adafruit_MAX31855 thermocouple(MAX_CS);

// ─── Pressure sensor ─────────────────────────────────────────────────────────
int   sensorPin = A0;
float pressure  = 0.0;

// ─── Capacitive coupling (water level) ───────────────────────────────────────
// FDC1004 → I²C: SCL → A5 (Uno) / SCL (Nano), SDA → A4 (Uno) / SDA (Nano)
#define CHANNEL     0
#define MEASUREMENT 0
#define LOWER_BOUND 6.0    // min capacitance in pF (calibrate experimentally)
#define UPPER_BOUND 6.9    // max capacitance in pF

int     capdac           = 0;
int16_t msb;
int32_t capacitance;
float   capacitance_pF;
float   capacitance_pF_old = LOWER_BOUND;
FDC1004 fdc;

// ─── Brewing state ───────────────────────────────────────────────────────────
bool          isBrewing         = false;
unsigned long brewStartTime     = 0;
float         brewDuration      = 0.0;
const float   pressureThreshold = 0.5;  // Bar
int           blinkCount        = 0;
unsigned long lastBlinkTime     = 0;
bool          showTimer         = true;

// ─── RTC ─────────────────────────────────────────────────────────────────────
RTC_DS3231 rtc;

// ─── FIX: last-drawn value cache (prevents unnecessary redraws / flicker) ────
int lastTemp             = -9999;
int lastPressureDisplay  = -9999;   // stored as tenths of Bar (×10)
int lastWater            = -9999;

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  delay(1000);

  tft.init(240, 280);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  thermocouple.begin();
  pinMode(sensorPin, INPUT);
  Wire.begin();

  drawStaticLayout();
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // ── Water level (capacitance) ─────────────────────────────────────────────
  fdc.configureMeasurementSingle(MEASUREMENT, CHANNEL, capdac);
  fdc.triggerSingleMeasurement(MEASUREMENT, FDC1004_100HZ);
  delay(LOOP_DURATION);  // ≥9 ms required for conversion; doubles as loop cadence

  uint16_t value[2];
  if (!fdc.readMeasurement(MEASUREMENT, value)) {
    msb           = (int16_t)value[0];
    capacitance   = 457L * msb + 3028L * capdac * 1000L;
    capacitance_pF = capacitance / 1e6;

    if (DEBUG_CAPACITANCE) {
      Serial.print("capacitance_pF: ");
      Serial.print(capacitance_pF, 4);
      Serial.println(" pF");
    }
  }

  // Clamp to last valid reading when out of range
  if (capacitance_pF < LOWER_BOUND || capacitance_pF > UPPER_BOUND) {
    capacitance_pF = capacitance_pF_old;
    if (DEBUG_CAPACITANCE) Serial.println("Capacitance out of bounds — using previous value.");
  } else {
    capacitance_pF_old = capacitance_pF;
  }

  float water_level = 100.0 - (capacitance_pF - LOWER_BOUND) / (UPPER_BOUND - LOWER_BOUND) * 100.0;

  if (DEBUG_CAPACITANCE) {
    Serial.print("Water Level: ");
    Serial.print((int)water_level);
    Serial.println(" %");
  }

  // ── Temperature ───────────────────────────────────────────────────────────
  float temperature = thermocouple.readCelsius();
  if (isnan(temperature)) temperature = -1000;

  if (DEBUG_TEMPERATURE) {
    Serial.print("Temperature [°C]: ");
    Serial.println(temperature);
  }

  // ── Pressure ─────────────────────────────────────────────────────────────
  int   pressure_voltage = analogRead(sensorPin);
  float vin              = (pressure_voltage * 5.0) / 1024.0;
  pressure               = (vin < 0.5) ? 0.0 : (vin * 20.0) - 10.0;

  if (DEBUG_PRESSURE) {
    Serial.print("Pressure ADC: "); Serial.println(pressure_voltage);
    Serial.print("Pressure [Bar]: "); Serial.println(pressure);
  }

  Serial.println("------------------------");

  // ── Update display ────────────────────────────────────────────────────────
  updateDisplay(temperature, pressure, water_level);
}

// ─────────────────────────────────────────────────────────────────────────────
void print2digits(int number) {
  if (number < 10) tft.print("0");
  tft.print(number);
}

// ─────────────────────────────────────────────────────────────────────────────
void drawStaticLayout() {
  tft.fillScreen(ST77XX_BLACK);

  // Water-tank outline (drawn once; never overwritten by updateDisplay)
  int barX = 185, barY = 55, barW = 40, barH = 160;
  tft.drawRect(barX, barY, barW, barH, ST77XX_WHITE);
}

// ─────────────────────────────────────────────────────────────────────────────
void updateDisplay(float temperature, float pressure, float water_level) {

  // ── Quantise to display units ─────────────────────────────────────────────
  int tempDisplay     = (int)temperature;
  int pressureDisplay = (int)(pressure * 10);          // tenths of Bar
  int waterDisplay    = constrain((int)water_level, 0, 100);

  // ── Temperature (only redraw when integer °C changes) ────────────────────
  if (tempDisplay != lastTemp) {
    // Clear only the temperature region
    tft.fillRect(10, 25, 150, 70, ST77XX_BLACK);

    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // bg colour prevents ghost pixels
    tft.setTextSize(6);
    tft.setCursor(20, 35);
    tft.print(tempDisplay);
    tft.setTextSize(4);
    tft.print(" C");

    lastTemp = tempDisplay;
  }

  // ── Pressure (only redraw when tenths-of-Bar value changes) ──────────────
  if (pressureDisplay != lastPressureDisplay) {
    tft.fillRect(10, 120, 150, 70, ST77XX_BLACK);

    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(6);
    tft.setCursor(20, 140);
    tft.print(pressure, 1);
    tft.setTextSize(4);
    tft.print(" B");

    lastPressureDisplay = pressureDisplay;
  }

  // ── Water level % + bar (only redraw when integer % changes) ─────────────
  if (waterDisplay != lastWater) {
    // Percentage label
    tft.fillRect(165, 10, 75, 30, ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(168, 15);
    tft.print(waterDisplay);
    tft.print("%");

    // Vertical bar graph
    const int barX = 185, barY = 55, barW = 40, barH = 160;

    // Clear interior (leave 1-px border)
    tft.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, ST77XX_BLACK);

    // Fill from the bottom
    int fillH = (int)((waterDisplay / 100.0) * (barH - 2));
    if (fillH > 0) {
      tft.fillRect(barX + 1,
                   barY + barH - 1 - fillH,
                   barW - 2,
                   fillH,
                   ST77XX_BLUE);
    }

    // Redraw border (interior fill erases the inner edge of the rect)
    tft.drawRect(barX, barY, barW, barH, ST77XX_WHITE);

    lastWater = waterDisplay;
  }
}
