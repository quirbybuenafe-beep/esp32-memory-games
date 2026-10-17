#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>

// ============================================================
// MX CLUB GAMES - MEMORY//4
// ESP32 + ST7789 240x320 + HW-171 PCF8574 + PASSIVE BUZZER
//
// PCF8574 address confirmed: 0x27
//
// PCF buttons:
// P0 START
// P1 STOP
// P2 UP
// P3 LEFT
// P4 DOWN
// P5 RIGHT
//
// TFT:
// MOSI/SDA = GPIO23
// CLK/SCL  = GPIO18
// CS       = GPIO5
// DC       = GPIO27
// RST      = GPIO33
//
// PCF8574:
// SDA = GPIO21
// SCL = GPIO22
//
// Active buzzer:
// SIG = GPIO15
// ============================================================

// ---------------- TFT ----------------
#define TFT_CS    5
#define TFT_DC    27
#define TFT_RST   33
#define TFT_MOSI  23
#define TFT_SCLK  18

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// ---------------- PCF8574 ----------------
#define PCF_ADDR 0x27
#define I2C_SDA  21
#define I2C_SCL  22

#define BTN_START 0
#define BTN_STOP  1
#define BTN_UP    2
#define BTN_LEFT  3
#define BTN_DOWN  4
#define BTN_RIGHT 5

// ---------------- PASSIVE BUZZER ----------------
#define BUZZER_PIN 15

// ---------------- GAME ----------------
#define MAX_SEQUENCE 64

uint8_t sequenceData[MAX_SEQUENCE];
int sequenceLength = 0;
int roundNumber = 0;
uint16_t score = 0;
uint16_t highScore = 0;

Preferences prefs;

// ============================================================
// COLORS
// ============================================================

uint16_t BG;
uint16_t PANEL;
uint16_t PANEL2;
uint16_t CYAN;
uint16_t PINK;
uint16_t GREEN;
uint16_t YELLOW;
uint16_t PURPLE;
uint16_t REDCOLOR;
uint16_t MUTED;

uint16_t LOGO_PURPLE;
uint16_t LOGO_WHITE;
uint16_t LOGO_LIGHT;

// ============================================================
// EMBEDDED MX CLUB LOGO
// 80x80, 2 bits per pixel.
// 0 = transparent
// 1 = dark purple
// 2 = off-white
// 3 = light purple
//
// The white OUTSIDE background from your uploaded logo was removed.
// This is all inside this one sketch. No .h file is needed.
// ============================================================

#define MX_LOGO_W 80
#define MX_LOGO_H 80

const uint8_t mxLogo2bpp[1600] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x55, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x01, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x15, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x50, 0x00, 0x55, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x15, 0x55, 0x01, 0x55, 0x55, 0x55, 0x7F, 0xF5, 0x55, 0x55, 0x55, 0x01, 0x55, 0x50, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0x57, 0xEA, 0xAA, 0xAA, 0xBD, 0x55, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x55, 0x55, 0x55, 0x55, 0xFA, 0xAB, 0xFB, 0xFA, 0xAA, 0xB5, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0x55, 0x55, 0x5E, 0xAB, 0xFF, 0xAA, 0xAB, 0xFE, 0xAA, 0xD5, 0x55, 0x55, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x55, 0x55, 0xEA, 0xBB, 0xAA, 0xBF, 0xFA, 0xAA, 0xFA, 0xA5, 0x55, 0x55, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0x57, 0xAE, 0xAA, 0xFF, 0xFF, 0xFF, 0xFE, 0xAF, 0xAB, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0x7A, 0xFA, 0xBF, 0xEA, 0xAA, 0xAA, 0xAF, 0xFA, 0xFA, 0x95, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0xEA, 0xAB, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBF, 0xAB, 0xAD, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x57, 0xAF, 0xAF, 0xAA, 0xAA, 0xAE, 0xEA, 0xAA, 0xAB, 0xEB, 0xAB, 0x55, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x5E, 0xFA, 0xFA, 0xAA, 0xAA, 0xBA, 0xBA, 0xAA, 0xAA, 0xBE, 0xFA, 0xD5, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0x7B, 0xEB, 0xEA, 0xAA, 0xAA, 0xEA, 0xAE, 0xAA, 0xAA, 0xAF, 0xAF, 0xB5, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0xEB, 0xE7, 0xAA, 0xAA, 0xAA, 0xEA, 0xAE, 0xAA, 0xAA, 0xAB, 0x6B, 0xED, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x57, 0xAE, 0x9E, 0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xDA, 0xE9, 0x55, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x56, 0xBA, 0x7A, 0xAA, 0xAA, 0xAD, 0x55, 0x55, 0xEA, 0xAA, 0xAA, 0xB6, 0xBA, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x5E, 0xFB, 0xEA, 0xAA, 0xBF, 0xB5, 0x55, 0x55, 0x7B, 0xFA, 0xAA, 0xAF, 0xAE, 0xD5, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x5A, 0xEF, 0xAA, 0xAA, 0x55, 0xD5, 0x55, 0x55, 0x5D, 0x56, 0xAA, 0xAB, 0xEE, 0x95, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x6B, 0xA6, 0xAA, 0xA9, 0x55, 0xD5, 0x55, 0x55, 0x5D, 0x55, 0xAA, 0xAA, 0x6B, 0xB5, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xEB, 0xBE, 0xAA, 0xA9, 0x55, 0xF5, 0x55, 0x55, 0x7D, 0x55, 0xAA, 0xAA, 0xFA, 0xA5, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xEA, 0xBA, 0xFF, 0xFD, 0x5F, 0xFD, 0x55, 0x55, 0xFF, 0xD5, 0xFF, 0xFE, 0xBA, 0xAD, 0x54, 0x00, 0x00,
  0x00, 0x01, 0x55, 0xAA, 0xEB, 0xAA, 0xAD, 0x7D, 0x5F, 0x55, 0x57, 0xD5, 0xF5, 0xEA, 0xAB, 0xAE, 0xA9, 0x55, 0x00, 0x00,
  0x00, 0x01, 0x57, 0xAA, 0xEA, 0xAA, 0xA9, 0x75, 0x57, 0x55, 0x57, 0x55, 0x75, 0xAA, 0xAA, 0xAE, 0xAB, 0x55, 0x00, 0x00,
  0x00, 0x01, 0x56, 0xAB, 0xEB, 0xAA, 0xA9, 0x75, 0x57, 0x55, 0x57, 0x55, 0x75, 0xAA, 0xAB, 0xAF, 0xAB, 0x55, 0x00, 0x00,
  0x00, 0x15, 0x56, 0xAB, 0xAB, 0xAA, 0xA9, 0x7D, 0x5F, 0x55, 0x57, 0xD5, 0xF5, 0xAA, 0xB7, 0xAB, 0xAA, 0x55, 0x50, 0x00,
  0x05, 0x55, 0x5E, 0xAB, 0xAA, 0xEA, 0xA9, 0x5F, 0xFD, 0x55, 0x55, 0xFF, 0xD5, 0xAA, 0xD5, 0xEB, 0xAA, 0xD5, 0x55, 0x40,
  0x55, 0x55, 0x5E, 0xAF, 0xAA, 0xBA, 0xA9, 0x57, 0xF5, 0x55, 0x55, 0x7F, 0x55, 0xAA, 0x7F, 0xEB, 0xEA, 0xD5, 0x55, 0x54,
  0x55, 0x55, 0x5E, 0xAF, 0xAA, 0xAE, 0xA9, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xD5, 0xAB, 0xEA, 0xD5, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAB, 0xAB, 0x55, 0x55, 0x5E, 0xD5, 0x55, 0x57, 0xAB, 0xBE, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xEA, 0xBF, 0xDF, 0x7A, 0xB7, 0xDF, 0xFA, 0xAE, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xBE, 0xAA, 0xFA, 0xAA, 0xAA, 0xBE, 0xAA, 0xFA, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xA5, 0xAF, 0xBA, 0xEA, 0xAE, 0xBB, 0xE9, 0x6A, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xAD, 0xF7, 0xA9, 0x7A, 0xB5, 0xAB, 0x7D, 0xEA, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xAD, 0x55, 0xB5, 0x5E, 0xD5, 0x79, 0x55, 0xEA, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xAD, 0x55, 0xB5, 0x57, 0x55, 0x79, 0x55, 0xEA, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5A, 0xAE, 0xAA, 0xAA, 0xB9, 0x55, 0xD5, 0x57, 0x55, 0x5F, 0x55, 0xFA, 0xAA, 0xAA, 0xEA, 0x95, 0x55, 0x54,
  0x55, 0x55, 0x5E, 0xAE, 0xAA, 0xAA, 0xE9, 0x57, 0x95, 0x57, 0x55, 0x5B, 0x55, 0xAE, 0xAA, 0xAA, 0xEA, 0xD5, 0x55, 0x54,
  0x55, 0x55, 0x5E, 0xAF, 0xAA, 0xDF, 0xA9, 0x56, 0x95, 0x57, 0x55, 0x5A, 0x55, 0xAB, 0xAA, 0xAB, 0xEA, 0xD5, 0x55, 0x54,
  0x01, 0x55, 0x5E, 0xAF, 0xAB, 0x77, 0xAB, 0x56, 0x95, 0x57, 0x55, 0x5A, 0x57, 0xAA, 0xEA, 0xAB, 0xAA, 0xD5, 0x55, 0x00,
  0x00, 0x0D, 0x56, 0xAB, 0xAB, 0xFF, 0xAB, 0x56, 0xD5, 0x57, 0x55, 0x5E, 0xD7, 0xAA, 0xBA, 0xAB, 0xAA, 0x55, 0x40, 0x00,
  0x00, 0x01, 0x56, 0xAB, 0xAA, 0x56, 0xAB, 0x5E, 0xD5, 0x57, 0x55, 0x5E, 0xD7, 0xAA, 0xAE, 0xAB, 0xAA, 0x55, 0x00, 0x00,
  0x00, 0x01, 0x57, 0xAB, 0xEA, 0xFA, 0xAB, 0x5E, 0xD5, 0x57, 0x55, 0x56, 0xD7, 0xAA, 0xAE, 0xAF, 0xAB, 0x55, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xAA, 0xEB, 0xAA, 0xAA, 0x5A, 0x55, 0x57, 0x55, 0x56, 0x96, 0xAA, 0xAB, 0xAE, 0xA9, 0x55, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xAA, 0xFB, 0xAA, 0xAA, 0x5F, 0x55, 0x57, 0x55, 0x57, 0xD6, 0xAA, 0xAB, 0xBE, 0xA9, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0xEA, 0x9A, 0xEA, 0xAB, 0x5B, 0x55, 0x57, 0x55, 0x57, 0xB7, 0xAA, 0xAE, 0xBA, 0xAD, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x6A, 0xBE, 0xBF, 0xFA, 0xFB, 0x55, 0x57, 0x55, 0x57, 0xBE, 0xBF, 0xFA, 0xFA, 0xA5, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x7A, 0xAF, 0xAA, 0xAA, 0xAB, 0x55, 0x57, 0x55, 0x55, 0xAA, 0xAA, 0xAB, 0xEA, 0xB5, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x5E, 0xA9, 0xAA, 0xAA, 0xA9, 0x55, 0x57, 0x55, 0x55, 0xAA, 0xAA, 0xA9, 0xAA, 0x95, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x56, 0xAA, 0x6A, 0xAA, 0xAB, 0x55, 0x57, 0x55, 0x57, 0xAA, 0xAA, 0xAE, 0xAA, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x57, 0xAA, 0xFA, 0xAA, 0xAA, 0xB5, 0x57, 0x55, 0x7A, 0xAA, 0xAA, 0xBE, 0xAB, 0x55, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0xEA, 0xBE, 0xAA, 0xAA, 0xAB, 0xD7, 0x57, 0xAA, 0xAA, 0xAA, 0xFA, 0xAD, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0x6A, 0xAD, 0xAA, 0xAA, 0xAA, 0xEA, 0xAE, 0xAA, 0xAA, 0xAB, 0xEA, 0xA5, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x5A, 0xAA, 0x7A, 0xAA, 0xAA, 0xEA, 0xAE, 0xAA, 0xAA, 0xB6, 0xAA, 0x95, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x56, 0xAA, 0xBE, 0xAA, 0xAA, 0xBA, 0xBA, 0xAA, 0xAA, 0xFA, 0xAA, 0x55, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0xAA, 0xAF, 0xEA, 0xAA, 0xBE, 0xBA, 0xAA, 0xAD, 0xEA, 0xAD, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0x7A, 0xAA, 0xFF, 0xAA, 0xAB, 0xAA, 0xAB, 0xFE, 0xAA, 0xB5, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x55, 0x55, 0x55, 0x5E, 0xAB, 0xAB, 0xFF, 0xAA, 0xAB, 0xFF, 0xAB, 0xEA, 0xD5, 0x55, 0x55, 0x54, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x55, 0x55, 0x55, 0xEB, 0xFA, 0xAF, 0xFF, 0xFF, 0xEA, 0xAF, 0xED, 0x55, 0x55, 0x55, 0x50, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x55, 0x55, 0x55, 0x7A, 0xBF, 0xEA, 0xAA, 0xAA, 0xAB, 0xEA, 0xB5, 0x55, 0x55, 0x55, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x55, 0x55, 0x55, 0x57, 0xEA, 0xFF, 0xFF, 0xFF, 0xEA, 0xAF, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0x5F, 0xAA, 0xAE, 0xEA, 0xAB, 0xD5, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x15, 0x55, 0x01, 0x55, 0x55, 0x7F, 0xEA, 0xAF, 0xF5, 0x55, 0x55, 0x01, 0x55, 0x50, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x05, 0x50, 0x00, 0x15, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x50, 0x00, 0x15, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x55, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ============================================================
// DIFFICULTY
// ============================================================

enum Difficulty {
  EASY = 0,
  NORMAL = 1,
  HARD = 2,
  INSANE = 3
};

Difficulty difficulty = NORMAL;

const char* difficultyName() {
  switch (difficulty) {
    case EASY:   return "EASY";
    case NORMAL: return "NORMAL";
    case HARD:   return "HARD";
    case INSANE: return "INSANE";
  }
  return "NORMAL";
}

const char* difficultyShort() {
  switch (difficulty) {
    case EASY:   return "E";
    case NORMAL: return "N";
    case HARD:   return "H";
    case INSANE: return "X";
  }
  return "N";
}

uint16_t difficultyColor() {
  switch (difficulty) {
    case EASY:   return GREEN;
    case NORMAL: return CYAN;
    case HARD:   return YELLOW;
    case INSANE: return PINK;
  }
  return CYAN;
}

int scoreMultiplier() {
  switch (difficulty) {
    case EASY:   return 1;
    case NORMAL: return 2;
    case HARD:   return 3;
    case INSANE: return 5;
  }
  return 1;
}

int sequenceSpeed() {
  int baseSpeed;
  int dropPerRound;
  int minimumSpeed;

  switch (difficulty) {
    case EASY:
      baseSpeed = 850;
      dropPerRound = 12;
      minimumSpeed = 380;
      break;

    case NORMAL:
      baseSpeed = 620;
      dropPerRound = 18;
      minimumSpeed = 220;
      break;

    case HARD:
      baseSpeed = 430;
      dropPerRound = 18;
      minimumSpeed = 140;
      break;

    default: // INSANE
      baseSpeed = 280;
      dropPerRound = 10;
      minimumSpeed = 90;
      break;
  }

  int value = baseSpeed - (roundNumber * dropPerRound);

  if (value < minimumSpeed) {
    value = minimumSpeed;
  }

  return value;
}

// ============================================================
// PCF8574
// ============================================================

bool checkPCF() {
  Wire.beginTransmission(PCF_ADDR);
  return Wire.endTransmission() == 0;
}

void initPCF() {
  Wire.beginTransmission(PCF_ADDR);

  // PCF8574 pins are inputs when written HIGH.
  Wire.write(0xFF);

  Wire.endTransmission();
}

uint8_t readPCF() {
  Wire.requestFrom((int)PCF_ADDR, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF;
}

bool buttonPressed(uint8_t pin) {
  return bitRead(readPCF(), pin) == LOW;
}

void waitButtonRelease(uint8_t pin) {
  while (buttonPressed(pin)) {
    delay(4);
  }

  delay(22);
}

void waitAllReleased() {
  while ((readPCF() & 0x3F) != 0x3F) {
    delay(4);
  }

  delay(22);
}

bool confirmedPress(uint8_t pin) {
  if (!buttonPressed(pin)) {
    return false;
  }

  delay(20);

  if (!buttonPressed(pin)) {
    return false;
  }

  waitButtonRelease(pin);
  return true;
}

// ============================================================
// PASSIVE BUZZER - REAL MUSICAL TONES
// ============================================================

void buzzerOff() {
  noTone(BUZZER_PIN);
}

void playToneHz(
  int frequency,
  int durationMs
) {
  tone(
    BUZZER_PIN,
    frequency
  );

  delay(durationMs);

  noTone(BUZZER_PIN);
}

void directionTone(
  int direction
) {
  // Simon-style musical notes
  //
  // UP    = C6
  // LEFT  = E6
  // DOWN  = G6
  // RIGHT = C7

  switch (direction) {

    case 0:
      tone(BUZZER_PIN, 1047);
      break;

    case 1:
      tone(BUZZER_PIN, 1319);
      break;

    case 2:
      tone(BUZZER_PIN, 1568);
      break;

    default:
      tone(BUZZER_PIN, 2093);
      break;
  }
}

void soundStart() {
  playToneHz(988, 90);
  delay(35);

  playToneHz(1319, 90);
  delay(35);

  playToneHz(1760, 140);
}

void soundSelect() {
  playToneHz(1760, 55);
}

void soundCorrect() {
  playToneHz(1319, 70);
  delay(25);

  playToneHz(1568, 70);
  delay(25);

  playToneHz(2093, 130);
}

void soundWrong() {
  playToneHz(392, 180);
  delay(35);

  playToneHz(294, 260);
}

// ============================================================
// TEXT HELPERS
// ============================================================

void centerText(
  const String &text,
  int y,
  int size,
  uint16_t color
) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.setTextWrap(false);

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &w,
    &h
  );

  tft.setCursor(
    (240 - w) / 2,
    y
  );

  tft.print(text);
}

// ============================================================
// DRAW EMBEDDED TRANSPARENT MX CLUB LOGO
// ============================================================

void drawMXLogo(int x0, int y0) {
  for (int y = 0; y < MX_LOGO_H; y++) {
    for (int x = 0; x < MX_LOGO_W; x++) {

      int pixelIndex = y * MX_LOGO_W + x;
      int byteIndex = pixelIndex >> 2;
      int shift = 6 - ((pixelIndex & 3) * 2);

      uint8_t packed =
        pgm_read_byte(&mxLogo2bpp[byteIndex]);

      uint8_t value =
        (packed >> shift) & 0x03;

      if (value == 0) {
        // transparent
        continue;
      }

      uint16_t color;

      if (value == 1) {
        color = LOGO_PURPLE;
      }
      else if (value == 2) {
        color = LOGO_WHITE;
      }
      else {
        color = LOGO_LIGHT;
      }

      tft.drawPixel(
        x0 + x,
        y0 + y,
        color
      );
    }
  }
}

// ============================================================
// BACKGROUND
// ============================================================

void drawBackground() {
  tft.fillScreen(BG);

  tft.fillRect(
    0,
    0,
    240,
    3,
    CYAN
  );

  tft.fillRect(
    0,
    317,
    240,
    3,
    PINK
  );
}

// ============================================================
// BOOT SCREEN
// ============================================================

void bootScreen() {
  drawBackground();

  drawMXLogo(
    80,
    24
  );

  centerText(
    "MX CLUB GAMES",
    126,
    2,
    ST77XX_WHITE
  );

  centerText(
    "MECHATRONIX",
    153,
    1,
    PURPLE
  );

  centerText(
    "MEMORY//4",
    186,
    3,
    CYAN
  );

  tft.drawRoundRect(
    35,
    235,
    170,
    14,
    5,
    PURPLE
  );

  for (int w = 0; w <= 162; w += 6) {
    tft.fillRoundRect(
      39,
      239,
      w,
      6,
      3,
      PURPLE
    );

    delay(10);
  }

  centerText(
    "SYSTEM READY",
    272,
    1,
    GREEN
  );

  delay(650);
}

// ============================================================
// TITLE SCREEN
// Drawn once. It does NOT continuously redraw the screen.
// ============================================================

void titleScreen() {
  waitAllReleased();

  drawBackground();

  drawMXLogo(
    80,
    14
  );

  centerText(
    "MX CLUB GAMES",
    111,
    2,
    ST77XX_WHITE
  );

  centerText(
    "MEMORY//4",
    151,
    3,
    CYAN
  );

  centerText(
    "WATCH  REMEMBER  REPEAT",
    196,
    1,
    MUTED
  );

  tft.fillRoundRect(
    32,
    235,
    176,
    48,
    10,
    PANEL
  );

  tft.drawRoundRect(
    32,
    235,
    176,
    48,
    10,
    GREEN
  );

  centerText(
    "PRESS START",
    251,
    2,
    GREEN
  );

  centerText(
    "P0",
    288,
    1,
    MUTED
  );

  while (true) {
    if (confirmedPress(BTN_START)) {
      soundStart();
      return;
    }

    delay(5);
  }
}

// ============================================================
// DIFFICULTY MENU
// ============================================================

uint16_t diffColorFor(int index) {
  switch (index) {
    case EASY:   return GREEN;
    case NORMAL: return CYAN;
    case HARD:   return YELLOW;
    default:     return PINK;
  }
}

const char* diffNameFor(int index) {
  switch (index) {
    case EASY:   return "EASY";
    case NORMAL: return "NORMAL";
    case HARD:   return "HARD";
    default:     return "INSANE";
  }
}

void drawDifficultyOption(
  int index,
  bool selected
) {
  int y = 94 + (index * 44);
  uint16_t c = diffColorFor(index);

  if (selected) {
    tft.fillRoundRect(
      36,
      y,
      168,
      34,
      8,
      c
    );

    tft.setTextColor(BG);
  }
  else {
    tft.fillRoundRect(
      36,
      y,
      168,
      34,
      8,
      PANEL
    );

    tft.drawRoundRect(
      36,
      y,
      168,
      34,
      8,
      c
    );

    tft.setTextColor(c);
  }

  tft.setTextSize(2);

  String text =
    diffNameFor(index);

  int16_t x1, y1;
  uint16_t tw, th;

  tft.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &tw,
    &th
  );

  tft.setCursor(
    (240 - tw) / 2,
    y + 9
  );

  tft.print(text);
}

void drawDifficultyMenu() {
  drawBackground();

  centerText(
    "SELECT MODE",
    26,
    3,
    ST77XX_WHITE
  );

  centerText(
    "UP / DOWN TO SELECT",
    68,
    1,
    MUTED
  );

  for (int i = 0; i < 4; i++) {
    drawDifficultyOption(
      i,
      i == (int)difficulty
    );
  }

  centerText(
    "START=PLAY   STOP=BACK",
    286,
    1,
    ST77XX_WHITE
  );
}

bool chooseDifficulty() {
  waitAllReleased();

  drawDifficultyMenu();

  while (true) {

    if (confirmedPress(BTN_UP)) {
      int oldIndex =
        (int)difficulty;

      int nextIndex =
        oldIndex - 1;

      if (nextIndex < 0) {
        nextIndex = 3;
      }

      difficulty =
        (Difficulty)nextIndex;

      drawDifficultyOption(
        oldIndex,
        false
      );

      drawDifficultyOption(
        nextIndex,
        true
      );

      soundSelect();
    }

    if (confirmedPress(BTN_DOWN)) {
      int oldIndex =
        (int)difficulty;

      int nextIndex =
        oldIndex + 1;

      if (nextIndex > 3) {
        nextIndex = 0;
      }

      difficulty =
        (Difficulty)nextIndex;

      drawDifficultyOption(
        oldIndex,
        false
      );

      drawDifficultyOption(
        nextIndex,
        true
      );

      soundSelect();
    }

    if (confirmedPress(BTN_START)) {
      soundStart();
      return true;
    }

    if (confirmedPress(BTN_STOP)) {
      return false;
    }

    delay(4);
  }
}

// ============================================================
// GAME HEADER
// ============================================================

void drawHeaderFrame() {
  tft.fillRoundRect(
    8,
    9,
    224,
    42,
    8,
    PANEL
  );

  tft.drawRoundRect(
    8,
    9,
    224,
    42,
    8,
    PURPLE
  );
}

void updateHeader() {
  // Only clear the inside of the header.
  // This prevents a full-screen blink.
  tft.fillRect(
    15,
    17,
    210,
    26,
    PANEL
  );

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(17, 27);
  tft.print("R:");
  tft.print(roundNumber);

  tft.setCursor(67, 27);
  tft.print("S:");
  tft.print(score);

  tft.setCursor(130, 27);
  tft.print("HI:");
  tft.print(highScore);

  tft.setTextColor(
    difficultyColor()
  );

  tft.setCursor(208, 27);
  tft.print(
    difficultyShort()
  );
}

// ============================================================
// STATUS STRIP
// ============================================================

void drawStatus(
  const String &text,
  uint16_t color
) {
  // Only this strip is repainted.
  tft.fillRoundRect(
    20,
    57,
    200,
    23,
    6,
    PANEL2
  );

  tft.drawRoundRect(
    20,
    57,
    200,
    23,
    6,
    color
  );

  tft.setTextSize(1);
  tft.setTextColor(color);

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &w,
    &h
  );

  tft.setCursor(
    (240 - w) / 2,
    65
  );

  tft.print(text);
}

// ============================================================
// D-PAD
//
// game direction:
// 0 = UP
// 1 = LEFT
// 2 = DOWN
// 3 = RIGHT
// ============================================================

void directionGeometry(
  int direction,
  int &x,
  int &y,
  int &w,
  int &h
) {
  w = 66;
  h = 52;

  switch (direction) {
    case 0:
      x = 87;
      y = 86;
      break;

    case 1:
      x = 18;
      y = 146;
      break;

    case 2:
      x = 87;
      y = 206;
      break;

    default:
      x = 156;
      y = 146;
      break;
  }
}

uint16_t directionColor(
  int direction
) {
  switch (direction) {
    case 0:
      return CYAN;

    case 1:
      return PINK;

    case 2:
      return GREEN;

    default:
      return YELLOW;
  }
}

char directionSymbol(
  int direction
) {
  switch (direction) {
    case 0:
      return '^';

    case 1:
      return '<';

    case 2:
      return 'v';

    default:
      return '>';
  }
}

const char* directionLabel(
  int direction
) {
  switch (direction) {
    case 0:
      return "UP";

    case 1:
      return "LEFT";

    case 2:
      return "DOWN";

    default:
      return "RIGHT";
  }
}

// ============================================================
// DRAW ONE DIRECTION BUTTON
//
// IMPORTANT:
// This function redraws ONE pad only.
// It does not clear the whole screen.
// ============================================================

void drawDirection(
  int direction,
  bool active
) {
  int x, y, w, h;

  directionGeometry(
    direction,
    x,
    y,
    w,
    h
  );

  uint16_t c =
    directionColor(direction);

  if (active) {
    tft.fillRoundRect(
      x,
      y,
      w,
      h,
      9,
      c
    );

    tft.drawRoundRect(
      x,
      y,
      w,
      h,
      9,
      ST77XX_WHITE
    );

    tft.setTextColor(BG);
  }
  else {
    tft.fillRoundRect(
      x,
      y,
      w,
      h,
      9,
      PANEL
    );

    tft.drawRoundRect(
      x,
      y,
      w,
      h,
      9,
      c
    );

    tft.setTextColor(c);
  }

  tft.setTextSize(3);

  tft.setCursor(
    x + 24,
    y + 6
  );

  tft.print(
    directionSymbol(direction)
  );

  tft.setTextSize(1);

  String label =
    directionLabel(direction);

  int16_t x1, y1;
  uint16_t tw, th;

  tft.getTextBounds(
    label,
    0,
    0,
    &x1,
    &y1,
    &tw,
    &th
  );

  tft.setCursor(
    x + (w - tw) / 2,
    y + 38
  );

  tft.print(label);
}

// ============================================================
// BOTTOM START / STOP LABELS
// ============================================================

void drawControlBar() {
  tft.fillRoundRect(
    11,
    270,
    103,
    35,
    7,
    PANEL
  );

  tft.drawRoundRect(
    11,
    270,
    103,
    35,
    7,
    GREEN
  );

  tft.fillRoundRect(
    126,
    270,
    103,
    35,
    7,
    PANEL
  );

  tft.drawRoundRect(
    126,
    270,
    103,
    35,
    7,
    PINK
  );

  tft.setTextSize(1);

  tft.setTextColor(GREEN);
  tft.setCursor(31, 284);
  tft.print("START P0");

  tft.setTextColor(PINK);
  tft.setCursor(149, 284);
  tft.print("STOP P1");
}

// ============================================================
// DRAW STATIC GAME SCREEN ONCE
// ============================================================

void drawGameLayout() {
  drawBackground();

  drawHeaderFrame();
  updateHeader();

  drawStatus(
    "GET READY",
    CYAN
  );

  for (int i = 0; i < 4; i++) {
    drawDirection(
      i,
      false
    );
  }

  drawControlBar();
}

// ============================================================
// STOP-AWARE DELAY
// ============================================================

bool delayWithStop(
  int durationMs
) {
  unsigned long started =
    millis();

  while (
    millis() - started <
    (unsigned long)durationMs
  ) {

    if (buttonPressed(BTN_STOP)) {
      delay(20);

      if (buttonPressed(BTN_STOP)) {
        waitButtonRelease(BTN_STOP);

        buzzerOff();

        return true;
      }
    }

    delay(4);
  }

  return false;
}

// ============================================================
// FLASH ONE DIRECTION
//
// No fillScreen() here.
// Only the active pad is redrawn.
// ============================================================

bool flashDirection(
  int direction,
  int durationMs
) {
  drawDirection(
    direction,
    true
  );

  directionTone(direction);

  if (delayWithStop(durationMs)) {
    buzzerOff();

    drawDirection(
      direction,
      false
    );

    return false;
  }

  buzzerOff();

  drawDirection(
    direction,
    false
  );

  if (delayWithStop(85)) {
    return false;
  }

  return true;
}

// ============================================================
// SHOW MEMORY SEQUENCE
// ============================================================

bool showSequence() {
  updateHeader();

  drawStatus(
    String("ROUND ") +
    roundNumber +
    "  WATCH",
    PINK
  );

  if (delayWithStop(420)) {
    return false;
  }

  int speedMs =
    sequenceSpeed();

  for (
    int i = 0;
    i < sequenceLength;
    i++
  ) {
    if (
      !flashDirection(
        sequenceData[i],
        speedMs
      )
    ) {
      return false;
    }
  }

  drawStatus(
    "YOUR TURN",
    GREEN
  );

  return true;
}

// ============================================================
// WAIT FOR PLAYER DIRECTION
//
// returns:
// 0 = UP
// 1 = LEFT
// 2 = DOWN
// 3 = RIGHT
// -1 = STOP
// ============================================================

int waitDirection() {
  while (true) {

    if (confirmedPress(BTN_STOP)) {
      return -1;
    }

    if (confirmedPress(BTN_UP)) {
      return 0;
    }

    if (confirmedPress(BTN_LEFT)) {
      return 1;
    }

    if (confirmedPress(BTN_DOWN)) {
      return 2;
    }

    if (confirmedPress(BTN_RIGHT)) {
      return 3;
    }

    delay(3);
  }
}

// ============================================================
// PLAYER TURN
//
// returns:
//  1 = correct
//  0 = wrong
// -1 = stop
// ============================================================

int playerTurn() {
  for (
    int i = 0;
    i < sequenceLength;
    i++
  ) {
    int pressed =
      waitDirection();

    if (pressed == -1) {
      return -1;
    }

    // Only redraw the button the player pressed.
    drawDirection(
      pressed,
      true
    );

    directionTone(pressed);

    delay(95);

    buzzerOff();

    drawDirection(
      pressed,
      false
    );

    if (
      pressed !=
      sequenceData[i]
    ) {
      return 0;
    }
  }

  return 1;
}

// ============================================================
// GAME FEEDBACK
//
// Again: only the little status strip changes.
// ============================================================

void successFeedback(
  uint16_t gained
) {
  drawStatus(
    String("CLEAR!  +") + gained,
    GREEN
  );

  soundCorrect();

  delay(420);
}

void wrongFeedback() {
  drawStatus(
    "WRONG!",
    REDCOLOR
  );

  soundWrong();

  delay(500);
}

void stoppedFeedback() {
  drawStatus(
    "STOPPED - MENU",
    PINK
  );

  delay(450);
}

// ============================================================
// GAME DATA
// ============================================================

void resetGame() {
  sequenceLength = 0;
  roundNumber = 0;
  score = 0;

  memset(
    sequenceData,
    0,
    sizeof(sequenceData)
  );
}

void addStep() {
  if (
    sequenceLength >=
    MAX_SEQUENCE
  ) {
    return;
  }

  sequenceData[
    sequenceLength
  ] = random(0, 4);

  sequenceLength++;
}

void saveHighScoreIfNeeded() {
  if (score > highScore) {
    highScore = score;

    prefs.putUShort(
      "highscore",
      highScore
    );
  }
}

// ============================================================
// GAME OVER
//
// START = replay same difficulty
// STOP  = title screen
// ============================================================

bool gameOverScreen() {
  bool newRecord =
    score > highScore;

  saveHighScoreIfNeeded();

  drawBackground();

  centerText(
    "MX CLUB GAMES",
    26,
    2,
    MUTED
  );

  centerText(
    "GAME OVER",
    68,
    4,
    PINK
  );

  tft.fillRoundRect(
    29,
    126,
    182,
    98,
    12,
    PANEL
  );

  tft.drawRoundRect(
    29,
    126,
    182,
    98,
    12,
    PURPLE
  );

  tft.setTextSize(2);
  tft.setTextColor(
    ST77XX_WHITE
  );

  tft.setCursor(54, 145);
  tft.print("SCORE ");
  tft.print(score);

  tft.setCursor(54, 176);
  tft.print("ROUND ");
  tft.print(roundNumber);

  tft.setCursor(54, 207);
  tft.print("BEST  ");
  tft.print(highScore);

  if (newRecord) {
    centerText(
      "NEW CLUB RECORD!",
      238,
      1,
      YELLOW
    );
  }
  else {
    centerText(
      difficultyName(),
      238,
      1,
      difficultyColor()
    );
  }

  tft.fillRoundRect(
    18,
    267,
    98,
    35,
    7,
    PANEL
  );

  tft.drawRoundRect(
    18,
    267,
    98,
    35,
    7,
    GREEN
  );

  tft.fillRoundRect(
    124,
    267,
    98,
    35,
    7,
    PANEL
  );

  tft.drawRoundRect(
    124,
    267,
    98,
    35,
    7,
    PINK
  );

  tft.setTextSize(1);

  tft.setTextColor(GREEN);
  tft.setCursor(34, 281);
  tft.print("START AGAIN");

  tft.setTextColor(PINK);
  tft.setCursor(145, 281);
  tft.print("STOP MENU");

  waitAllReleased();

  while (true) {

    if (confirmedPress(BTN_START)) {
      soundStart();
      return true;
    }

    if (confirmedPress(BTN_STOP)) {
      return false;
    }

    delay(4);
  }
}

// ============================================================
// WIN SCREEN
// ============================================================

bool winScreen() {
  saveHighScoreIfNeeded();

  drawBackground();

  centerText(
    "MX CLUB GAMES",
    35,
    2,
    MUTED
  );

  centerText(
    "MEMORY",
    82,
    4,
    YELLOW
  );

  centerText(
    "MASTER!",
    127,
    4,
    GREEN
  );

  centerText(
    "64 MOVES CLEARED",
    194,
    2,
    ST77XX_WHITE
  );

  centerText(
    "START=AGAIN  STOP=MENU",
    266,
    1,
    MUTED
  );

  waitAllReleased();

  while (true) {

    if (confirmedPress(BTN_START)) {
      soundStart();
      return true;
    }

    if (confirmedPress(BTN_STOP)) {
      return false;
    }

    delay(4);
  }
}

// ============================================================
// MAIN GAME
// ============================================================

void playGame() {
  while (true) {

    resetGame();

    // The full game layout is drawn ONE TIME here.
    drawGameLayout();

    bool replay = false;

    while (true) {
      roundNumber++;

      addStep();

      if (!showSequence()) {
        saveHighScoreIfNeeded();
        stoppedFeedback();
        return;
      }

      int result =
        playerTurn();

      // STOP
      if (result == -1) {
        saveHighScoreIfNeeded();
        stoppedFeedback();
        return;
      }

      // WRONG
      if (result == 0) {
        wrongFeedback();

        replay =
          gameOverScreen();

        break;
      }

      // CORRECT
      uint16_t gained =
        roundNumber *
        10 *
        scoreMultiplier();

      score += gained;

      updateHeader();

      successFeedback(
        gained
      );

      // 64 step win
      if (
        sequenceLength >=
        MAX_SEQUENCE
      ) {
        replay =
          winScreen();

        break;
      }

      drawStatus(
        "NEXT ROUND",
        CYAN
      );

      if (delayWithStop(350)) {
        saveHighScoreIfNeeded();
        stoppedFeedback();
        return;
      }
    }

    if (!replay) {
      return;
    }
  }
}

// ============================================================
// PCF ERROR SCREEN
// ============================================================

void pcfErrorScreen() {
  drawBackground();

  centerText(
    "PCF8574 ERROR",
    95,
    3,
    ST77XX_RED
  );

  centerText(
    "0x27 NOT FOUND",
    145,
    2,
    ST77XX_WHITE
  );

  centerText(
    "SDA=21  SCL=22",
    184,
    1,
    YELLOW
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);

  // ---------------- BUZZER ----------------
  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  buzzerOff();

  // ---------------- PCF / I2C ----------------
  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  delay(100);

  // ---------------- TFT / SPI ----------------
  SPI.begin(
    TFT_SCLK,
    -1,
    TFT_MOSI,
    TFT_CS
  );

  tft.init(
    240,
    320
  );

  // Portrait orientation.
  // Your earlier landscape needed 3 instead of 1,
  // so this uses portrait rotation 2.
  //
  // If THIS is upside down, change 2 to 0.
  tft.setRotation(2);

  // ---------------- COLORS ----------------
  BG =
    tft.color565(
      5,
      8,
      18
    );

  PANEL =
    tft.color565(
      15,
      21,
      39
    );

  PANEL2 =
    tft.color565(
      10,
      15,
      30
    );

  CYAN =
    tft.color565(
      0,
      220,
      255
    );

  PINK =
    tft.color565(
      255,
      45,
      145
    );

  GREEN =
    tft.color565(
      30,
      255,
      145
    );

  YELLOW =
    tft.color565(
      255,
      220,
      40
    );

  PURPLE =
    tft.color565(
      112,
      44,
      190
    );

  REDCOLOR =
    tft.color565(
      230,
      35,
      55
    );

  MUTED =
    tft.color565(
      145,
      155,
      180
    );

  // MX Club logo palette
  LOGO_PURPLE =
    tft.color565(
      49,
      8,
      89
    );

  LOGO_WHITE =
    tft.color565(
      235,
      232,
      225
    );

  LOGO_LIGHT =
    tft.color565(
      120,
      82,
      160
    );

  // ---------------- CHECK PCF ----------------
  if (!checkPCF()) {
    Serial.println(
      "ERROR: PCF8574 0x27 NOT FOUND"
    );

    pcfErrorScreen();

    while (true) {
      delay(1000);
    }
  }

  Serial.println(
    "PCF8574 FOUND AT 0x27"
  );

  initPCF();

  // ---------------- HIGH SCORE ----------------
  prefs.begin(
    "mxmemory4",
    false
  );

  highScore =
    prefs.getUShort(
      "highscore",
      0
    );

  randomSeed(
    esp_random()
  );

  bootScreen();
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  titleScreen();

  if (chooseDifficulty()) {
    playGame();
  }
}
