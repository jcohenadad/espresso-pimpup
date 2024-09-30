#include <RTClib.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

/* Create an rtc object */
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  delay(1000);  // Give some time for Serial communication to stabilize

  // Print to Serial Monitor before initializing the OLED
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
}

void loop()
{
  // Serial.println("Hello world!");
  // delay(1000);

  // Clear the display buffer
  display.clearDisplay();
  display.setCursor(0, 0);

  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  
  Serial.print("Celsius temperature: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire

  display.print(sensors.getTempCByIndex(0), 2);
  display.print(" C");

  // Ultrasound probe
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

  delay(1000);
}

void print2digits(int number) {
  if (number < 10) {
    display.print("0"); // Print a 0 before if the number is < 10
  }
  display.print(number);
}