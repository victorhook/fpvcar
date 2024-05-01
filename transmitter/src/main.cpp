#include <Arduino.h>
#include <SPIFFS.h>

// Audio
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// Display
#include "Adafruit_GFX.h"    // Core graphics library
#include "Adafruit_ST7735.h" // Hardware-specific library for ST7735


// -- Pin definitions -- //
#define PIN_LED_RED   GPIO_NUM_9
#define PIN_LED_GREEN GPIO_NUM_46

#define PIN_I2S_BCK_IO  GPIO_NUM_48  // BCK pin
#define PIN_I2S_WS_IO   GPIO_NUM_45  // WS (word select) pin (LRC)
#define PIN_I2S_DO_IO   GPIO_NUM_18  // Data out pin

#define PIN_TFT_CS     38  // Chip select line for TFT display
#define PIN_TFT_RST    39  // Reset line for TFT (could connect to Arduino RESET pin)
#define PIN_TFT_DC     40  // Data/command line for TFT
#define PIN_TFT_MOSI  35  // Data out
#define PIN_TFT_SCLK  36  // Clock out

// -- Type definitions -- //
typedef enum
{
    ERROR_SPIFFS_INIT = 1
} device_error_t;

// -- Function definitions -- //
static void goto_error();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);


// -- Instantiations -- //
static uint32_t status = 0;

AudioFileSourceSPIFFS *file;
AudioGeneratorMP3 *mp3;
AudioOutputI2S *out;

Adafruit_ST7735 tft = Adafruit_ST7735(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_MOSI, PIN_TFT_SCLK, PIN_TFT_RST);


void setup()
{
    Serial.begin(115200);
    Serial.println("Booting up!");
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);

    if (!SPIFFS.begin())
    {
        status |= ERROR_SPIFFS_INIT;
    }

    if (status != 0)
    {
        goto_error();
    }

    file = new AudioFileSourceSPIFFS("/song.mp3");
    out = new AudioOutputI2S();
    out->SetPinout(PIN_I2S_BCK_IO, PIN_I2S_WS_IO, PIN_I2S_DO_IO);
    mp3 = new AudioGeneratorMP3();
    out->SetGain(0.1);

    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_RED, HIGH);

    pinMode(41, OUTPUT);
    digitalWrite(41, HIGH);


    tft.initR(INITR_GREENTAB);      // Init ST7735S chip, black tab
    tft.fillScreen(ST7735_BLACK);   // Fill screen with black

    // Text example
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.println("Hello, world!");

    // Rectangle example
    tft.drawRect(10, 20, 50, 40, ST7735_WHITE);

    // Circle example
    tft.drawCircle(90, 50, 10, ST7735_RED);

    //mp3->begin(file, out);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.println("MP3 playback finished");
    delay(1000);
  }
}

static void goto_error()
{
    Serial.printf("Error: %d\n", status);
    digitalWrite(PIN_LED_RED, HIGH);
    delay(500);
    digitalWrite(PIN_LED_RED, LOW);
    delay(500);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}