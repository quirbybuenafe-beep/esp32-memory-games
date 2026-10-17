#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_MISO -1

#define I2C_SDA 21
#define I2C_SCL 22
#define BUZZER_PIN 15

#define PCF_START 0
#define PCF_STOP  1
#define PCF_UP    2
#define PCF_LEFT  3
#define PCF_DOWN  4
#define PCF_RIGHT 5

#define BTN_START 0x01
#define BTN_STOP  0x02
#define BTN_UP    0x04
#define BTN_LEFT  0x08
#define BTN_DOWN  0x10
#define BTN_RIGHT 0x20

#define SCREEN_W 240
#define SCREEN_H 320
#define REGION_W 230
#define REGION_H 100

#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_BG      0x000C
#define C_GRID    0x0843
#define C_PANEL   0x10A4
#define C_PANEL2  0x18E6
#define C_DARK    0x0842
#define C_CYAN    0x07FF
#define C_BLUE    0x041F
#define C_PINK    0xF81F
#define C_PURPLE  0xB19C
#define C_GREEN   0x07E0
#define C_RED     0xF800
#define C_YELLOW  0xFFE0
#define C_ORANGE  0xFD20
#define C_GREY    0x8410
#define C_DIM     0x4208
#define C_HPBG    0x2945

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 region(REGION_W, REGION_H);
Preferences prefs;

const uint16_t directionColor[4] = {
  C_CYAN,
  C_PINK,
  C_YELLOW,
  C_GREEN
};

const uint8_t directionHeldMask[4] = {
  BTN_UP,
  BTN_LEFT,
  BTN_DOWN,
  BTN_RIGHT
};

const char* directionName[4] = {
  "UP",
  "LEFT",
  "DOWN",
  "RIGHT"
};

enum GameState {
  STATE_TITLE,
  STATE_DIFFICULTY,
  STATE_WATCH,
  STATE_PLAY,
  STATE_LEVEL_CLEAR,
  STATE_ENEMY_ATTACK,
  STATE_GAME_OVER,
  STATE_WIN
};

GameState gameState = STATE_TITLE;

unsigned long stateStart = 0;

bool screenDirty = true;
bool battleHeaderDirty = false;
bool enemyDirty = false;
bool statusDirty = false;
bool dpadDirty = false;
bool indicatorDirty = false;
bool menuAnimDirty = false;

uint8_t difficultyDirtyMask = 0;

int difficulty = 1;

const char* difficultyName[4] = {
  "EASY",
  "NORMAL",
  "HARD",
  "INSANE"
};

const char* difficultyDescription[4] = {
  "RELAXED",
  "BALANCED",
  "FAST",
  "BRUTAL"
};

int levelNumber = 1;
const int FINAL_LEVEL = 12;

int bestLevel = 1;
int hearts = 3;
int combo = 0;
int bestCombo = 0;
int misses = 0;
int wrongThisRound = 0;

int enemyType = 0;
int enemyHP = 1;
int enemyMaxHP = 1;

unsigned long enemyHitUntil = 0;
unsigned long lastIdleEnemyFrame = 0;
unsigned long lastFastEnemyFrame = 0;

#define MAX_SEQUENCE 24

uint8_t sequenceDirection[MAX_SEQUENCE];
uint8_t sequenceGapType[MAX_SEQUENCE];
uint32_t sequenceTime[MAX_SEQUENCE];

int sequenceCount = 0;
int sequenceIndex = 0;

uint16_t beatLength = 700;

unsigned long playerTurnStarted = 0;

uint8_t pcfAddress = 0x27;
bool pcfFound = false;

const uint8_t keyMask[6] = {
  BTN_START,
  BTN_STOP,
  BTN_UP,
  BTN_LEFT,
  BTN_DOWN,
  BTN_RIGHT
};

const uint8_t pcfPinForKey[6] = {
  PCF_START,
  PCF_STOP,
  PCF_UP,
  PCF_LEFT,
  PCF_DOWN,
  PCF_RIGHT
};

enum KeyCode {
  KEY_NONE = 0,
  KEY_START,
  KEY_STOP,
  KEY_UP,
  KEY_LEFT,
  KEY_DOWN,
  KEY_RIGHT
};

const uint8_t keyCodeForIndex[6] = {
  KEY_START,
  KEY_STOP,
  KEY_UP,
  KEY_LEFT,
  KEY_DOWN,
  KEY_RIGHT
};

bool rawPressedState[6] = {
  false, false, false,
  false, false, false
};

bool buttonStablePressed[6] = {
  false, false, false,
  false, false, false
};

unsigned long rawChangedAt[6] = {
  0, 0, 0, 0, 0, 0
};

uint8_t buttonsHeld = 0;

unsigned long lastButtonSample = 0;

const unsigned long BUTTON_SAMPLE_MS = 1;
const unsigned long PRESS_DEBOUNCE_MS = 3;
const unsigned long RELEASE_DEBOUNCE_MS = 10;

uint8_t lastRawPCF = 0xFF;

bool lastPCFReadGood = false;

unsigned long lastPCFOkAt = 0;
unsigned long lastPCFRetryAt = 0;

bool lastPCFUiOnline = false;

#define KEY_QUEUE_SIZE 24

uint8_t keyQueue[KEY_QUEUE_SIZE];

uint8_t keyQueueHead = 0;
uint8_t keyQueueTail = 0;

char lastInputText[18] = "NONE";

unsigned long lastInputUntil = 0;

bool inputWasVisible = false;

unsigned long padFlashUntil[4] = {
  0, 0, 0, 0
};

uint8_t lastPadVisualMask = 0;

char feedbackText[24] = "";

uint16_t feedbackColor = C_WHITE;

unsigned long feedbackUntil = 0;

bool feedbackWasVisible = false;

bool attackActive = false;

unsigned long attackStarted = 0;

int attackDirection = 0;

bool attackPerfect = false;

unsigned long lastMenuFrame = 0;

uint8_t menuEnemyType = 0;

unsigned long lastAttackFrame = 0;

struct AudioStep {
  uint16_t frequency;
  uint16_t duration;
  uint16_t gap;
};

enum MusicMode {
  MUSIC_SILENT,
  MUSIC_MENU,
  MUSIC_BATTLE,
  MUSIC_BOSS
};

MusicMode musicMode = MUSIC_SILENT;

int musicStep = 0;

unsigned long musicNextAt = 0;
unsigned long bossMusicStartedAt = 0;

int lastBossMusicLayer = -1;

const AudioStep* activeSfx = NULL;

int activeSfxLength = 0;
int activeSfxIndex = 0;

unsigned long sfxNextAt = 0;

const AudioStep menuMusic[] = {
  {294, 100, 80},
  {349, 100, 80},
  {440, 125, 90},
  {349, 95, 75},

  {262, 100, 80},
  {330, 100, 80},
  {392, 125, 90},
  {330, 95, 120}
};

const int MENU_MUSIC_COUNT =
  sizeof(menuMusic) /
  sizeof(menuMusic[0]);

const AudioStep battleMusic[] = {
  {147, 115, 35},
  {220, 95, 30},
  {294, 160, 45},
  {349, 120, 35},

  {147, 100, 30},
  {262, 115, 35},
  {330, 150, 45},
  {294, 190, 65},

  {131, 115, 35},
  {196, 95, 30},
  {262, 160, 45},
  {330, 120, 35},

  {131, 100, 30},
  {220, 115, 35},
  {294, 150, 45},
  {262, 190, 65},

  {117, 115, 35},
  {175, 95, 30},
  {233, 150, 40},
  {294, 120, 35},

  {349, 145, 40},
  {392, 120, 35},
  {349, 120, 35},
  {294, 200, 70},

  {110, 115, 35},
  {165, 95, 30},
  {220, 145, 40},
  {277, 120, 35},

  {330, 140, 40},
  {294, 120, 35},
  {220, 120, 35},
  {147, 230, 90}
};

const int BATTLE_MUSIC_COUNT =
  sizeof(battleMusic) /
  sizeof(battleMusic[0]);

const AudioStep bossMusic1[] = {
  {147, 125, 45},
  {220, 105, 35},
  {294, 150, 40},
  {349, 110, 35},

  {131, 120, 40},
  {196, 100, 35},
  {277, 155, 40},
  {330, 180, 60},

  {117, 120, 40},
  {175, 100, 35},
  {233, 145, 40},
  {349, 180, 70}
};

const int BOSS1_COUNT =
  sizeof(bossMusic1) /
  sizeof(bossMusic1[0]);

const AudioStep bossMusic2[] = {
  {147, 75, 18},
  {220, 70, 16},
  {294, 95, 18},
  {349, 80, 16},

  {175, 70, 16},
  {262, 75, 16},
  {349, 95, 18},
  {440, 105, 24},

  {147, 70, 16},
  {233, 70, 16},
  {330, 90, 16},
  {466, 110, 28}
};

const int BOSS2_COUNT =
  sizeof(bossMusic2) /
  sizeof(bossMusic2[0]);

const AudioStep bossMusic3[] = {
  {147, 48, 8},
  {294, 62, 8},
  {349, 48, 7},
  {440, 62, 8},

  {523, 48, 7},
  {440, 48, 7},
  {349, 62, 8},
  {277, 48, 7},

  {147, 48, 8},
  {330, 62, 8},
  {415, 48, 7},
  {554, 62, 8},

  {622, 48, 7},
  {554, 48, 7},
  {415, 62, 8},
  {294, 85, 20}
};

const int BOSS3_COUNT =
  sizeof(bossMusic3) /
  sizeof(bossMusic3[0]);

const AudioStep sfxMenuMove[] = {
  {660, 28, 0}
};

const AudioStep sfxMenuConfirm[] = {
  {660, 35, 8},
  {880, 55, 0}
};

const AudioStep sfxWatchUp[] = {
  {587, 75, 0}
};

const AudioStep sfxWatchLeft[] = {
  {349, 75, 0}
};

const AudioStep sfxWatchDown[] = {
  {440, 75, 0}
};

const AudioStep sfxWatchRight[] = {
  {698, 75, 0}
};

const AudioStep sfxAttackUp[] = {
  {220, 18, 3},
  {587, 26, 3},
  {740, 34, 3},
  {988, 78, 0}
};

const AudioStep sfxAttackLeft[] = {
  {110, 22, 3},
  {147, 38, 3},
  {220, 46, 3},
  {294, 82, 0}
};

const AudioStep sfxAttackDown[] = {
  {247, 18, 3},
  {523, 28, 3},
  {392, 40, 3},
  {196, 90, 0}
};

const AudioStep sfxAttackRight[] = {
  {247, 16, 3},
  {660, 22, 3},
  {988, 28, 3},
  {1319, 72, 0}
};

const AudioStep sfxWrong[] = {
  {180, 70, 8},
  {145, 80, 0}
};

const AudioStep sfxEnemyAttack[] = {
  {392, 35, 5},
  {294, 40, 5},
  {220, 50, 5},
  {147, 100, 0}
};

const AudioStep sfxClear[] = {
  {523, 55, 8},
  {659, 55, 8},
  {784, 70, 8},
  {1047, 130, 0}
};

const AudioStep sfxFail[] = {
  {330, 80, 8},
  {247, 90, 8},
  {180, 150, 0}
};

const AudioStep sfxWin[] = {
  {523, 50, 6},
  {659, 50, 6},
  {784, 50, 6},
  {1047, 65, 6},
  {1319, 75, 6},
  {1568, 160, 0}
};

void centerTextOn(
  Adafruit_GFX &g,
  const char* text,
  int width,
  int y,
  uint16_t color,
  uint8_t size
) {
  int16_t x1;
  int16_t y1;

  uint16_t w;
  uint16_t h;

  g.setTextSize(size);

  g.getTextBounds(
    (char*)text,
    0,
    y,
    &x1,
    &y1,
    &w,
    &h
  );

  g.setTextColor(color);

  g.setCursor(
    (width - w) / 2,
    y
  );

  g.print(text);
}

void panel3DOn(
  Adafruit_GFX &g,
  int x,
  int y,
  int w,
  int h,
  uint16_t fill,
  uint16_t edge
) {
  g.fillRoundRect(
    x + 4,
    y + 5,
    w,
    h,
    6,
    C_BLACK
  );

  g.fillRoundRect(
    x,
    y,
    w,
    h,
    6,
    fill
  );

  g.drawRoundRect(
    x,
    y,
    w,
    h,
    6,
    edge
  );

  g.drawFastHLine(
    x + 8,
    y + 2,
    w - 16,
    C_WHITE
  );

  g.drawFastHLine(
    x + 8,
    y + h - 3,
    w - 16,
    C_BLACK
  );
}

void pxOn(
  Adafruit_GFX &g,
  int x,
  int y,
  int size,
  uint16_t color
) {
  g.fillRect(
    x,
    y,
    size,
    size,
    color
  );
}

void drawHeartOn(
  Adafruit_GFX &g,
  int x,
  int y,
  uint16_t color
) {
  g.fillRect(x + 3, y, 3, 3, color);
  g.fillRect(x + 9, y, 3, 3, color);

  g.fillRect(x, y + 3, 15, 3, color);
  g.fillRect(x, y + 6, 15, 3, color);

  g.fillRect(x + 3, y + 9, 9, 3, color);

  g.fillRect(x + 6, y + 12, 3, 3, color);

  g.drawPixel(
    x + 3,
    y + 3,
    C_WHITE
  );
}

bool addressExists(
  uint8_t address
) {
  Wire.beginTransmission(
    address
  );

  return
    Wire.endTransmission() ==
    0;
}

void detectPCF() {
  pcfFound =
    false;

  if (
    addressExists(
      0x27
    )
  ) {
    pcfAddress =
      0x27;

    pcfFound =
      true;

    return;
  }

  for (
    uint8_t address = 0x20;
    address <= 0x27;
    address++
  ) {
    if (
      addressExists(
        address
      )
    ) {
      pcfAddress =
        address;

      pcfFound =
        true;

      return;
    }
  }
}

void pcfWrite(
  uint8_t value
) {
  if (
    !pcfFound
  ) {
    return;
  }

  Wire.beginTransmission(
    pcfAddress
  );

  Wire.write(
    value
  );

  Wire.endTransmission();
}

uint8_t readPCFRaw() {
  if (
    !pcfFound
  ) {
    lastPCFReadGood =
      false;

    return
      0xFF;
  }

  uint8_t got =
    Wire.requestFrom(
      (uint8_t)pcfAddress,
      (uint8_t)1
    );

  if (
    got ==
      1 &&
    Wire.available()
  ) {
    uint8_t raw =
      Wire.read();

    raw |=
      0xC0;

    lastRawPCF =
      raw;

    lastPCFReadGood =
      true;

    lastPCFOkAt =
      millis();

    return
      raw;
  }

  lastPCFReadGood =
    false;

  return
    lastRawPCF;
}

bool pcfIsOnline() {
  return
    pcfFound &&
    (
      millis() -
      lastPCFOkAt <
      500
    );
}

bool keyQueueEmpty() {
  return
    keyQueueHead ==
    keyQueueTail;
}

void clearKeyQueue() {
  keyQueueHead =
    0;

  keyQueueTail =
    0;
}

void pushKey(
  uint8_t key
) {
  uint8_t next =
    (
      keyQueueHead +
      1
    ) %
    KEY_QUEUE_SIZE;

  if (
    next ==
    keyQueueTail
  ) {
    keyQueueTail =
      (
        keyQueueTail +
        1
      ) %
      KEY_QUEUE_SIZE;
  }

  keyQueue[
    keyQueueHead
  ] =
    key;

  keyQueueHead =
    next;
}

uint8_t popKey() {
  if (
    keyQueueEmpty()
  ) {
    return
      KEY_NONE;
  }

  uint8_t key =
    keyQueue[
      keyQueueTail
    ];

  keyQueueTail =
    (
      keyQueueTail +
      1
    ) %
    KEY_QUEUE_SIZE;

  return
    key;
}

const char* keyName(
  uint8_t key
) {
  if (key == KEY_START) return "START";
  if (key == KEY_STOP)  return "STOP";
  if (key == KEY_UP)    return "UP";
  if (key == KEY_LEFT)  return "LEFT";
  if (key == KEY_DOWN)  return "DOWN";
  if (key == KEY_RIGHT) return "RIGHT";

  return
    "NONE";
}

void recordInput(
  uint8_t key
) {
  strncpy(
    lastInputText,
    keyName(key),
    sizeof(lastInputText) - 1
  );

  lastInputText[
    sizeof(lastInputText) - 1
  ] =
    0;

  lastInputUntil =
    millis() +
    500;

  indicatorDirty =
    true;

  statusDirty =
    true;

  Serial.print(
    "BUTTON="
  );

  Serial.print(
    keyName(key)
  );

  Serial.print(
    " RAW=0x"
  );

  if (
    lastRawPCF <
    16
  ) {
    Serial.print(
      '0'
    );
  }

  Serial.print(
    lastRawPCF,
    HEX
  );

  Serial.print(
    " STATE="
  );

  Serial.println(
    (int)gameState
  );
}

void initializeButtonFilter() {
  uint8_t raw =
    readPCFRaw();

  unsigned long now =
    millis();

  buttonsHeld =
    0;

  for (
    int i = 0;
    i < 6;
    i++
  ) {
    bool pressed =
      lastPCFReadGood &&
      (
        (
          raw &
          (
            1 <<
            pcfPinForKey[i]
          )
        ) ==
        0
      );

    rawPressedState[i] =
      pressed;

    buttonStablePressed[i] =
      pressed;

    rawChangedAt[i] =
      now;

    if (
      pressed
    ) {
      buttonsHeld |=
        keyMask[i];
    }
  }

  clearKeyQueue();
}

void servicePCFConnection() {
  unsigned long now =
    millis();

  if (
    pcfFound
  ) {
    return;
  }

  if (
    now -
    lastPCFRetryAt <
    750
  ) {
    return;
  }

  lastPCFRetryAt =
    now;

  detectPCF();

  if (
    pcfFound
  ) {
    pcfWrite(
      0xFF
    );

    delay(
      2
    );

    initializeButtonFilter();

    Serial.print(
      "PCF RECOVERED AT 0x"
    );

    Serial.println(
      pcfAddress,
      HEX
    );

    indicatorDirty =
      true;

    statusDirty =
      true;
  }
}

void updateButtons() {
  servicePCFConnection();

  if (
    !pcfFound
  ) {
    return;
  }

  unsigned long now =
    millis();

  if (
    now -
    lastButtonSample <
    BUTTON_SAMPLE_MS
  ) {
    return;
  }

  lastButtonSample =
    now;

  uint8_t raw =
    readPCFRaw();

  if (
    !lastPCFReadGood
  ) {
    if (
      now -
      lastPCFOkAt >
      400
    ) {
      pcfFound =
        false;

      buttonsHeld =
        0;

      dpadDirty =
        true;

      statusDirty =
        true;

      indicatorDirty =
        true;

      Serial.println(
        "PCF LOST - RETRYING"
      );
    }

    return;
  }

  uint8_t previousHeld =
    buttonsHeld;

  for (
    int i = 0;
    i < 6;
    i++
  ) {
    bool physicalPressed =
      (
        raw &
        (
          1 <<
          pcfPinForKey[i]
        )
      ) ==
      0;

    if (
      physicalPressed !=
      rawPressedState[i]
    ) {
      rawPressedState[i] =
        physicalPressed;

      rawChangedAt[i] =
        now;
    }

    if (
      physicalPressed !=
      buttonStablePressed[i]
    ) {
      unsigned long required =
        physicalPressed ?
        PRESS_DEBOUNCE_MS :
        RELEASE_DEBOUNCE_MS;

      if (
        now -
        rawChangedAt[i] >=
        required
      ) {
        buttonStablePressed[i] =
          physicalPressed;

        if (
          physicalPressed
        ) {
          buttonsHeld |=
            keyMask[i];

          uint8_t key =
            keyCodeForIndex[i];

          pushKey(
            key
          );

          recordInput(
            key
          );
        }

        else {
          buttonsHeld &=
            ~keyMask[i];
        }
      }
    }
  }

  if (
    previousHeld !=
    buttonsHeld
  ) {
    dpadDirty =
      true;
  }
}

void startSfx(
  const AudioStep* effect,
  int count
) {
  activeSfx =
    effect;

  activeSfxLength =
    count;

  activeSfxIndex =
    0;

  sfxNextAt =
    0;
}

void setMusicMode(
  MusicMode newMode,
  bool restartTrack = false
) {
  if (
    musicMode ==
      newMode &&
    !restartTrack
  ) {
    return;
  }

  musicMode =
    newMode;

  musicStep =
    0;

  musicNextAt =
    millis() +
    25;

  if (
    newMode ==
    MUSIC_BOSS
  ) {
    bossMusicStartedAt =
      millis();

    lastBossMusicLayer =
      -1;
  }
}

int getBossMusicLayer() {
  unsigned long age =
    millis() -
    bossMusicStartedAt;

  if (
    age <
    2200
  ) {
    return
      0;
  }

  if (
    age <
    5200
  ) {
    return
      1;
  }

  return
    2;
}

AudioStep getMusicStep() {
  AudioStep step = {
    0,
    20,
    40
  };

  if (
    musicMode ==
    MUSIC_MENU
  ) {
    step =
      menuMusic[
        musicStep %
        MENU_MUSIC_COUNT
      ];

    musicStep++;

    return
      step;
  }

  if (
    musicMode ==
    MUSIC_BATTLE
  ) {
    step =
      battleMusic[
        musicStep %
        BATTLE_MUSIC_COUNT
      ];

    musicStep++;

    return
      step;
  }

  if (
    musicMode ==
    MUSIC_BOSS
  ) {
    int layer =
      getBossMusicLayer();

    if (
      layer !=
      lastBossMusicLayer
    ) {
      lastBossMusicLayer =
        layer;

      musicStep =
        0;

      statusDirty =
        true;
    }

    if (
      layer ==
      0
    ) {
      step =
        bossMusic1[
          musicStep %
          BOSS1_COUNT
        ];
    }

    else if (
      layer ==
      1
    ) {
      step =
        bossMusic2[
          musicStep %
          BOSS2_COUNT
        ];
    }

    else {
      step =
        bossMusic3[
          musicStep %
          BOSS3_COUNT
        ];
    }

    musicStep++;

    return
      step;
  }

  return
    step;
}

void updateAudio() {
  unsigned long now =
    millis();

  if (
    activeSfx !=
    NULL
  ) {
    if (
      now <
      sfxNextAt
    ) {
      return;
    }

    if (
      activeSfxIndex >=
      activeSfxLength
    ) {
      activeSfx =
        NULL;

      activeSfxLength =
        0;

      activeSfxIndex =
        0;

      musicNextAt =
        now +
        18;

      return;
    }

    AudioStep step =
      activeSfx[
        activeSfxIndex
      ];

    if (
      step.frequency >
      0
    ) {
      tone(
        BUZZER_PIN,
        step.frequency,
        step.duration
      );
    }

    else {
      noTone(
        BUZZER_PIN
      );
    }

    sfxNextAt =
      now +
      step.duration +
      step.gap;

    activeSfxIndex++;

    return;
  }

  if (
    musicMode ==
    MUSIC_SILENT
  ) {
    return;
  }

  if (
    now <
    musicNextAt
  ) {
    return;
  }

  AudioStep step =
    getMusicStep();

  if (
    step.frequency >
    0
  ) {
    tone(
      BUZZER_PIN,
      step.frequency,
      step.duration
    );
  }

  else {
    noTone(
      BUZZER_PIN
    );
  }

  musicNextAt =
    now +
    step.duration +
    step.gap;
}

void playAttackSound(
  int direction
) {
  if (
    direction ==
    0
  ) {
    startSfx(
      sfxAttackUp,
      sizeof(sfxAttackUp) /
      sizeof(sfxAttackUp[0])
    );
  }

  else if (
    direction ==
    1
  ) {
    startSfx(
      sfxAttackLeft,
      sizeof(sfxAttackLeft) /
      sizeof(sfxAttackLeft[0])
    );
  }

  else if (
    direction ==
    2
  ) {
    startSfx(
      sfxAttackDown,
      sizeof(sfxAttackDown) /
      sizeof(sfxAttackDown[0])
    );
  }

  else {
    startSfx(
      sfxAttackRight,
      sizeof(sfxAttackRight) /
      sizeof(sfxAttackRight[0])
    );
  }
}

void playWatchSound(
  int direction
) {
  if (
    direction ==
    0
  ) {
    startSfx(
      sfxWatchUp,
      1
    );
  }

  else if (
    direction ==
    1
  ) {
    startSfx(
      sfxWatchLeft,
      1
    );
  }

  else if (
    direction ==
    2
  ) {
    startSfx(
      sfxWatchDown,
      1
    );
  }

  else {
    startSfx(
      sfxWatchRight,
      1
    );
  }
}

void showFeedback(
  const char* text,
  uint16_t color,
  unsigned long duration
) {
  strncpy(
    feedbackText,
    text,
    sizeof(feedbackText) - 1
  );

  feedbackText[
    sizeof(feedbackText) - 1
  ] =
    0;

  feedbackColor =
    color;

  feedbackUntil =
    millis() +
    duration;

  statusDirty =
    true;
}

const char* enemyName(
  int type
) {
  if (
    type ==
    0
  ) {
    return
      "BEAT SLIME";
  }

  if (
    type ==
    1
  ) {
    return
      "DRUM CRAB";
  }

  if (
    type ==
    2
  ) {
    return
      "NOTE GHOST";
  }

  if (
    type ==
    3
  ) {
    return
      "CORE BOT";
  }

  return
    "OMEGA DJ";
}

void drawSlime(
  Adafruit_GFX &g,
  int ox,
  int oy,
  int s,
  bool anim,
  bool hurt
) {
  uint16_t body =
    hurt ?
    C_WHITE :
    C_GREEN;

  for (
    int x = 2;
    x <= 5;
    x++
  ) {
    pxOn(
      g,
      ox + x * s,
      oy,
      s,
      body
    );
  }

  for (
    int x = 1;
    x <= 6;
    x++
  ) {
    pxOn(
      g,
      ox + x * s,
      oy + s,
      s,
      body
    );
  }

  for (
    int y = 2;
    y <= 5;
    y++
  ) {
    for (
      int x = 0;
      x <= 7;
      x++
    ) {
      pxOn(
        g,
        ox + x * s,
        oy + y * s,
        s,
        body
      );
    }
  }

  pxOn(
    g,
    ox + 2 * s,
    oy + 3 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 3 * s,
    s,
    C_BLACK
  );

  if (
    anim
  ) {
    pxOn(
      g,
      ox + 3 * s,
      oy + 5 * s,
      s,
      C_BLACK
    );

    pxOn(
      g,
      ox + 4 * s,
      oy + 5 * s,
      s,
      C_BLACK
    );
  }

  else {
    pxOn(
      g,
      ox + 3 * s,
      oy + 4 * s,
      s,
      C_BLACK
    );
  }
}

void drawCrab(
  Adafruit_GFX &g,
  int ox,
  int oy,
  int s,
  bool anim,
  bool hurt
) {
  uint16_t body =
    hurt ?
    C_WHITE :
    C_RED;

  for (
    int y = 2;
    y <= 5;
    y++
  ) {
    for (
      int x = 1;
      x <= 6;
      x++
    ) {
      pxOn(
        g,
        ox + x * s,
        oy + y * s,
        s,
        body
      );
    }
  }

  pxOn(
    g,
    ox + 2 * s,
    oy + 2 * s,
    s,
    C_WHITE
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 2 * s,
    s,
    C_WHITE
  );

  pxOn(
    g,
    ox + 2 * s,
    oy + 3 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 3 * s,
    s,
    C_BLACK
  );

  int movement =
    anim ?
    1 :
    0;

  pxOn(
    g,
    ox,
    oy +
    (
      2 +
      movement
    ) *
    s,
    s,
    body
  );

  pxOn(
    g,
    ox + 7 * s,
    oy +
    (
      2 -
      movement
    ) *
    s,
    s,
    body
  );

  pxOn(
    g,
    ox + s,
    oy + 6 * s,
    s,
    body
  );

  pxOn(
    g,
    ox + 6 * s,
    oy + 6 * s,
    s,
    body
  );
}

void drawGhost(
  Adafruit_GFX &g,
  int ox,
  int oy,
  int s,
  bool anim,
  bool hurt
) {
  uint16_t body =
    hurt ?
    C_WHITE :
    C_PURPLE;

  for (
    int y = 1;
    y <= 6;
    y++
  ) {
    int first =
      y < 3 ?
      2 :
      1;

    int last =
      y < 3 ?
      4 :
      5;

    for (
      int x = first;
      x <= last;
      x++
    ) {
      pxOn(
        g,
        ox + x * s,
        oy + y * s,
        s,
        body
      );
    }
  }

  pxOn(
    g,
    ox + s,
    oy + 7 * s,
    s,
    body
  );

  pxOn(
    g,
    ox + 3 * s,
    oy +
    (
      anim ?
      8 :
      7
    ) *
    s,
    s,
    body
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 7 * s,
    s,
    body
  );

  pxOn(
    g,
    ox + 2 * s,
    oy + 4 * s,
    s,
    C_WHITE
  );

  pxOn(
    g,
    ox + 4 * s,
    oy + 4 * s,
    s,
    C_WHITE
  );

  pxOn(
    g,
    ox + 2 * s,
    oy + 5 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 4 * s,
    oy + 5 * s,
    s,
    C_BLACK
  );
}

void drawRobot(
  Adafruit_GFX &g,
  int ox,
  int oy,
  int s,
  bool anim,
  bool hurt
) {
  uint16_t body =
    hurt ?
    C_WHITE :
    C_GREY;

  uint16_t light =
    anim ?
    C_CYAN :
    C_YELLOW;

  for (
    int y = 2;
    y <= 6;
    y++
  ) {
    for (
      int x = 1;
      x <= 6;
      x++
    ) {
      pxOn(
        g,
        ox + x * s,
        oy + y * s,
        s,
        body
      );
    }
  }

  pxOn(
    g,
    ox + 4 * s,
    oy,
    s,
    light
  );

  pxOn(
    g,
    ox + 4 * s,
    oy + s,
    s,
    light
  );

  pxOn(
    g,
    ox + 2 * s,
    oy + 4 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 4 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 3 * s,
    oy + 5 * s,
    s,
    C_CYAN
  );

  pxOn(
    g,
    ox + 4 * s,
    oy + 5 * s,
    s,
    C_CYAN
  );
}

void drawBoss(
  Adafruit_GFX &g,
  int ox,
  int oy,
  int s,
  bool anim,
  bool hurt
) {
  uint16_t body =
    hurt ?
    C_WHITE :
    C_ORANGE;

  for (
    int y = 2;
    y <= 7;
    y++
  ) {
    for (
      int x = 1;
      x <= 6;
      x++
    ) {
      pxOn(
        g,
        ox + x * s,
        oy + y * s,
        s,
        body
      );
    }
  }

  pxOn(
    g,
    ox,
    oy,
    s,
    C_YELLOW
  );

  pxOn(
    g,
    ox + s,
    oy + s,
    s,
    C_YELLOW
  );

  pxOn(
    g,
    ox + 7 * s,
    oy,
    s,
    C_YELLOW
  );

  pxOn(
    g,
    ox + 6 * s,
    oy + s,
    s,
    C_YELLOW
  );

  pxOn(
    g,
    ox + 2 * s,
    oy + 4 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 5 * s,
    oy + 4 * s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 3 * s,
    oy +
    (
      anim ?
      6 :
      5
    ) *
    s,
    s,
    C_BLACK
  );

  pxOn(
    g,
    ox + 4 * s,
    oy +
    (
      anim ?
      6 :
      5
    ) *
    s,
    s,
    C_BLACK
  );
}

void drawEnemyOn(
  Adafruit_GFX &g,
  int centerX,
  int centerY,
  int type
) {
  bool hurt =
    millis() <
    enemyHitUntil;

  bool anim =
    (
      (
        millis() /
        240
      ) %
      2
    ) !=
    0;

  int bob =
    anim ?
    2 :
    0;

  int shake =
    0;

  if (
    hurt
  ) {
    shake =
      (
        (
          millis() /
          25
        ) %
        2
      ) ?
      5 :
      -5;
  }

  int ox =
    centerX -
    20 +
    shake;

  int oy =
    centerY -
    18 +
    bob;

  g.fillRoundRect(
    centerX - 28,
    centerY + 29,
    56,
    7,
    3,
    C_BLACK
  );

  if (
    type ==
    0
  ) {
    drawSlime(
      g,
      ox,
      oy,
      5,
      anim,
      hurt
    );
  }

  else if (
    type ==
    1
  ) {
    drawCrab(
      g,
      ox,
      oy,
      5,
      anim,
      hurt
    );
  }

  else if (
    type ==
    2
  ) {
    drawGhost(
      g,
      ox,
      oy,
      5,
      anim,
      hurt
    );
  }

  else if (
    type ==
    3
  ) {
    drawRobot(
      g,
      ox,
      oy,
      5,
      anim,
      hurt
    );
  }

  else {
    drawBoss(
      g,
      ox,
      oy,
      5,
      anim,
      hurt
    );
  }
}

void drawHPBarOn(
  Adafruit_GFX &g
) {
  const int x = 34;
  const int y = 82;
  const int w = 162;
  const int h = 10;

  g.fillRoundRect(
    x,
    y,
    w,
    h,
    3,
    C_HPBG
  );

  g.drawRoundRect(
    x,
    y,
    w,
    h,
    3,
    C_WHITE
  );

  if (
    enemyMaxHP <=
    0
  ) {
    return;
  }

  int fillWidth =
    (
      enemyHP *
      (
        w -
        4
      )
    ) /
    enemyMaxHP;

  if (
    fillWidth <
    0
  ) {
    fillWidth =
      0;
  }

  uint16_t color =
    C_GREEN;

  if (
    levelNumber %
      5 ==
    0
  ) {
    color =
      C_ORANGE;
  }

  else if (
    enemyHP *
      2 <=
    enemyMaxHP
  ) {
    color =
      C_YELLOW;
  }

  if (
    levelNumber %
      5 !=
      0 &&
    enemyHP *
      3 <=
    enemyMaxHP
  ) {
    color =
      C_RED;
  }

  if (
    fillWidth >
    0
  ) {
    g.fillRoundRect(
      x + 2,
      y + 2,
      fillWidth,
      h - 4,
      2,
      color
    );
  }
}

void updateLevelTiming() {
  int bpm =
    68 +
    difficulty *
    10 +
    (
      levelNumber -
      1
    ) *
    4;

  if (
    bpm >
    160
  ) {
    bpm =
      160;
  }

  beatLength =
    60000 /
    bpm;
}

uint8_t randomGapType() {
  int r =
    random(
      100
    );

  if (
    difficulty ==
    0
  ) {
    return
      0;
  }

  if (
    difficulty ==
    1
  ) {
    return
      r <
        80 ?
      0 :
      1;
  }

  if (
    difficulty ==
    2
  ) {
    if (
      r <
      65
    ) {
      return
        0;
    }

    if (
      r <
      90
    ) {
      return
        2;
    }

    return
      1;
  }

  if (
    r <
    50
  ) {
    return
      0;
  }

  if (
    r <
    80
  ) {
    return
      2;
  }

  return
    3;
}

uint16_t gapTime(
  uint8_t type
) {
  if (
    type ==
    1
  ) {
    return
      beatLength +
      beatLength /
      2;
  }

  if (
    type ==
    2
  ) {
    return
      beatLength /
      2;
  }

  if (
    type ==
    3
  ) {
    return
      (
        beatLength *
        3
      ) /
      4;
  }

  return
    beatLength;
}

void rebuildSequenceTimes() {
  if (
    sequenceCount <=
    0
  ) {
    return;
  }

  sequenceTime[0] =
    1000;

  for (
    int i = 1;
    i < sequenceCount;
    i++
  ) {
    sequenceTime[i] =
      sequenceTime[
        i - 1
      ] +
      gapTime(
        sequenceGapType[i]
      );
  }
}

void generateRandomRoundSequence(
  bool forceFirstUp
) {
  uint8_t previousDirection[MAX_SEQUENCE];

  int previousCount =
    sequenceCount;

  if (
    previousCount >
    MAX_SEQUENCE
  ) {
    previousCount =
      MAX_SEQUENCE;
  }

  for (
    int i = 0;
    i < previousCount;
    i++
  ) {
    previousDirection[i] =
      sequenceDirection[i];
  }

  int targetCount =
    levelNumber;

  if (
    targetCount <
    1
  ) {
    targetCount =
      1;
  }

  if (
    targetCount >
    MAX_SEQUENCE
  ) {
    targetCount =
      MAX_SEQUENCE;
  }

  for (
    int attempt = 0;
    attempt < 10;
    attempt++
  ) {
    sequenceCount =
      targetCount;

    for (
      int i = 0;
      i < sequenceCount;
      i++
    ) {
      int direction;

      if (
        forceFirstUp &&
        i ==
          0
      ) {
        direction =
          0;
      }

      else {
        direction =
          random(
            4
          );
      }

      if (
        i >=
        2
      ) {
        int last =
          sequenceDirection[
            i - 1
          ];

        int before =
          sequenceDirection[
            i - 2
          ];

        if (
          last ==
            before &&
          direction ==
            last
        ) {
          direction =
            (
              direction +
              random(
                1,
                4
              )
            ) %
            4;
        }
      }

      sequenceDirection[i] =
        direction;

      sequenceGapType[i] =
        (
          i ==
          0
        ) ?
        0 :
        randomGapType();
    }

    bool identical =
      previousCount ==
      sequenceCount;

    if (
      identical
    ) {
      for (
        int i = 0;
        i < sequenceCount;
        i++
      ) {
        if (
          sequenceDirection[i] !=
          previousDirection[i]
        ) {
          identical =
            false;

          break;
        }
      }
    }

    if (
      !identical ||
      forceFirstUp ||
      previousCount ==
        0
    ) {
      break;
    }
  }

  rebuildSequenceTimes();

  Serial.print(
    "NEW RANDOM ROUND LV "
  );

  Serial.print(
    levelNumber
  );

  Serial.print(
    ": "
  );

  for (
    int i = 0;
    i < sequenceCount;
    i++
  ) {
    Serial.print(
      directionName[
        sequenceDirection[i]
      ]
    );

    if (
      i <
      sequenceCount -
        1
    ) {
      Serial.print(
        " -> "
      );
    }
  }

  Serial.println();
}

void configureEnemy() {
  if (
    levelNumber %
      5 ==
    0
  ) {
    enemyType =
      4;
  }

  else if (
    levelNumber <=
    2
  ) {
    enemyType =
      0;
  }

  else if (
    levelNumber <=
    4
  ) {
    enemyType =
      1;
  }

  else if (
    levelNumber <=
    7
  ) {
    enemyType =
      2;
  }

  else {
    enemyType =
      3;
  }

  enemyMaxHP =
    sequenceCount;

  enemyHP =
    enemyMaxHP;

  enemyDirty =
    true;

  battleHeaderDirty =
    true;
}

bool isBattleState(
  GameState state
) {
  return
    state ==
      STATE_WATCH ||
    state ==
      STATE_PLAY ||
    state ==
      STATE_LEVEL_CLEAR ||
    state ==
      STATE_ENEMY_ATTACK;
}

void syncMusicToGameState() {
  if (
    gameState ==
      STATE_TITLE ||
    gameState ==
      STATE_DIFFICULTY
  ) {
    setMusicMode(
      MUSIC_MENU
    );

    return;
  }

  if (
    isBattleState(
      gameState
    )
  ) {
    if (
      levelNumber %
        5 ==
      0
    ) {
      setMusicMode(
        MUSIC_BOSS
      );
    }

    else {
      setMusicMode(
        MUSIC_BATTLE
      );
    }

    return;
  }

  setMusicMode(
    MUSIC_SILENT
  );
}

void changeState(
  GameState newState
) {
  GameState oldState =
    gameState;

  gameState =
    newState;

  stateStart =
    millis();

  clearKeyQueue();

  if (
    isBattleState(
      oldState
    ) &&
    isBattleState(
      newState
    )
  ) {
    enemyDirty =
      true;

    statusDirty =
      true;

    dpadDirty =
      true;

    battleHeaderDirty =
      true;
  }

  else {
    screenDirty =
      true;
  }

  if (
    newState ==
    STATE_TITLE
  ) {
    menuAnimDirty =
      true;
  }

  if (
    newState ==
    STATE_DIFFICULTY
  ) {
    difficultyDirtyMask =
      0x0F;
  }

  syncMusicToGameState();
}

void startGame() {
  levelNumber =
    1;

  hearts =
    3;

  combo =
    0;

  bestCombo =
    0;

  misses =
    0;

  wrongThisRound =
    0;

  updateLevelTiming();

  generateRandomRoundSequence(
    true
  );

  configureEnemy();

  sequenceIndex =
    0;

  changeState(
    STATE_WATCH
  );
}

void startWatch() {
  updateLevelTiming();

  rebuildSequenceTimes();

  sequenceIndex =
    0;

  combo =
    0;

  misses =
    0;

  wrongThisRound =
    0;

  enemyHP =
    enemyMaxHP;

  changeState(
    STATE_WATCH
  );
}

void startPlayerTurn() {
  sequenceIndex =
    0;

  combo =
    0;

  misses =
    0;

  wrongThisRound =
    0;

  enemyHP =
    enemyMaxHP;

  playerTurnStarted =
    millis();

  clearKeyQueue();

  changeState(
    STATE_PLAY
  );
}

void updateWatch() {
  uint32_t elapsed =
    millis() -
    stateStart;

  if (
    sequenceIndex <
    sequenceCount
  ) {
    if (
      elapsed >=
      sequenceTime[
        sequenceIndex
      ]
    ) {
      int direction =
        sequenceDirection[
          sequenceIndex
        ];

      padFlashUntil[
        direction
      ] =
        millis() +
        220;

      dpadDirty =
        true;

      playWatchSound(
        direction
      );

      sequenceIndex++;

      statusDirty =
        true;
    }
  }

  else {
    uint32_t finalTime =
      sequenceTime[
        sequenceCount -
        1
      ];

    if (
      elapsed >
      finalTime +
      650
    ) {
      startPlayerTurn();
    }
  }
}

int keyToDirection(
  uint8_t key
) {
  if (
    key ==
    KEY_UP
  ) {
    return
      0;
  }

  if (
    key ==
    KEY_LEFT
  ) {
    return
      1;
  }

  if (
    key ==
    KEY_DOWN
  ) {
    return
      2;
  }

  if (
    key ==
    KEY_RIGHT
  ) {
    return
      3;
  }

  return
    -1;
}

void startPlayerAttack(
  int direction,
  bool perfect
) {
  attackActive =
    true;

  attackStarted =
    millis();

  attackDirection =
    direction;

  attackPerfect =
    perfect;

  enemyDirty =
    true;
}

void registerCorrectMove(
  int direction
) {
  unsigned long elapsed =
    millis() -
    playerTurnStarted;

  long timingError =
    (long)elapsed -
    (long)sequenceTime[
      sequenceIndex
    ];

  long absError =
    labs(
      timingError
    );

  bool perfect =
    absError <=
    120;

  combo++;

  if (
    combo >
    bestCombo
  ) {
    bestCombo =
      combo;
  }

  enemyHP--;

  if (
    enemyHP <
    0
  ) {
    enemyHP =
      0;
  }

  padFlashUntil[
    direction
  ] =
    millis() +
    180;

  enemyHitUntil =
    millis() +
    280;

  dpadDirty =
    true;

  enemyDirty =
    true;

  statusDirty =
    true;

  startPlayerAttack(
    direction,
    perfect
  );

  playAttackSound(
    direction
  );

  if (
    perfect
  ) {
    showFeedback(
      "PERFECT!",
      C_CYAN,
      480
    );
  }

  else if (
    absError <=
    500
  ) {
    showFeedback(
      "GOOD!",
      C_GREEN,
      450
    );
  }

  else {
    showFeedback(
      "OFFBEAT!",
      C_YELLOW,
      450
    );
  }

  sequenceIndex++;

  statusDirty =
    true;

  if (
    sequenceIndex >=
      sequenceCount &&
    enemyHP <=
      0
  ) {
    if (
      levelNumber >
      bestLevel
    ) {
      bestLevel =
        levelNumber;

      prefs.putInt(
        "best",
        bestLevel
      );
    }

    startSfx(
      sfxClear,
      sizeof(sfxClear) /
      sizeof(sfxClear[0])
    );

    changeState(
      STATE_LEVEL_CLEAR
    );
  }
}

void registerWrongMove(
  int direction
) {
  misses++;

  wrongThisRound++;

  combo =
    0;

  padFlashUntil[
    direction
  ] =
    millis() +
    160;

  dpadDirty =
    true;

  statusDirty =
    true;

  showFeedback(
    "WRONG!",
    C_RED,
    450
  );

  startSfx(
    sfxWrong,
    sizeof(sfxWrong) /
    sizeof(sfxWrong[0])
  );

  if (
    wrongThisRound >=
    3
  ) {
    hearts--;

    enemyDirty =
      true;

    if (
      hearts <=
      0
    ) {
      hearts =
        0;

      startSfx(
        sfxFail,
        sizeof(sfxFail) /
        sizeof(sfxFail[0])
      );

      changeState(
        STATE_GAME_OVER
      );
    }

    else {
      startSfx(
        sfxEnemyAttack,
        sizeof(sfxEnemyAttack) /
        sizeof(sfxEnemyAttack[0])
      );

      changeState(
        STATE_ENEMY_ATTACK
      );
    }
  }
}

void handleKeyEvent(
  uint8_t key
) {
  if (
    gameState ==
    STATE_TITLE
  ) {
    if (
      key ==
      KEY_START
    ) {
      startSfx(
        sfxMenuConfirm,
        sizeof(sfxMenuConfirm) /
        sizeof(sfxMenuConfirm[0])
      );

      changeState(
        STATE_DIFFICULTY
      );
    }

    return;
  }

  if (
    gameState ==
    STATE_DIFFICULTY
  ) {
    if (
      key ==
      KEY_UP
    ) {
      int oldDifficulty =
        difficulty;

      difficulty--;

      if (
        difficulty <
        0
      ) {
        difficulty =
          3;
      }

      difficultyDirtyMask |=
        (
          1 <<
          oldDifficulty
        );

      difficultyDirtyMask |=
        (
          1 <<
          difficulty
        );

      startSfx(
        sfxMenuMove,
        1
      );
    }

    else if (
      key ==
      KEY_DOWN
    ) {
      int oldDifficulty =
        difficulty;

      difficulty++;

      if (
        difficulty >
        3
      ) {
        difficulty =
          0;
      }

      difficultyDirtyMask |=
        (
          1 <<
          oldDifficulty
        );

      difficultyDirtyMask |=
        (
          1 <<
          difficulty
        );

      startSfx(
        sfxMenuMove,
        1
      );
    }

    else if (
      key ==
      KEY_START
    ) {
      startSfx(
        sfxMenuConfirm,
        sizeof(sfxMenuConfirm) /
        sizeof(sfxMenuConfirm[0])
      );

      startGame();
    }

    else if (
      key ==
      KEY_STOP
    ) {
      changeState(
        STATE_TITLE
      );
    }

    return;
  }

  if (
    gameState ==
    STATE_WATCH
  ) {
    if (
      key ==
      KEY_STOP
    ) {
      changeState(
        STATE_TITLE
      );
    }

    return;
  }

  if (
    gameState ==
    STATE_PLAY
  ) {
    if (
      key ==
      KEY_STOP
    ) {
      changeState(
        STATE_TITLE
      );

      return;
    }

    int direction =
      keyToDirection(
        key
      );

    if (
      direction <
        0 ||
      sequenceIndex >=
        sequenceCount
    ) {
      return;
    }

    int expected =
      sequenceDirection[
        sequenceIndex
      ];

    Serial.print(
      "EXPECTED "
    );

    Serial.print(
      directionName[
        expected
      ]
    );

    Serial.print(
      " | PRESSED "
    );

    Serial.println(
      directionName[
        direction
      ]
    );

    if (
      direction ==
      expected
    ) {
      registerCorrectMove(
        direction
      );
    }

    else {
      registerWrongMove(
        direction
      );
    }

    return;
  }

  if (
    gameState ==
      STATE_GAME_OVER ||
    gameState ==
      STATE_WIN
  ) {
    if (
      key ==
      KEY_START
    ) {
      changeState(
        STATE_TITLE
      );
    }
  }
}

void updateGame() {
  while (
    !keyQueueEmpty()
  ) {
    uint8_t key =
      popKey();

    handleKeyEvent(
      key
    );
  }

  if (
    gameState ==
    STATE_WATCH
  ) {
    updateWatch();
  }

  else if (
    gameState ==
    STATE_LEVEL_CLEAR
  ) {
    if (
      millis() -
      stateStart >
      1200
    ) {
      levelNumber++;

      if (
        levelNumber >
        FINAL_LEVEL
      ) {
        if (
          FINAL_LEVEL >
          bestLevel
        ) {
          bestLevel =
            FINAL_LEVEL;

          prefs.putInt(
            "best",
            bestLevel
          );
        }

        startSfx(
          sfxWin,
          sizeof(sfxWin) /
          sizeof(sfxWin[0])
        );

        changeState(
          STATE_WIN
        );
      }

      else {
        updateLevelTiming();

        generateRandomRoundSequence(
          false
        );

        configureEnemy();

        if (
          levelNumber %
            4 ==
          0 &&
          hearts <
          5
        ) {
          hearts++;
        }

        startWatch();
      }
    }
  }

  else if (
    gameState ==
    STATE_ENEMY_ATTACK
  ) {
    if (
      millis() -
      stateStart >
      1200
    ) {
      generateRandomRoundSequence(
        false
      );

      configureEnemy();

      startWatch();
    }
  }
}

void arrowUpOn(
  Adafruit_GFX &g,
  int cx,
  int cy,
  uint16_t color
) {
  g.fillTriangle(
    cx,
    cy - 12,
    cx - 11,
    cy,
    cx + 11,
    cy,
    color
  );

  g.fillRect(
    cx - 4,
    cy,
    8,
    12,
    color
  );
}

void arrowDownOn(
  Adafruit_GFX &g,
  int cx,
  int cy,
  uint16_t color
) {
  g.fillRect(
    cx - 4,
    cy - 12,
    8,
    12,
    color
  );

  g.fillTriangle(
    cx - 11,
    cy,
    cx + 11,
    cy,
    cx,
    cy + 12,
    color
  );
}

void arrowLeftOn(
  Adafruit_GFX &g,
  int cx,
  int cy,
  uint16_t color
) {
  g.fillTriangle(
    cx - 12,
    cy,
    cx,
    cy - 11,
    cx,
    cy + 11,
    color
  );

  g.fillRect(
    cx,
    cy - 4,
    12,
    8,
    color
  );
}

void arrowRightOn(
  Adafruit_GFX &g,
  int cx,
  int cy,
  uint16_t color
) {
  g.fillRect(
    cx - 12,
    cy - 4,
    12,
    8,
    color
  );

  g.fillTriangle(
    cx,
    cy - 11,
    cx + 12,
    cy,
    cx,
    cy + 11,
    color
  );
}

void drawPadButtonOn(
  Adafruit_GFX &g,
  int direction,
  int x,
  int y
) {
  bool active =
    millis() <
    padFlashUntil[
      direction
    ];

  if (
    buttonsHeld &
    directionHeldMask[
      direction
    ]
  ) {
    active =
      true;
  }

  uint16_t color =
    directionColor[
      direction
    ];

  g.fillRoundRect(
    x + 4,
    y + 5,
    46,
    42,
    7,
    C_BLACK
  );

  g.fillRoundRect(
    x,
    y,
    46,
    42,
    7,
    active ?
    color :
    C_PANEL
  );

  g.drawRoundRect(
    x,
    y,
    46,
    42,
    7,
    active ?
    C_WHITE :
    color
  );

  g.drawFastHLine(
    x + 8,
    y + 3,
    30,
    C_WHITE
  );

  g.drawFastHLine(
    x + 8,
    y + 38,
    30,
    C_BLACK
  );

  int cx =
    x +
    23;

  int cy =
    y +
    20;

  uint16_t arrowColor =
    active ?
    C_BLACK :
    color;

  if (
    direction ==
    0
  ) {
    arrowUpOn(
      g,
      cx,
      cy,
      arrowColor
    );
  }

  else if (
    direction ==
    1
  ) {
    arrowLeftOn(
      g,
      cx,
      cy,
      arrowColor
    );
  }

  else if (
    direction ==
    2
  ) {
    arrowDownOn(
      g,
      cx,
      cy,
      arrowColor
    );
  }

  else {
    arrowRightOn(
      g,
      cx,
      cy,
      arrowColor
    );
  }
}

void drawAttackInEnemyRegion() {
  if (
    !attackActive
  ) {
    return;
  }

  unsigned long age =
    millis() -
    attackStarted;

  const int travelTime =
    145;

  const int totalTime =
    390;

  if (
    age >
    totalTime
  ) {
    attackActive =
      false;

    enemyDirty =
      true;

    return;
  }

  const int targetX =
    115;

  const int targetY =
    51;

  int startX =
    115;

  int startY =
    96;

  if (
    attackDirection ==
    1
  ) {
    startX =
      8;

    startY =
      78;
  }

  else if (
    attackDirection ==
    3
  ) {
    startX =
      222;

    startY =
      78;
  }

  uint16_t color =
    directionColor[
      attackDirection
    ];

  if (
    age <
    travelTime
  ) {
    int progress =
      (
        age *
        255
      ) /
      travelTime;

    int x =
      startX +
      (
        (
          targetX -
          startX
        ) *
        progress
      ) /
      255;

    int y =
      startY +
      (
        (
          targetY -
          startY
        ) *
        progress
      ) /
      255;

    region.drawLine(
      startX,
      startY,
      x,
      y,
      color
    );

    region.drawLine(
      startX + 1,
      startY,
      x + 1,
      y,
      color
    );

    region.drawLine(
      startX - 1,
      startY,
      x - 1,
      y,
      C_WHITE
    );

    region.fillCircle(
      x,
      y,
      attackPerfect ?
      8 :
      7,
      color
    );

    region.drawCircle(
      x,
      y,
      attackPerfect ?
      10 :
      9,
      C_WHITE
    );

    region.fillCircle(
      x,
      y,
      2,
      C_WHITE
    );

    return;
  }

  unsigned long impactAge =
    age -
    travelTime;

  int radius =
    5 +
    impactAge /
    6;

  if (
    radius >
    26
  ) {
    radius =
      26;
  }

  if (
    impactAge <
    70
  ) {
    region.fillCircle(
      targetX,
      targetY,
      15,
      C_WHITE
    );

    region.fillCircle(
      targetX,
      targetY,
      11,
      color
    );

    region.fillCircle(
      targetX,
      targetY,
      5,
      C_WHITE
    );
  }

  else {
    region.fillCircle(
      targetX,
      targetY,
      6,
      color
    );

    region.fillCircle(
      targetX,
      targetY,
      3,
      C_WHITE
    );
  }

  region.drawCircle(
    targetX,
    targetY,
    radius,
    color
  );

  if (
    radius >
    8
  ) {
    region.drawCircle(
      targetX,
      targetY,
      radius - 7,
      C_WHITE
    );
  }

  if (
    attackPerfect &&
    radius +
      5 <
      32
  ) {
    region.drawCircle(
      targetX,
      targetY,
      radius + 5,
      C_CYAN
    );
  }

  if (
    attackDirection ==
    0
  ) {
    region.drawLine(
      targetX - 14,
      targetY + 16,
      targetX + 14,
      targetY - 16,
      C_WHITE
    );

    region.drawLine(
      targetX - 10,
      targetY + 16,
      targetX + 18,
      targetY - 16,
      color
    );
  }

  else if (
    attackDirection ==
    1
  ) {
    region.drawLine(
      targetX + 20,
      targetY - 9,
      targetX - 20,
      targetY + 9,
      C_WHITE
    );

    region.drawLine(
      targetX + 20,
      targetY - 5,
      targetX - 20,
      targetY + 13,
      color
    );
  }

  else if (
    attackDirection ==
    2
  ) {
    region.drawLine(
      targetX - 16,
      targetY - 15,
      targetX + 16,
      targetY + 15,
      C_WHITE
    );

    region.drawLine(
      targetX - 12,
      targetY - 15,
      targetX + 20,
      targetY + 15,
      color
    );
  }

  else {
    region.drawFastHLine(
      targetX - 22,
      targetY,
      44,
      C_WHITE
    );

    region.drawFastHLine(
      targetX - 19,
      targetY + 3,
      38,
      color
    );
  }

  int sparkDistance =
    8 +
    impactAge /
    7;

  if (
    sparkDistance >
    28
  ) {
    sparkDistance =
      28;
  }

  region.fillRect(
    targetX + sparkDistance,
    targetY,
    3,
    3,
    C_YELLOW
  );

  region.fillRect(
    targetX - sparkDistance,
    targetY - 2,
    3,
    3,
    C_WHITE
  );

  region.fillRect(
    targetX,
    targetY + sparkDistance,
    3,
    3,
    color
  );

  region.fillRect(
    targetX - 2,
    targetY - sparkDistance,
    3,
    3,
    C_YELLOW
  );

  region.fillRect(
    targetX + sparkDistance / 2,
    targetY + sparkDistance / 2,
    3,
    3,
    C_WHITE
  );

  region.fillRect(
    targetX - sparkDistance / 2,
    targetY - sparkDistance / 2,
    3,
    3,
    color
  );

  if (
    impactAge <
    190
  ) {
    region.setTextSize(
      attackPerfect ?
      2 :
      1
    );

    region.setTextColor(
      attackPerfect ?
      C_CYAN :
      C_YELLOW
    );

    region.setCursor(
      attackPerfect ?
      153 :
      164,
      35
    );

    region.print(
      attackPerfect ?
      "CRIT!" :
      "HIT!"
    );
  }
}

void renderBattleHeader() {
  region.fillScreen(
    C_DARK
  );

  uint16_t edge =
    levelNumber %
      5 ==
      0 ?
    C_ORANGE :
    C_CYAN;

  region.drawFastHLine(
    0,
    0,
    REGION_W,
    edge
  );

  region.setTextSize(
    1
  );

  region.setTextColor(
    C_CYAN
  );

  region.setCursor(
    7,
    4
  );

  region.print(
    "MX CLUB"
  );

  region.setTextColor(
    levelNumber %
      5 ==
      0 ?
    C_ORANGE :
    C_WHITE
  );

  region.setCursor(
    62,
    4
  );

  region.print(
    levelNumber %
      5 ==
      0 ?
    "BOSS BATTLE" :
    "MEMORY BATTLE"
  );

  tft.drawRGBBitmap(
    5,
    5,
    region.getBuffer(),
    REGION_W,
    15
  );

  updateButtons();
}

void renderEnemyRegion() {
  region.fillScreen(
    C_DARK
  );

  uint16_t edge =
    levelNumber %
      5 ==
      0 ?
    C_ORANGE :
    C_CYAN;

  region.drawFastVLine(
    0,
    0,
    95,
    edge
  );

  region.drawFastVLine(
    REGION_W - 1,
    0,
    95,
    edge
  );

  region.setTextSize(
    1
  );

  region.setTextColor(
    C_WHITE
  );

  region.setCursor(
    8,
    5
  );

  region.print(
    "LV "
  );

  region.print(
    levelNumber
  );

  region.setCursor(
    46,
    5
  );

  region.print(
    "SEQ "
  );

  region.print(
    sequenceCount
  );

  if (
    levelNumber %
      5 ==
    0
  ) {
    region.fillRoundRect(
      84,
      1,
      70,
      15,
      3,
      C_RED
    );

    region.drawRoundRect(
      84,
      1,
      70,
      15,
      3,
      C_YELLOW
    );

    region.setTextColor(
      C_WHITE
    );

    region.setCursor(
      96,
      5
    );

    region.print(
      "BOSS WAVE"
    );
  }

  else if (
    gameState ==
    STATE_PLAY
  ) {
    region.setCursor(
      92,
      5
    );

    region.print(
      "STEP "
    );

    region.print(
      sequenceIndex +
      1
    );

    region.print(
      "/"
    );

    region.print(
      sequenceCount
    );
  }

  for (
    int i = 0;
    i < hearts;
    i++
  ) {
    drawHeartOn(
      region,
      158 +
      i *
      15,
      2,
      C_RED
    );
  }

  centerTextOn(
    region,
    enemyName(
      enemyType
    ),
    REGION_W,
    18,
    levelNumber %
      5 ==
      0 ?
    C_ORANGE :
    C_WHITE,
    1
  );

  drawEnemyOn(
    region,
    115,
    52,
    enemyType
  );

  drawHPBarOn(
    region
  );

  drawAttackInEnemyRegion();

  tft.drawRGBBitmap(
    5,
    20,
    region.getBuffer(),
    REGION_W,
    95
  );

  updateButtons();
}

void renderStatusRegion() {
  region.fillScreen(
    C_PANEL
  );

  region.drawFastVLine(
    0,
    0,
    74,
    C_PURPLE
  );

  region.drawFastVLine(
    REGION_W - 1,
    0,
    74,
    C_PURPLE
  );

  if (
    gameState ==
    STATE_WATCH
  ) {
    centerTextOn(
      region,
      levelNumber %
        5 ==
        0 ?
      "BOSS RHYTHM!" :
      "WATCH!",
      REGION_W,
      4,
      levelNumber %
        5 ==
        0 ?
      C_ORANGE :
      C_YELLOW,
      2
    );

    char text[30];

    snprintf(
      text,
      sizeof(text),
      "MEMORIZE %d MOVE%s",
      sequenceCount,
      sequenceCount ==
        1 ?
      "" :
      "S"
    );

    centerTextOn(
      region,
      text,
      REGION_W,
      27,
      C_WHITE,
      1
    );
  }

  else if (
    gameState ==
    STATE_PLAY
  ) {
    centerTextOn(
      region,
      "YOUR TURN!",
      REGION_W,
      4,
      C_GREEN,
      2
    );

    char text[30];

    snprintf(
      text,
      sizeof(text),
      "STEP %d OF %d",
      sequenceIndex +
      1,
      sequenceCount
    );

    centerTextOn(
      region,
      text,
      REGION_W,
      27,
      C_WHITE,
      1
    );
  }

  else if (
    gameState ==
    STATE_LEVEL_CLEAR
  ) {
    centerTextOn(
      region,
      levelNumber %
        5 ==
        0 ?
      "BOSS DEFEATED!" :
      "ENEMY DOWN!",
      REGION_W,
      4,
      C_GREEN,
      2
    );

    centerTextOn(
      region,
      "+1 RANDOM MOVE",
      REGION_W,
      27,
      C_YELLOW,
      1
    );
  }

  else if (
    gameState ==
    STATE_ENEMY_ATTACK
  ) {
    centerTextOn(
      region,
      "ENEMY ATTACK!",
      REGION_W,
      4,
      C_RED,
      2
    );

    centerTextOn(
      region,
      "RETRY THE CHAIN",
      REGION_W,
      27,
      C_WHITE,
      1
    );

    if (
      (
        millis() /
        90
      ) %
      2
    ) {
      region.drawRect(
        2,
        2,
        226,
        66,
        C_RED
      );
    }
  }

  if (
    millis() <
    feedbackUntil
  ) {
    centerTextOn(
      region,
      feedbackText,
      REGION_W,
      43,
      feedbackColor,
      1
    );
  }

  else {
    char text[36];

    snprintf(
      text,
      sizeof(text),
      "COMBO %d   WRONG %d/3",
      combo,
      wrongThisRound
    );

    centerTextOn(
      region,
      text,
      REGION_W,
      43,
      C_WHITE,
      1
    );
  }

  if (
    levelNumber %
      5 ==
      0 &&
    musicMode ==
      MUSIC_BOSS
  ) {
    char bossText[24];

    snprintf(
      bossText,
      sizeof(bossText),
      "BOSS SYNC %d/3",
      getBossMusicLayer() +
      1
    );

    centerTextOn(
      region,
      bossText,
      REGION_W,
      55,
      C_ORANGE,
      1
    );
  }

  else {
    int visible =
      sequenceCount;

    if (
      visible >
      12
    ) {
      visible =
        12;
    }

    if (
      visible >
      0
    ) {
      int spacing =
        190 /
        visible;

      int startX =
        (
          REGION_W -
          spacing *
          visible
        ) /
        2 +
        spacing /
        2;

      for (
        int i = 0;
        i < visible;
        i++
      ) {
        uint16_t color =
          i <
            sequenceIndex ?
          C_GREEN :
          C_DIM;

        int x =
          startX +
          i *
          spacing;

        region.fillCircle(
          x,
          61,
          3,
          color
        );

        region.drawCircle(
          x,
          61,
          4,
          C_WHITE
        );
      }
    }
  }

  if (
    !pcfIsOnline()
  ) {
    centerTextOn(
      region,
      "PCF LOST",
      REGION_W,
      67,
      C_RED,
      1
    );
  }

  else if (
    millis() <
    lastInputUntil
  ) {
    char inputLine[28];

    snprintf(
      inputLine,
      sizeof(inputLine),
      "INPUT %s",
      lastInputText
    );

    centerTextOn(
      region,
      inputLine,
      REGION_W,
      67,
      C_CYAN,
      1
    );
  }

  tft.drawRGBBitmap(
    5,
    125,
    region.getBuffer(),
    REGION_W,
    74
  );

  updateButtons();
}

void renderDPadRegion() {
  region.fillScreen(
    0x0008
  );

  region.drawFastVLine(
    0,
    0,
    100,
    C_CYAN
  );

  region.drawFastVLine(
    REGION_W - 1,
    0,
    100,
    C_CYAN
  );

  drawPadButtonOn(
    region,
    0,
    92,
    0
  );

  drawPadButtonOn(
    region,
    1,
    43,
    45
  );

  drawPadButtonOn(
    region,
    2,
    92,
    45
  );

  drawPadButtonOn(
    region,
    3,
    141,
    45
  );

  region.setTextSize(
    1
  );

  region.setTextColor(
    C_DIM
  );

  region.setCursor(
    3,
    89
  );

  region.print(
    "START"
  );

  region.setCursor(
    194,
    89
  );

  region.print(
    "STOP"
  );

  if (
    gameState ==
      STATE_ENEMY_ATTACK &&
    (
      (
        millis() /
        80
      ) %
      2
    )
  ) {
    region.drawRoundRect(
      37,
      0,
      160,
      94,
      8,
      C_RED
    );

    region.drawRoundRect(
      38,
      1,
      158,
      92,
      8,
      C_ORANGE
    );
  }

  tft.drawRGBBitmap(
    5,
    210,
    region.getBuffer(),
    REGION_W,
    100
  );

  updateButtons();
}

void drawBackground() {
  tft.fillScreen(
    C_BG
  );

  for (
    int x = 0;
    x < SCREEN_W;
    x += 24
  ) {
    tft.drawFastVLine(
      x,
      0,
      SCREEN_H,
      C_GRID
    );
  }

  for (
    int y = 0;
    y < SCREEN_H;
    y += 24
  ) {
    tft.drawFastHLine(
      0,
      y,
      SCREEN_W,
      C_GRID
    );
  }
}

void drawTitleBase() {
  drawBackground();

  panel3DOn(
    tft,
    10,
    10,
    220,
    300,
    C_DARK,
    C_CYAN
  );

  centerTextOn(
    tft,
    "MX CLUB",
    SCREEN_W,
    23,
    C_CYAN,
    2
  );

  centerTextOn(
    tft,
    "RHYTHM",
    SCREEN_W,
    50,
    C_WHITE,
    3
  );

  centerTextOn(
    tft,
    "MEMORY BATTLE",
    SCREEN_W,
    81,
    C_WHITE,
    2
  );

  panel3DOn(
    tft,
    42,
    211,
    156,
    38,
    C_PANEL,
    C_GREEN
  );

  centerTextOn(
    tft,
    "PRESS START",
    SCREEN_W,
    223,
    C_GREEN,
    2
  );

  centerTextOn(
    tft,
    "BUILD THE CHAIN",
    SCREEN_W,
    263,
    C_YELLOW,
    1
  );

  centerTextOn(
    tft,
    "BOSS EVERY 5 LEVELS",
    SCREEN_W,
    279,
    C_WHITE,
    1
  );

  menuAnimDirty =
    true;

  indicatorDirty =
    true;
}

void renderMenuAnimation() {
  region.fillScreen(
    C_DARK
  );

  panel3DOn(
    region,
    5,
    2,
    220,
    94,
    C_PANEL,
    C_PURPLE
  );

  unsigned long now =
    millis();

  int pulse =
    (
      now /
      80
    ) %
    12;

  if (
    pulse >
    6
  ) {
    pulse =
      12 -
      pulse;
  }

  region.drawCircle(
    115,
    50,
    31 +
    pulse,
    C_PURPLE
  );

  region.drawCircle(
    115,
    50,
    24 +
    pulse,
    C_CYAN
  );

  for (
    int i = 0;
    i < 10;
    i++
  ) {
    int x =
      (
        i *
        29 +
        now /
        18
      ) %
      214 +
      8;

    int y =
      12 +
      (
        i *
        17
      ) %
      72;

    region.fillRect(
      x,
      y,
      2,
      2,
      i %
        2 ?
      C_CYAN :
      C_PINK
    );
  }

  menuEnemyType =
    (
      now /
      1800
    ) %
    5;

  unsigned long savedHit =
    enemyHitUntil;

  enemyHitUntil =
    0;

  drawEnemyOn(
    region,
    115,
    49,
    menuEnemyType
  );

  enemyHitUntil =
    savedHit;

  region.fillRoundRect(
    13,
    74,
    76,
    14,
    3,
    C_BLACK
  );

  region.drawRoundRect(
    13,
    74,
    76,
    14,
    3,
    C_CYAN
  );

  region.setTextSize(
    1
  );

  region.setTextColor(
    C_WHITE
  );

  region.setCursor(
    18,
    78
  );

  region.print(
    enemyName(
      menuEnemyType
    )
  );

  tft.drawRGBBitmap(
    5,
    103,
    region.getBuffer(),
    REGION_W,
    100
  );

  updateButtons();
}

void drawTitleInputIndicator() {
  region.fillScreen(
    C_DARK
  );

  if (
    !pcfIsOnline()
  ) {
    centerTextOn(
      region,
      "PCF LOST - CHECK SDA/SCL",
      REGION_W,
      3,
      C_RED,
      1
    );
  }

  else if (
    millis() <
    lastInputUntil
  ) {
    char text[28];

    snprintf(
      text,
      sizeof(text),
      "INPUT: %s",
      lastInputText
    );

    centerTextOn(
      region,
      text,
      REGION_W,
      3,
      C_GREEN,
      1
    );
  }

  else {
    char text[28];

    snprintf(
      text,
      sizeof(text),
      "PCF 0x%02X READY",
      pcfAddress
    );

    centerTextOn(
      region,
      text,
      REGION_W,
      3,
      C_CYAN,
      1
    );
  }

  tft.drawRGBBitmap(
    5,
    296,
    region.getBuffer(),
    REGION_W,
    16
  );
}

void drawDifficultyBase() {
  drawBackground();

  panel3DOn(
    tft,
    10,
    10,
    220,
    300,
    C_DARK,
    C_PURPLE
  );

  centerTextOn(
    tft,
    "SELECT MODE",
    SCREEN_W,
    22,
    C_CYAN,
    2
  );

  centerTextOn(
    tft,
    "UP / DOWN SELECT",
    SCREEN_W,
    252,
    C_WHITE,
    1
  );

  centerTextOn(
    tft,
    "START = PLAY",
    SCREEN_W,
    267,
    C_GREEN,
    1
  );

  centerTextOn(
    tft,
    "STOP = BACK",
    SCREEN_W,
    282,
    C_RED,
    1
  );

  difficultyDirtyMask =
    0x0F;

  indicatorDirty =
    true;
}

void renderDifficultyOption(
  int index
) {
  region.fillScreen(
    C_DARK
  );

  bool selected =
    index ==
    difficulty;

  panel3DOn(
    region,
    28,
    2,
    174,
    38,
    selected ?
    C_CYAN :
    C_PANEL,
    selected ?
    C_WHITE :
    C_DIM
  );

  if (
    selected
  ) {
    region.fillTriangle(
      12,
      21,
      21,
      14,
      21,
      28,
      C_YELLOW
    );
  }

  region.setTextSize(
    2
  );

  region.setTextColor(
    selected ?
    C_BLACK :
    C_WHITE
  );

  region.setCursor(
    43,
    8
  );

  region.print(
    difficultyName[
      index
    ]
  );

  region.setTextSize(
    1
  );

  region.setTextColor(
    selected ?
    C_BLUE :
    C_DIM
  );

  region.setCursor(
    43,
    27
  );

  region.print(
    difficultyDescription[
      index
    ]
  );

  int screenY =
    67 +
    index *
    44;

  tft.drawRGBBitmap(
    5,
    screenY,
    region.getBuffer(),
    REGION_W,
    42
  );

  updateButtons();
}

void drawBattleBase() {
  drawBackground();

  panel3DOn(
    tft,
    5,
    5,
    230,
    112,
    C_DARK,
    C_CYAN
  );

  panel3DOn(
    tft,
    5,
    120,
    230,
    80,
    C_PANEL,
    C_PURPLE
  );

  panel3DOn(
    tft,
    5,
    205,
    230,
    108,
    0x0008,
    C_CYAN
  );

  battleHeaderDirty =
    true;

  enemyDirty =
    true;

  statusDirty =
    true;

  dpadDirty =
    true;
}

void drawGameOver() {
  drawBackground();

  panel3DOn(
    tft,
    10,
    10,
    220,
    300,
    C_DARK,
    C_RED
  );

  centerTextOn(
    tft,
    "GAME OVER",
    SCREEN_W,
    42,
    C_RED,
    3
  );

  drawEnemyOn(
    tft,
    120,
    130,
    enemyType
  );

  char text[30];

  snprintf(
    text,
    sizeof(text),
    "LEVEL %d",
    levelNumber
  );

  centerTextOn(
    tft,
    text,
    SCREEN_W,
    190,
    C_YELLOW,
    2
  );

  snprintf(
    text,
    sizeof(text),
    "%d MOVE CHAIN",
    sequenceCount
  );

  centerTextOn(
    tft,
    text,
    SCREEN_W,
    220,
    C_CYAN,
    1
  );

  centerTextOn(
    tft,
    "PRESS START",
    SCREEN_W,
    273,
    C_GREEN,
    2
  );
}

void drawWin() {
  drawBackground();

  panel3DOn(
    tft,
    10,
    10,
    220,
    300,
    C_DARK,
    C_GREEN
  );

  centerTextOn(
    tft,
    "RAID CLEAR!",
    SCREEN_W,
    42,
    C_GREEN,
    3
  );

  drawEnemyOn(
    tft,
    120,
    130,
    4
  );

  centerTextOn(
    tft,
    "MEMORY MASTER",
    SCREEN_W,
    196,
    C_CYAN,
    2
  );

  centerTextOn(
    tft,
    "PRESS START",
    SCREEN_W,
    273,
    C_GREEN,
    2
  );
}

void drawWholeScreen() {
  if (
    gameState ==
    STATE_TITLE
  ) {
    drawTitleBase();
  }

  else if (
    gameState ==
    STATE_DIFFICULTY
  ) {
    drawDifficultyBase();
  }

  else if (
    isBattleState(
      gameState
    )
  ) {
    drawBattleBase();
  }

  else if (
    gameState ==
    STATE_GAME_OVER
  ) {
    drawGameOver();
  }

  else {
    drawWin();
  }
}

void updateVisualDirtyFlags() {
  unsigned long now =
    millis();

  bool pcfOnlineNow =
    pcfIsOnline();

  if (
    pcfOnlineNow !=
    lastPCFUiOnline
  ) {
    lastPCFUiOnline =
      pcfOnlineNow;

    indicatorDirty =
      true;

    statusDirty =
      true;
  }

  if (
    gameState ==
      STATE_TITLE &&
    now -
      lastMenuFrame >=
      150
  ) {
    lastMenuFrame =
      now;

    menuAnimDirty =
      true;
  }

  uint8_t padMask =
    0;

  for (
    int i = 0;
    i < 4;
    i++
  ) {
    bool active =
      now <
      padFlashUntil[i];

    if (
      buttonsHeld &
      directionHeldMask[i]
    ) {
      active =
        true;
    }

    if (
      active
    ) {
      padMask |=
        (
          1 <<
          i
        );
    }
  }

  if (
    padMask !=
    lastPadVisualMask
  ) {
    lastPadVisualMask =
      padMask;

    dpadDirty =
      true;
  }

  bool feedbackVisible =
    now <
    feedbackUntil;

  if (
    feedbackVisible !=
    feedbackWasVisible
  ) {
    feedbackWasVisible =
      feedbackVisible;

    statusDirty =
      true;
  }

  bool inputVisible =
    now <
    lastInputUntil;

  if (
    inputVisible !=
    inputWasVisible
  ) {
    inputWasVisible =
      inputVisible;

    indicatorDirty =
      true;

    statusDirty =
      true;
  }

  if (
    isBattleState(
      gameState
    ) &&
    !attackActive &&
    now >=
      enemyHitUntil
  ) {
    if (
      now -
      lastIdleEnemyFrame >=
      260
    ) {
      lastIdleEnemyFrame =
        now;

      enemyDirty =
        true;
    }
  }

  if (
    attackActive ||
    now <
      enemyHitUntil
  ) {
    if (
      now -
      lastFastEnemyFrame >=
      35
    ) {
      lastFastEnemyFrame =
        now;

      enemyDirty =
        true;
    }
  }

  if (
    gameState ==
      STATE_ENEMY_ATTACK &&
    now -
      lastAttackFrame >=
      80
  ) {
    lastAttackFrame =
      now;

    statusDirty =
      true;

    dpadDirty =
      true;
  }
}

void flushDifficultyCards() {
  if (
    gameState !=
    STATE_DIFFICULTY
  ) {
    return;
  }

  for (
    int i = 0;
    i < 4;
    i++
  ) {
    if (
      difficultyDirtyMask &
      (
        1 <<
        i
      )
    ) {
      difficultyDirtyMask &=
        ~(
          1 <<
          i
        );

      renderDifficultyOption(
        i
      );
    }
  }
}

void flushBattleRegions() {
  if (
    !isBattleState(
      gameState
    )
  ) {
    return;
  }

  if (
    battleHeaderDirty
  ) {
    battleHeaderDirty =
      false;

    renderBattleHeader();
  }

  if (
    enemyDirty
  ) {
    enemyDirty =
      false;

    renderEnemyRegion();
  }

  if (
    statusDirty
  ) {
    statusDirty =
      false;

    renderStatusRegion();
  }

  if (
    dpadDirty
  ) {
    dpadDirty =
      false;

    renderDPadRegion();
  }
}

void setup() {
  Serial.begin(
    115200
  );

  delay(
    250
  );

  randomSeed(
    micros()
  );

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  Wire.setClock(
    100000
  );

  detectPCF();

  if (
    pcfFound
  ) {
    Serial.print(
      "PCF8574 FOUND AT 0x"
    );

    Serial.println(
      pcfAddress,
      HEX
    );

    pcfWrite(
      0xFF
    );

    delay(
      20
    );

    initializeButtonFilter();
  }

  else {
    Serial.println(
      "PCF8574 NOT FOUND"
    );
  }

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  noTone(
    BUZZER_PIN
  );

  SPI.begin(
    TFT_SCLK,
    TFT_MISO,
    TFT_MOSI,
    TFT_CS
  );

  tft.init(
    240,
    320
  );

  tft.setRotation(
    2
  );

  tft.setTextWrap(
    false
  );

  tft.fillScreen(
    C_BLACK
  );

  region.setTextWrap(
    false
  );

  prefs.begin(
    "mxrhythm",
    false
  );

  bestLevel =
    prefs.getInt(
      "best",
      1
    );

  gameState =
    STATE_TITLE;

  stateStart =
    millis();

  screenDirty =
    true;

  lastPCFUiOnline =
    pcfIsOnline();

  setMusicMode(
    MUSIC_MENU,
    true
  );

  Serial.println();

  Serial.println(
    "===================================="
  );

  Serial.println(
    "MX CLUB RHYTHM MEMORY V15"
  );

  Serial.println(
    "FAST BUTTON DEBUG + PCF RECOVERY"
  );

  Serial.println(
    "BOSS MUSIC BUILDS IN 3 STAGES"
  );

  Serial.println(
    "BUTTON MAP: P0 START, P1 STOP, P2 UP, P3 LEFT, P4 DOWN, P5 RIGHT"
  );

  Serial.println(
    "BUTTONS MUST SHORT PCF PIN TO GND WHEN PRESSED"
  );

  Serial.println(
    "===================================="
  );
}

void loop() {
  servicePCFConnection();

  updateButtons();

  updateAudio();

  updateGame();

  updateVisualDirtyFlags();

  if (
    screenDirty
  ) {
    screenDirty =
      false;

    drawWholeScreen();

    updateButtons();

    updateAudio();

    updateGame();
  }

  if (
    gameState ==
      STATE_TITLE &&
    menuAnimDirty
  ) {
    menuAnimDirty =
      false;

    renderMenuAnimation();
  }

  flushDifficultyCards();

  if (
    indicatorDirty
  ) {
    indicatorDirty =
      false;

    if (
      gameState ==
        STATE_TITLE ||
      gameState ==
        STATE_DIFFICULTY
    ) {
      drawTitleInputIndicator();
    }
  }

  flushBattleRegions();

  updateButtons();

  updateAudio();

  updateGame();

  delay(
    1
  );
}