#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>
#include "mx_logo_120.h"

// =====================================================
// MX CLUB MEMORY//4
// ESP32 + ST7789 240x320 + HW-171 PCF8574 + buzzer
//
// PCF8574 address confirmed by your scanner: 0x27
//
// PCF button map:
// P0 = START
// P1 = STOP
// P2 = UP
// P3 = LEFT
// P4 = DOWN
// P5 = RIGHT
//
// Buttons connect: PCF pin -> button -> GND
// =====================================================

// ---------------- TFT ----------------
#define TFT_CS    5
#define TFT_DC    27
#define TFT_RST   33
#define TFT_MOSI  23
#define TFT_SCLK  18

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// ---------------- PCF8574 / I2C ----------------
#define PCF_ADDR 0x27
#define I2C_SDA  21
#define I2C_SCL  22

#define BTN_START 0
#define BTN_STOP  1
#define BTN_UP    2
#define BTN_LEFT  3
#define BTN_DOWN  4
#define BTN_RIGHT 5

// ---------------- ACTIVE BUZZER ----------------
// SIG / I-O -> ESP32 GPIO15
#define BUZZER_PIN 15

// ---------------- GAME ----------------
#define MAX_SEQUENCE 64
uint8_t sequenceData[MAX_SEQUENCE];

int sequenceLength = 0;
int roundNumber = 0;
uint16_t score = 0;
uint16_t highScore = 0;

Preferences prefs;

// =====================================================
// COLORS
// =====================================================

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

// =====================================================
// DIFFICULTY
// =====================================================

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
  if (value < minimumSpeed) value = minimumSpeed;
  return value;
}

// =====================================================
// PCF8574
// =====================================================

bool checkPCF() {
  Wire.beginTransmission(PCF_ADDR);
  return Wire.endTransmission() == 0;
}

void initPCF() {
  Wire.beginTransmission(PCF_ADDR);

  // PCF8574 pins act as inputs when written HIGH.
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
  if (!buttonPressed(pin)) return false;

  delay(20);

  if (!buttonPressed(pin)) return false;

  waitButtonRelease(pin);
  return true;
}

// =====================================================
// BUZZER
// This is for a 3-pin ACTIVE buzzer module.
// =====================================================

void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
}

void chirp(int ms) {
  buzzerOn();
  delay(ms);
  buzzerOff();
}

void soundStart() {
  chirp(55);
  delay(35);
  chirp(85);
}

void soundSelect() {
  chirp(35);
}

void soundCorrect() {
  chirp(40);
  delay(25);
  chirp(55);
  delay(25);
  chirp(80);
}

void soundWrong() {
  chirp(330);
}

// =====================================================
// TEXT HELPERS
// =====================================================

void centerText(const String &text, int y, int size, uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.setTextWrap(false);

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, y);
  tft.print(text);
}

// =====================================================
// TRANSPARENT MX CLUB LOGO
// The outer white image background is removed in mx_logo_120.h.
// =====================================================

void drawMXLogo(int x0, int y0) {
  const int maskStride = (MX_LOGO_W + 7) / 8;

  for (int y = 0; y < MX_LOGO_H; y++) {
    for (int x = 0; x < MX_LOGO_W; x++) {
      int maskIndex = y * maskStride + (x >> 3);
      uint8_t maskByte = pgm_read_byte(&mxLogo120Mask[maskIndex]);

      if (maskByte & (0x80 >> (x & 7))) {
        uint16_t c = pgm_read_word(&mxLogo120[y * MX_LOGO_W + x]);
        tft.drawPixel(x0 + x, y0 + y, c);
      }
    }
  }
}

// =====================================================
// BASE UI
// =====================================================

void drawBackground() {
  tft.fillScreen(BG);
  tft.fillRect(0, 0, 240, 3, CYAN);
  tft.fillRect(0, 317, 240, 3, PINK);
}

// =====================================================
// BOOT / BRANDING
// =====================================================

void bootScreen() {
  drawBackground();

  drawMXLogo(60, 24);

  centerText("MX CLUB GAMES", 160, 2, ST77XX_WHITE);
  centerText("MECHATRONIX", 190, 1, CYAN);

  tft.drawRoundRect(35, 230, 170, 14, 5, PURPLE);

  for (int w = 0; w <= 162; w += 6) {
    tft.fillRoundRect(39, 234, w, 6, 3, PURPLE);
    delay(10);
  }

  centerText("MEMORY SYSTEM READY", 267, 1, GREEN);
  delay(650);
}

// =====================================================
// TITLE SCREEN
// Drawn ONCE. No repeated full-screen blinking.
// =====================================================

void titleScreen() {
  waitAllReleased();

  drawBackground();
  drawMXLogo(60, 12);

  centerText("MX CLUB GAMES", 142, 2, ST77XX_WHITE);
  centerText("MEMORY//4", 174, 3, CYAN);
  centerText("WATCH  REMEMBER  REPEAT", 215, 1, MUTED);

  tft.fillRoundRect(32, 246, 176, 46, 10, PANEL);
  tft.drawRoundRect(32, 246, 176, 46, 10, GREEN);
  centerText("PRESS START", 261, 2, GREEN);

  while (true) {
    if (confirmedPress(BTN_START)) {
      soundStart();
      return;
    }
    delay(5);
  }
}

// =====================================================
// DIFFICULTY MENU
// Only the changed option boxes are redrawn.
// =====================================================

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

void drawDifficultyOption(int index, bool selected) {
  int y = 94 + index * 44;
  uint16_t c = diffColorFor(index);

  if (selected) {
    tft.fillRoundRect(36, y, 168, 34, 8, c);
    tft.setTextColor(BG);
  } else {
    tft.fillRoundRect(36, y, 168, 34, 8, PANEL);
    tft.drawRoundRect(36, y, 168, 34, 8, c);
    tft.setTextColor(c);
  }

  tft.setTextSize(2);

  String text = diffNameFor(index);
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(text, 0, y, &x1, &y1, &tw, &th);
  tft.setCursor((240 - tw) / 2, y + 9);
  tft.print(text);
}

void drawDifficultyMenu() {
  drawBackground();

  centerText("SELECT MODE", 28, 3, ST77XX_WHITE);
  centerText("UP / DOWN", 66, 1, MUTED);

  for (int i = 0; i < 4; i++) {
    drawDifficultyOption(i, i == (int)difficulty);
  }

  centerText("START = PLAY   STOP = BACK", 286, 1, ST77XX_WHITE);
}

bool chooseDifficulty() {
  waitAllReleased();
  drawDifficultyMenu();

  while (true) {
    if (confirmedPress(BTN_UP)) {
      int old = (int)difficulty;
      int next = old - 1;
      if (next < 0) next = 3;

      difficulty = (Difficulty)next;

      drawDifficultyOption(old, false);
      drawDifficultyOption(next, true);
      soundSelect();
    }

    if (confirmedPress(BTN_DOWN)) {
      int old = (int)difficulty;
      int next = old + 1;
      if (next > 3) next = 0;

      difficulty = (Difficulty)next;

      drawDifficultyOption(old, false);
      drawDifficultyOption(next, true);
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

// =====================================================
// GAME UI
// IMPORTANT: the whole screen is drawn only once.
// During gameplay we update only the header/status/pad that changed.
// =====================================================

void drawHeaderFrame() {
  tft.fillRoundRect(8, 9, 224, 42, 8, PANEL);
  tft.drawRoundRect(8, 9, 224, 42, 8, PURPLE);
}

void updateHeader() {
  // Clear only the INSIDE of the header, not the whole screen.
  tft.fillRect(15, 17, 210, 26, PANEL);

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

  tft.setTextColor(difficultyColor());
  tft.setCursor(208, 27);
  tft.print(difficultyShort());
}

void drawStatus(const String &text, uint16_t color) {
  // Only this small strip changes.
  tft.fillRoundRect(20, 57, 200, 23, 6, PANEL2);
  tft.drawRoundRect(20, 57, 200, 23, 6, color);

  tft.setTextSize(1);
  tft.setTextColor(color);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, 65);
  tft.print(text);
}

void directionGeometry(int direction, int &x, int &y, int &w, int &h) {
  w = 66;
  h = 52;

  switch (direction) {
    case 0: // UP
      x = 87;
      y = 86;
      break;

    case 1: // LEFT
      x = 18;
      y = 146;
      break;

    case 2: // DOWN
      x = 87;
      y = 206;
      break;

    default: // RIGHT
      x = 156;
      y = 146;
      break;
  }
}

uint16_t directionColor(int direction) {
  switch (direction) {
    case 0: return CYAN;
    case 1: return PINK;
    case 2: return GREEN;
    default: return YELLOW;
  }
}

char directionSymbol(int direction) {
  switch (direction) {
    case 0: return '^';
    case 1: return '<';
    case 2: return 'v';
    default: return '>';
  }
}

const char* directionLabel(int direction) {
  switch (direction) {
    case 0: return "UP";
    case 1: return "LEFT";
    case 2: return "DOWN";
    default: return "RIGHT";
  }
}

void drawDirection(int direction, bool active) {
  int x, y, w, h;
  directionGeometry(direction, x, y, w, h);

  uint16_t c = directionColor(direction);

  // This redraws ONLY ONE pad.
  if (active) {
    tft.fillRoundRect(x, y, w, h, 9, c);
    tft.drawRoundRect(x, y, w, h, 9, ST77XX_WHITE);
    tft.setTextColor(BG);
  } else {
    tft.fillRoundRect(x, y, w, h, 9, PANEL);
    tft.drawRoundRect(x, y, w, h, 9, c);
    tft.setTextColor(c);
  }

  tft.setTextSize(3);
  tft.setCursor(x + 24, y + 6);
  tft.print(directionSymbol(direction));

  tft.setTextSize(1);
  String label = directionLabel(direction);

  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor(x + (w - tw) / 2, y + 38);
  tft.print(label);
}

void drawControlBar() {
  tft.fillRoundRect(11, 270, 103, 35, 7, PANEL);
  tft.drawRoundRect(11, 270, 103, 35, 7, GREEN);

  tft.fillRoundRect(126, 270, 103, 35, 7, PANEL);
  tft.drawRoundRect(126, 270, 103, 35, 7, PINK);

  tft.setTextSize(1);

  tft.setTextColor(GREEN);
  tft.setCursor(31, 284);
  tft.print("START  P0");

  tft.setTextColor(PINK);
  tft.setCursor(149, 284);
  tft.print("STOP  P1");
}

void drawGameLayout() {
  drawBackground();

  drawHeaderFrame();
  updateHeader();

  drawStatus("GET READY", CYAN);

  for (int i = 0; i < 4; i++) {
    drawDirection(i, false);
  }

  drawControlBar();
}

// =====================================================
// STOP-AWARE DELAY
// =====================================================

bool delayWithStop(int durationMs) {
  unsigned long started = millis();

  while ((millis() - started) < (unsigned long)durationMs) {
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

// =====================================================
// SEQUENCE FLASH
// NO full-screen redraw, only the active pad changes.
// =====================================================

bool flashDirection(int direction, int durationMs) {
  drawDirection(direction, true);
  buzzerOn();

  if (delayWithStop(durationMs)) {
    buzzerOff();
    drawDirection(direction, false);
    return false;
  }

  buzzerOff();
  drawDirection(direction, false);

  if (delayWithStop(85)) {
    return false;
  }

  return true;
}

bool showSequence() {
  updateHeader();

  drawStatus("ROUND " + String(roundNumber) + "  WATCH", PINK);

  if (delayWithStop(420)) {
    return false;
  }

  int speedMs = sequenceSpeed();

  for (int i = 0; i < sequenceLength; i++) {
    if (!flashDirection(sequenceData[i], speedMs)) {
      return false;
    }
  }

  drawStatus("YOUR TURN", GREEN);
  return true;
}

// =====================================================
// PLAYER INPUT
// Returns:
// 0 UP, 1 LEFT, 2 DOWN, 3 RIGHT
// -1 STOP
// =====================================================

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

// Returns:
//  1 = correct
//  0 = wrong
// -1 = stop

int playerTurn() {
  for (int i = 0; i < sequenceLength; i++) {
    int pressed = waitDirection();

    if (pressed == -1) {
      return -1;
    }

    // Feedback affects only the pressed pad.
    drawDirection(pressed, true);
    buzzerOn();
    delay(70);
    buzzerOff();
    drawDirection(pressed, false);

    if (pressed != sequenceData[i]) {
      return 0;
    }
  }

  return 1;
}

// =====================================================
// GAME FEEDBACK
// Only the status bar changes, so there is no screen flash.
// =====================================================

void successFeedback(uint16_t gained) {
  drawStatus("CLEAR!  +" + String(gained), GREEN);
  soundCorrect();
  delay(420);
}

void wrongFeedback() {
  drawStatus("WRONG!", REDCOLOR);
  soundWrong();
  delay(500);
}

void stoppedFeedback() {
  drawStatus("STOPPED - MENU", PINK);
  delay(450);
}

// =====================================================
// GAME DATA
// =====================================================

void resetGame() {
  sequenceLength = 0;
  roundNumber = 0;
  score = 0;

  memset(sequenceData, 0, sizeof(sequenceData));
}

void addStep() {
  if (sequenceLength >= MAX_SEQUENCE) return;

  sequenceData[sequenceLength] = random(0, 4);
  sequenceLength++;
}

void saveHighScoreIfNeeded() {
  if (score > highScore) {
    highScore = score;
    prefs.putUShort("highscore", highScore);
  }
}

// =====================================================
// GAME OVER
// Returns true = replay same difficulty
// false = back to title
// =====================================================

bool gameOverScreen() {
  bool newRecord = score > highScore;
  saveHighScoreIfNeeded();

  drawBackground();

  centerText("MX CLUB GAMES", 26, 2, MUTED);
  centerText("GAME OVER", 68, 4, PINK);

  tft.fillRoundRect(29, 126, 182, 98, 12, PANEL);
  tft.drawRoundRect(29, 126, 182, 98, 12, PURPLE);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

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
    centerText("NEW CLUB RECORD!", 238, 1, YELLOW);
  } else {
    centerText(difficultyName(), 238, 1, difficultyColor());
  }

  tft.fillRoundRect(18, 267, 98, 35, 7, PANEL);
  tft.drawRoundRect(18, 267, 98, 35, 7, GREEN);

  tft.fillRoundRect(124, 267, 98, 35, 7, PANEL);
  tft.drawRoundRect(124, 267, 98, 35, 7, PINK);

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

// =====================================================
// WIN SCREEN
// =====================================================

bool winScreen() {
  saveHighScoreIfNeeded();

  drawBackground();

  centerText("MX CLUB GAMES", 35, 2, MUTED);
  centerText("MEMORY", 82, 4, YELLOW);
  centerText("MASTER!", 127, 4, GREEN);

  centerText("64 MOVES CLEARED", 194, 2, ST77XX_WHITE);
  centerText("START = AGAIN  STOP = MENU", 266, 1, MUTED);

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

// =====================================================
// MAIN GAME
// =====================================================

void playGame() {
  while (true) {
    resetGame();

    // Draw the whole game screen ONCE.
    drawGameLayout();

    bool replay = false;

    while (true) {
      roundNumber++;
      addStep();

      if (!showSequence()) {
        stoppedFeedback();
        return;
      }

      int result = playerTurn();

      if (result == -1) {
        stoppedFeedback();
        return;
      }

      if (result == 0) {
        wrongFeedback();
        replay = gameOverScreen();
        break;
      }

      uint16_t gained =
        roundNumber *
        10 *
        scoreMultiplier();

      score += gained;
      updateHeader();
      successFeedback(gained);

      if (sequenceLength >= MAX_SEQUENCE) {
        replay = winScreen();
        break;
      }

      drawStatus("NEXT ROUND", CYAN);

      if (delayWithStop(350)) {
        stoppedFeedback();
        return;
      }
    }

    if (!replay) {
      return;
    }
  }
}

// =====================================================
// PCF ERROR SCREEN
// =====================================================

void pcfErrorScreen() {
  drawBackground();

  centerText("PCF8574 ERROR", 95, 3, ST77XX_RED);
  centerText("0x27 NOT FOUND", 145, 2, ST77XX_WHITE);
  centerText("SDA=21  SCL=22", 184, 1, YELLOW);
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOff();

  // I2C / PCF8574
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);

  // TFT
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(240, 320);

  // Your previous landscape screen needed rotation 3 instead of 1,
  // so portrait is set to 2.
  // If portrait is upside down on your exact mounting, use 0 instead.
  tft.setRotation(2);

  // Colors
  BG       = tft.color565(5,   8,  18);
  PANEL    = tft.color565(15, 21,  39);
  PANEL2   = tft.color565(10, 15,  30);
  CYAN     = tft.color565(0, 220, 255);
  PINK     = tft.color565(255, 45, 145);
  GREEN    = tft.color565(30, 255, 145);
  YELLOW   = tft.color565(255, 220, 40);
  PURPLE   = tft.color565(112, 44, 190);
  REDCOLOR = tft.color565(230, 35, 55);
  MUTED    = tft.color565(145, 155, 180);

  // PCF check
  if (!checkPCF()) {
    Serial.println("ERROR: PCF8574 0x27 NOT FOUND");
    pcfErrorScreen();

    while (true) {
      delay(1000);
    }
  }

  Serial.println("PCF8574 FOUND AT 0x27");
  initPCF();

  // Saved high score
  prefs.begin("mxmemory4", false);
  highScore = prefs.getUShort("highscore", 0);

  randomSeed(esp_random());

  bootScreen();
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  titleScreen();

  if (chooseDifficulty()) {
    playGame();
  }
}
