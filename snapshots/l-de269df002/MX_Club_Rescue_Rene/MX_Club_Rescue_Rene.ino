#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// MX CLUB GAMES
// RESCUE RENE - BATEBRONIA
//
// PLAYER 1:
// GPIO25 -> BUTTON -> GND
//
// PLAYER 2:
// GPIO26 -> BUTTON -> GND
//
// WiFi:
// MX-CLUB-RESCUE
// Password: mxclub123
//
// Open:
// http://192.168.4.1
// ============================================================


// ============================================================
// WIFI
// ============================================================

const char* WIFI_NAME = "MX-CLUB-RESCUE";
const char* WIFI_PASSWORD = "REDACTED_CONFIG_VALUE";

WebServer server(80);


// ============================================================
// BUTTONS
// ============================================================

#define PLAYER1_PIN 25
#define PLAYER2_PIN 26

const unsigned long DEBOUNCE_MS = 18;


// ============================================================
// RACE SETTINGS
//
// 60 presses = 60 meters
// ============================================================

#define TARGET_DISTANCE 60

int player1Score = 0;
int player2Score = 0;

int winner = 0;


// ============================================================
// GAME STATE
// ============================================================

enum GameState {

  GAME_IDLE,

  GAME_COUNTDOWN,

  GAME_RACING,

  GAME_FINISHED
};


GameState gameState =
  GAME_IDLE;


unsigned long countdownStarted = 0;


// ============================================================
// BUTTON EDGE DETECTION
// ============================================================

bool lastP1 = HIGH;
bool lastP2 = HIGH;

unsigned long lastP1Press = 0;
unsigned long lastP2Press = 0;


// ============================================================
// RESET
// ============================================================

void resetGame() {

  player1Score = 0;

  player2Score = 0;

  winner = 0;

  gameState =
    GAME_IDLE;


  lastP1 =
    digitalRead(
      PLAYER1_PIN
    );


  lastP2 =
    digitalRead(
      PLAYER2_PIN
    );


  Serial.println(
    "MISSION RESET"
  );
}


// ============================================================
// START
// ============================================================

void startRace() {

  player1Score = 0;

  player2Score = 0;

  winner = 0;


  countdownStarted =
    millis();


  gameState =
    GAME_COUNTDOWN;


  lastP1 =
    digitalRead(
      PLAYER1_PIN
    );


  lastP2 =
    digitalRead(
      PLAYER2_PIN
    );


  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "RESCUE MISSION STARTING"
  );

  Serial.println(
    "=========================="
  );
}


// ============================================================
// REGISTER PLAYER PRESS
// ============================================================

void playerPressed(
  int player
) {

  if (
    gameState !=
    GAME_RACING
  ) {

    return;
  }


  // ==========================================================
  // PLAYER 1
  // ==========================================================

  if (
    player == 1
  ) {

    if (
      player1Score <
      TARGET_DISTANCE
    ) {

      player1Score++;
    }


    Serial.print(
      "P1 distance: "
    );

    Serial.println(
      player1Score
    );


    if (
      player1Score >=
      TARGET_DISTANCE
    ) {

      winner = 1;

      gameState =
        GAME_FINISHED;


      Serial.println(
        "PLAYER 1 RESCUED RENE!"
      );
    }
  }


  // ==========================================================
  // PLAYER 2
  // ==========================================================

  else if (
    player == 2
  ) {

    if (
      player2Score <
      TARGET_DISTANCE
    ) {

      player2Score++;
    }


    Serial.print(
      "P2 distance: "
    );

    Serial.println(
      player2Score
    );


    if (
      player2Score >=
      TARGET_DISTANCE
    ) {

      winner = 2;

      gameState =
        GAME_FINISHED;


      Serial.println(
        "PLAYER 2 RESCUED RENE!"
      );
    }
  }
}


// ============================================================
// COUNTDOWN
// ============================================================

void updateCountdown() {

  if (
    gameState !=
    GAME_COUNTDOWN
  ) {

    return;
  }


  unsigned long elapsed =
    millis() -
    countdownStarted;


  if (
    elapsed >= 3000
  ) {

    gameState =
      GAME_RACING;


    lastP1 =
      digitalRead(
        PLAYER1_PIN
      );


    lastP2 =
      digitalRead(
        PLAYER2_PIN
      );


    Serial.println(
      "RESCUE!"
    );
  }
}


// ============================================================
// COUNTDOWN VALUE
// ============================================================

int getCountdownNumber() {

  if (
    gameState !=
    GAME_COUNTDOWN
  ) {

    return 0;
  }


  unsigned long elapsed =
    millis() -
    countdownStarted;


  if (
    elapsed < 1000
  ) {

    return 3;
  }


  if (
    elapsed < 2000
  ) {

    return 2;
  }


  if (
    elapsed < 3000
  ) {

    return 1;
  }


  return 0;
}


// ============================================================
// BUTTON READING
// ============================================================

void updateButtons() {

  bool p1 =
    digitalRead(
      PLAYER1_PIN
    );


  bool p2 =
    digitalRead(
      PLAYER2_PIN
    );


  unsigned long now =
    millis();


  // PLAYER 1

  if (
    p1 == LOW &&
    lastP1 == HIGH &&
    now - lastP1Press >=
    DEBOUNCE_MS
  ) {

    lastP1Press =
      now;


    playerPressed(
      1
    );
  }


  // PLAYER 2

  if (
    p2 == LOW &&
    lastP2 == HIGH &&
    now - lastP2Press >=
    DEBOUNCE_MS
  ) {

    lastP2Press =
      now;


    playerPressed(
      2
    );
  }


  lastP1 =
    p1;


  lastP2 =
    p2;
}


// ============================================================
// WEBSITE
// ============================================================

const char PAGE[] PROGMEM =
R"HTML(

<!DOCTYPE html>

<html>

<head>

<meta
  name="viewport"
  content="width=device-width, initial-scale=1"
/>

<title>
MX Club - Rescue Rene
</title>


<style>

/* ============================================================
   GLOBAL
============================================================ */

* {

  box-sizing:
    border-box;
}


:root {

  --white:
    #eefcff;

  --deep:
    #02101c;

  --deep2:
    #051a2b;

  --panel:
    #061e31;

  --border:
    #bceeff;

  --ocean1:
    #159bc8;

  --ocean2:
    #0c78aa;

  --ocean3:
    #075a82;

  --ocean4:
    #033c5d;

  --ocean5:
    #022842;

  --p1:
    #25dfff;

  --p2:
    #ffab35;

  --rene:
    #ffe35e;

  --safe:
    #52f29a;

  --purple:
    #43206d;
}


html,
body {

  margin:
    0;

  min-height:
    100%;

  background:
    #010813;

  color:
    var(--white);

  font-family:
    "Courier New",
    monospace;
}


body {

  padding:
    12px;
}


/* ============================================================
   MAIN CONTAINER
============================================================ */

.machine {

  width:
    min(
      1400px,
      100%
    );

  margin:
    auto;

  border:
    4px solid
    var(--white);

  background:
    var(--deep);

  box-shadow:
    10px 10px 0
    #00050b;
}


/* ============================================================
   HEADER
============================================================ */

.header {

  display:
    grid;

  grid-template-columns:
    120px
    1fr
    120px;

  align-items:
    center;

  gap:
    18px;

  padding:
    16px 20px;

  border-bottom:
    4px solid
    var(--white);

  background:
    #061521;
}


/* ============================================================
   REAL MX CLUB LOGO
============================================================ */

.logoBox {

  width:
    100px;

  height:
    100px;

  display:
    grid;

  place-items:
    center;

  background:
    white;

  border:
    3px solid
    #8d61c9;

  padding:
    5px;
}


.logoBox img {

  width:
    88px;

  height:
    88px;

  object-fit:
    contain;
}


/* ============================================================
   TITLE
============================================================ */

.titleArea {

  text-align:
    center;
}


.club {

  font-size:
    14px;

  font-weight:
    900;

  letter-spacing:
    5px;

  color:
    #cfb7ee;
}


.title {

  margin-top:
    4px;

  font-size:
    clamp(
      38px,
      6vw,
      72px
    );

  line-height:
    .95;

  font-weight:
    900;

  letter-spacing:
    2px;

  text-shadow:
    5px 5px 0
    #064c70;
}


.subtitle {

  margin-top:
    10px;

  font-size:
    12px;

  letter-spacing:
    3px;

  opacity:
    .75;
}


.headerBadge {

  justify-self:
    end;

  min-width:
    105px;

  padding:
    12px 8px;

  text-align:
    center;

  border:
    3px solid
    var(--safe);

  background:
    #062019;

  color:
    var(--safe);

  font-size:
    11px;

  font-weight:
    900;

  line-height:
    1.5;
}


/* ============================================================
   STATUS
============================================================ */

.statusBar {

  display:
    grid;

  grid-template-columns:
    1fr
    auto
    1fr;

  align-items:
    center;

  gap:
    14px;

  padding:
    10px 18px;

  border-bottom:
    4px solid
    var(--white);

  background:
    var(--deep2);
}


.playerStatus {

  font-size:
    13px;

  font-weight:
    900;

  letter-spacing:
    1px;
}


.playerStatus.right {

  text-align:
    right;
}


#countdown {

  min-width:
    240px;

  padding:
    8px 18px;

  border:
    3px solid
    #7ce5ff;

  background:
    #020d17;

  text-align:
    center;

  color:
    #7ce5ff;

  font-size:
    clamp(
      28px,
      4vw,
      48px
    );

  font-weight:
    900;
}


/* ============================================================
   OCEAN
============================================================ */

.ocean {

  position:
    relative;

  overflow:
    hidden;

  border-bottom:
    4px solid
    var(--white);

  background:

    linear-gradient(

      to bottom,

      var(--ocean1)
      0%,

      var(--ocean2)
      24%,

      var(--ocean3)
      48%,

      var(--ocean4)
      72%,

      var(--ocean5)
      100%
    );
}


/* ============================================================
   OCEAN SURFACE
============================================================ */

.reflection {

  height:
    13px;

  opacity:
    .6;

  background:

    repeating-linear-gradient(

      90deg,

      rgba(
        255,
        255,
        255,
        .85
      )
      0 30px,

      transparent
      30px 55px
    );
}


.wave {

  width:
    110%;

  height:
    30px;

  margin-left:
    -60px;

  background:

    repeating-linear-gradient(

      90deg,

      #76e4ff
      0 24px,

      #40c5ed
      24px 48px,

      #1183ae
      48px 72px
    );

  clip-path:

    polygon(

      0 50%,

      4% 8%,

      8% 52%,

      12% 12%,

      16% 55%,

      20% 8%,

      24% 52%,

      28% 12%,

      32% 55%,

      36% 8%,

      40% 52%,

      44% 12%,

      48% 55%,

      52% 8%,

      56% 52%,

      60% 12%,

      64% 55%,

      68% 8%,

      72% 52%,

      76% 12%,

      80% 55%,

      84% 8%,

      88% 52%,

      92% 12%,

      96% 55%,

      100% 20%,

      100% 100%,

      0 100%
    );

  animation:

    waveMove
    1.1s
    steps(6)
    infinite;
}


@keyframes waveMove {

  to {

    transform:
      translateX(
        72px
      );
  }
}


/* ============================================================
   LARGE RACE TRACK
============================================================ */

.raceStage {

  position:
    relative;

  height:
    520px;

  min-height:
    520px;

  overflow:
    hidden;
}


/* ============================================================
   WATER LIGHT RAYS
============================================================ */

.lightRay {

  position:
    absolute;

  top:
    -20px;

  width:
    80px;

  height:
    470px;

  background:

    linear-gradient(

      to bottom,

      rgba(
        255,
        255,
        255,
        .16
      ),

      transparent
    );

  transform:
    skewX(
      -10deg
    );

  opacity:
    .28;
}


.lightRay.r1 {

  left:
    16%;
}


.lightRay.r2 {

  left:
    43%;

  width:
    55px;
}


.lightRay.r3 {

  left:
    74%;
}


/* ============================================================
   BUBBLES
============================================================ */

.bubbles {

  position:
    absolute;

  inset:
    0;

  opacity:
    .35;

  pointer-events:
    none;

  background-image:

    radial-gradient(
      circle,
      white
      0 2px,
      transparent
      3px
    ),

    radial-gradient(
      circle,
      white
      0 3px,
      transparent
      4px
    ),

    radial-gradient(
      circle,
      white
      0 1px,
      transparent
      2px
    );

  background-size:

    100px 130px,

    155px 180px,

    70px 90px;

  background-position:

    15px 20px,

    70px 100px,

    40px 60px;

  animation:

    bubbleMove
    6s
    linear
    infinite;
}


@keyframes bubbleMove {

  to {

    background-position:

      15px -110px,

      70px -80px,

      40px -30px;
  }
}


/* ============================================================
   CENTER RESCUE ZONE
============================================================ */

.rescueLine {

  position:
    absolute;

  left:
    50%;

  top:
    25px;

  bottom:
    25px;

  width:
    5px;

  transform:
    translateX(
      -50%
    );

  background:

    repeating-linear-gradient(

      to bottom,

      #ffe35e
      0 15px,

      #18384c
      15px 30px
    );

  opacity:
    .75;
}


.rescueLabel {

  position:
    absolute;

  left:
    50%;

  top:
    24px;

  transform:
    translateX(
      -50%
    );

  padding:
    5px 12px;

  background:
    #03101b;

  border:
    3px solid
    var(--rene);

  color:
    var(--rene);

  font-size:
    12px;

  font-weight:
    900;

  z-index:
    5;
}


/* ============================================================
   RENE
============================================================ */

.reneArea {

  position:
    absolute;

  left:
    50%;

  top:
    194px;

  width:
    120px;

  transform:
    translateX(
      -50%
    );

  text-align:
    center;

  z-index:
    9;
}


.rene {

  position:
    relative;

  width:
    74px;

  height:
    100px;

  margin:
    auto;

  animation:

    reneFloat
    .8s
    steps(2)
    infinite;
}


@keyframes reneFloat {

  50% {

    transform:
      translateY(
        8px
      );
  }
}


.reneHead {

  position:
    absolute;

  left:
    27px;

  top:
    2px;

  width:
    22px;

  height:
    22px;

  background:
    #efb17f;

  border:
    4px solid
    #172334;
}


.reneHair {

  position:
    absolute;

  left:
    26px;

  top:
    1px;

  width:
    24px;

  height:
    8px;

  background:
    #26170f;

  border:
    2px solid
    #172334;
}


.reneBody {

  position:
    absolute;

  left:
    23px;

  top:
    28px;

  width:
    31px;

  height:
    38px;

  background:
    var(--rene);

  border:
    4px solid
    #172334;
}


.reneArm {

  position:
    absolute;

  top:
    31px;

  width:
    23px;

  height:
    8px;

  background:
    #efb17f;

  border:
    3px solid
    #172334;

  transform-origin:
    center;
}


.reneArm.left {

  left:
    2px;

  transform:
    rotate(
      -40deg
    );

  animation:

    waveLeft
    .45s
    steps(2)
    infinite;
}


.reneArm.right {

  right:
    0;

  transform:
    rotate(
      40deg
    );

  animation:

    waveRight
    .45s
    steps(2)
    infinite;
}


@keyframes waveLeft {

  50% {

    transform:
      rotate(
        -72deg
      )
      translateY(
        -5px
      );
  }
}


@keyframes waveRight {

  50% {

    transform:
      rotate(
        72deg
      )
      translateY(
        -5px
      );
  }
}


.lifeRing {

  position:
    absolute;

  left:
    12px;

  top:
    62px;

  width:
    52px;

  height:
    23px;

  border:
    8px solid
    #ff6537;

  border-radius:
    50%;
}


.reneName {

  display:
    inline-block;

  margin-top:
    3px;

  padding:
    5px 10px;

  border:
    3px solid
    var(--rene);

  background:
    #03101b;

  color:
    var(--rene);

  font-size:
    13px;

  font-weight:
    900;
}


/* ============================================================
   SWIM LANES
============================================================ */

.lane {

  position:
    absolute;

  left:
    4%;

  right:
    4%;

  height:
    130px;

  border-top:
    3px dashed
    rgba(
      255,
      255,
      255,
      .4
    );

  border-bottom:
    3px dashed
    rgba(
      255,
      255,
      255,
      .18
    );
}


.lane1 {

  top:
    65px;
}


.lane2 {

  bottom:
    65px;
}


.laneTitle {

  position:
    absolute;

  top:
    -34px;

  padding:
    5px 9px;

  border:
    2px solid
    currentColor;

  background:
    #03111d;

  font-size:
    12px;

  font-weight:
    900;
}


.lane1 .laneTitle {

  left:
    0;

  color:
    var(--p1);
}


.lane2 .laneTitle {

  right:
    0;

  color:
    var(--p2);
}


/* ============================================================
   DIVER WRAPPER
============================================================ */

.diver {

  position:
    absolute;

  top:
    20px;

  width:
    150px;

  height:
    92px;

  z-index:
    8;

  transition:

    left
    .06s
    linear,

    right
    .06s
    linear;
}


.diver.p1 {

  left:
    0;
}


.diver.p2 {

  right:
    0;
}


/* ============================================================
   ACTUAL DIVER SPRITE
============================================================ */

.sprite {

  position:
    relative;

  width:
    142px;

  height:
    72px;

  transform-origin:
    center;
}


.p2 .sprite {

  transform:
    scaleX(
      -1
    );
}


/* ============================================================
   DIVER HEAD
============================================================ */

.diverHead {

  position:
    absolute;

  left:
    95px;

  top:
    10px;

  width:
    25px;

  height:
    25px;

  background:
    #efb17f;

  border:
    4px solid
    #102132;

  border-radius:
    3px;
}


/* ============================================================
   HAIR
============================================================ */

.diverHair {

  position:
    absolute;

  left:
    94px;

  top:
    7px;

  width:
    24px;

  height:
    8px;

  background:
    #2a1a11;

  border:
    3px solid
    #102132;
}


/* ============================================================
   MASK
============================================================ */

.diverMask {

  position:
    absolute;

  left:
    109px;

  top:
    13px;

  width:
    27px;

  height:
    15px;

  background:
    #9af2ff;

  border:
    4px solid
    #102132;
}


/* ============================================================
   SNORKEL
============================================================ */

.snorkel {

  position:
    absolute;

  left:
    129px;

  top:
    0px;

  width:
    7px;

  height:
    20px;

  border-left:
    5px solid
    #ff9b32;

  border-top:
    5px solid
    #ff9b32;
}


/* ============================================================
   TANK
============================================================ */

.tank {

  position:
    absolute;

  left:
    57px;

  top:
    23px;

  width:
    22px;

  height:
    34px;

  background:
    #7e92a5;

  border:
    4px solid
    #102132;
}


/* ============================================================
   TORSO
============================================================ */

.torso {

  position:
    absolute;

  left:
    74px;

  top:
    25px;

  width:
    41px;

  height:
    29px;

  border:
    4px solid
    #102132;

  background:
    var(--p1);
}


.p2 .torso {

  background:
    var(--p2);
}


/* ============================================================
   BELT
============================================================ */

.belt {

  position:
    absolute;

  left:
    75px;

  top:
    47px;

  width:
    39px;

  height:
    8px;

  background:
    #162435;

  border:
    2px solid
    #102132;
}


/* ============================================================
   FRONT ARM
============================================================ */

.armUpper {

  position:
    absolute;

  left:
    108px;

  top:
    31px;

  width:
    27px;

  height:
    9px;

  background:
    var(--p1);

  border:
    3px solid
    #102132;

  transform-origin:
    left center;
}


.p2 .armUpper {

  background:
    var(--p2);
}


.armLower {

  position:
    absolute;

  left:
    129px;

  top:
    27px;

  width:
    22px;

  height:
    9px;

  background:
    #efb17f;

  border:
    3px solid
    #102132;

  transform-origin:
    left center;
}


/* ============================================================
   BACK ARM
============================================================ */

.armBack {

  position:
    absolute;

  left:
    76px;

  top:
    20px;

  width:
    30px;

  height:
    8px;

  background:
    var(--p1);

  border:
    3px solid
    #102132;

  transform:
    rotate(
      -15deg
    );

  transform-origin:
    right center;
}


.p2 .armBack {

  background:
    var(--p2);
}


/* ============================================================
   LEGS
============================================================ */

.thigh1,
.thigh2 {

  position:
    absolute;

  left:
    41px;

  width:
    39px;

  height:
    10px;

  border:
    3px solid
    #102132;

  background:
    var(--p1);

  transform-origin:
    right center;
}


.p2 .thigh1,
.p2 .thigh2 {

  background:
    var(--p2);
}


.thigh1 {

  top:
    35px;

  transform:
    rotate(
      6deg
    );
}


.thigh2 {

  top:
    49px;

  transform:
    rotate(
      -8deg
    );
}


.calf1,
.calf2 {

  position:
    absolute;

  left:
    17px;

  width:
    31px;

  height:
    9px;

  border:
    3px solid
    #102132;

  background:
    var(--p1);

  transform-origin:
    right center;
}


.p2 .calf1,
.p2 .calf2 {

  background:
    var(--p2);
}


.calf1 {

  top:
    31px;

  transform:
    rotate(
      -8deg
    );
}


.calf2 {

  top:
    54px;

  transform:
    rotate(
      10deg
    );
}


/* ============================================================
   FLIPPERS
============================================================ */

.flipper1,
.flipper2 {

  position:
    absolute;

  left:
    0;

  width:
    25px;

  height:
    12px;

  background:
    #112434;

  border:
    3px solid
    #08131e;

  clip-path:

    polygon(

      100% 0,

      100% 100%,

      20% 100%,

      0 50%,

      20% 0
    );
}


.flipper1 {

  top:
    25px;

  transform:
    rotate(
      -8deg
    );
}


.flipper2 {

  top:
    56px;

  transform:
    rotate(
      10deg
    );
}


/* ============================================================
   SWIMMING ANIMATION
============================================================ */

.diver.swimming
.sprite {

  animation:

    bodySwim
    .20s
    steps(2)
    infinite;
}


@keyframes bodySwim {

  50% {

    transform:
      translateY(
        -3px
      );
  }
}


.p2.swimming
.sprite {

  animation:

    bodySwimP2
    .20s
    steps(2)
    infinite;
}


@keyframes bodySwimP2 {

  50% {

    transform:
      scaleX(
        -1
      )
      translateY(
        -3px
      );
  }
}


.diver.swimming
.thigh1 {

  animation:

    legKick1
    .18s
    steps(2)
    infinite;
}


.diver.swimming
.thigh2 {

  animation:

    legKick2
    .18s
    steps(2)
    infinite;
}


.diver.swimming
.calf1 {

  animation:

    calfKick1
    .18s
    steps(2)
    infinite;
}


.diver.swimming
.calf2 {

  animation:

    calfKick2
    .18s
    steps(2)
    infinite;
}


.diver.swimming
.armUpper {

  animation:

    stroke1
    .20s
    steps(2)
    infinite;
}


.diver.swimming
.armLower {

  animation:

    stroke2
    .20s
    steps(2)
    infinite;
}


@keyframes legKick1 {

  50% {

    transform:
      rotate(
        23deg
      );
  }
}


@keyframes legKick2 {

  50% {

    transform:
      rotate(
        -27deg
      );
  }
}


@keyframes calfKick1 {

  50% {

    transform:
      rotate(
        -20deg
      );
  }
}


@keyframes calfKick2 {

  50% {

    transform:
      rotate(
        25deg
      );
  }
}


@keyframes stroke1 {

  50% {

    transform:
      rotate(
        -25deg
      );
  }
}


@keyframes stroke2 {

  50% {

    transform:
      rotate(
        18deg
      );
  }
}


/* ============================================================
   WATER TRAIL
============================================================ */

.waterTrail {

  position:
    absolute;

  left:
    -25px;

  top:
    26px;

  color:
    rgba(
      255,
      255,
      255,
      .75
    );

  font-weight:
    900;

  font-size:
    16px;

  letter-spacing:
    -5px;

  animation:

    trail
    .45s
    steps(2)
    infinite;
}


.p2 .waterTrail {

  left:
    auto;

  right:
    -25px;
}


@keyframes trail {

  50% {

    opacity:
      .2;
  }
}


/* ============================================================
   DISTANCE TAG
============================================================ */

.distance {

  position:
    absolute;

  left:
    18px;

  top:
    76px;

  min-width:
    110px;

  padding:
    5px 7px;

  text-align:
    center;

  border:
    3px solid
    var(--p1);

  background:
    rgba(
      2,
      14,
      24,
      .95
    );

  color:
    var(--p1);

  font-size:
    11px;

  font-weight:
    900;
}


.p2 .distance {

  border-color:
    var(--p2);

  color:
    var(--p2);
}


/* ============================================================
   FISH
============================================================ */

.fish {

  position:
    absolute;

  left:
    -110px;

  color:
    rgba(
      255,
      255,
      255,
      .45
    );

  font-size:
    20px;

  font-weight:
    900;

  animation:

    fishMove
    13s
    linear
    infinite;
}


.fish1 {

  top:
    210px;
}


.fish2 {

  top:
    410px;

  animation-duration:
    17s;

  animation-delay:
    -8s;
}


@keyframes fishMove {

  to {

    transform:
      translateX(
        1550px
      );
  }
}


/* ============================================================
   SEABED
============================================================ */

.seabed {

  height:
    35px;

  border-top:
    5px solid
    #dab06a;

  background:

    repeating-linear-gradient(

      90deg,

      #ad7a41
      0 25px,

      #8d5c31
      25px 50px
    );
}


/* ============================================================
   BOTTOM INFO
============================================================ */

.infoRow {

  display:
    grid;

  grid-template-columns:
    1fr
    1fr;

  border-bottom:
    4px solid
    var(--white);
}


.playerInfo {

  padding:
    15px 18px;

  background:
    #061b2b;
}


.playerInfo:first-child {

  border-right:
    4px solid
    var(--white);
}


.infoTop {

  display:
    flex;

  align-items:
    center;

  justify-content:
    space-between;

  gap:
    10px;

  font-size:
    13px;

  font-weight:
    900;
}


.playerName1 {

  color:
    var(--p1);
}


.playerName2 {

  color:
    var(--p2);
}


.distanceBottom {

  font-size:
    16px;

  font-weight:
    900;
}


/* ============================================================
   PROGRESS TRACK
============================================================ */

.progress {

  height:
    26px;

  margin-top:
    12px;

  border:
    3px solid
    var(--white);

  background:
    #020b14;
}


.progressFill {

  width:
    0%;

  height:
    100%;

  transition:
    width
    .06s
    linear;
}


#bar1 {

  background:
    var(--p1);
}


#bar2 {

  background:
    var(--p2);
}


/* ============================================================
   GIANT WINNER BANNER
============================================================ */

#winnerBanner {

  display:
    none;

  position:
    absolute;

  left:
    4%;

  right:
    4%;

  top:
    50%;

  transform:
    translateY(
      -50%
    );

  z-index:
    50;

  padding:
    30px 20px;

  text-align:
    center;

  border:
    6px solid
    white;

  background:
    rgba(
      2,
      12,
      21,
      .97
    );

  box-shadow:
    10px 10px 0
    rgba(
      0,
      0,
      0,
      .45
    );
}


#winnerBanner.p1win {

  border-color:
    var(--p1);
}


#winnerBanner.p2win {

  border-color:
    var(--p2);
}


.winnerTop {

  font-size:
    14px;

  letter-spacing:
    5px;

  font-weight:
    900;

  color:
    var(--safe);
}


.winnerMain {

  margin-top:
    8px;

  font-size:
    clamp(
      36px,
      7vw,
      82px
    );

  line-height:
    1;

  font-weight:
    900;

  color:
    white;

  text-shadow:
    5px 5px 0
    #06364e;
}


.winnerSub {

  margin-top:
    12px;

  font-size:
    clamp(
      14px,
      2vw,
      22px
    );

  font-weight:
    900;

  letter-spacing:
    2px;

  color:
    var(--safe);
}


/* ============================================================
   CONTROLS
============================================================ */

.controls {

  display:
    flex;

  justify-content:
    center;

  flex-wrap:
    wrap;

  gap:
    12px;

  padding:
    16px;

  background:
    #04131f;
}


button {

  min-width:
    230px;

  min-height:
    58px;

  padding:
    15px 22px;

  border:
    3px solid
    white;

  background:
    #082238;

  color:
    white;

  font-family:
    "Courier New",
    monospace;

  font-size:
    17px;

  font-weight:
    900;

  cursor:
    pointer;
}


button:active {

  background:
    white;

  color:
    #03101b;
}


/* ============================================================
   FOOTER
============================================================ */

.footer {

  padding:
    10px;

  text-align:
    center;

  font-size:
    11px;

  letter-spacing:
    1px;

  opacity:
    .65;
}


/* ============================================================
   MOBILE
============================================================ */

@media(
  max-width: 760px
) {

  body {

    padding:
      4px;
  }


  .header {

    grid-template-columns:
      78px
      1fr;

    padding:
      10px;
  }


  .logoBox {

    width:
      70px;

    height:
      70px;
  }


  .logoBox img {

    width:
      61px;

    height:
      61px;
  }


  .headerBadge {

    display:
      none;
  }


  .club {

    font-size:
      9px;

    letter-spacing:
      2px;
  }


  .subtitle {

    font-size:
      9px;

    letter-spacing:
      1px;
  }


  .statusBar {

    grid-template-columns:
      1fr;
  }


  .playerStatus,
  .playerStatus.right {

    text-align:
      center;
  }


  #countdown {

    min-width:
      0;
  }


  .raceStage {

    height:
      500px;

    min-height:
      500px;
  }


  .lane {

    left:
      2%;

    right:
      2%;
  }


  .diver {

    transform:
      scale(
        .82
      );

    transform-origin:
      left center;
  }


  .p2.diver {

    transform-origin:
      right center;
  }


  .reneArea {

    transform:
      translateX(
        -50%
      )
      scale(
        .85
      );
  }


  .infoRow {

    grid-template-columns:
      1fr;
  }


  .playerInfo:first-child {

    border-right:
      none;

    border-bottom:
      4px solid
      white;
  }


  button {

    width:
      100%;
  }


  #winnerBanner {

    left:
      2%;

    right:
      2%;

    padding:
      24px 10px;
  }

}


/* ============================================================
   REDUCED MOTION
============================================================ */

@media(
  prefers-reduced-motion:
  reduce
) {

  * {

    animation:
      none !important;

    transition:
      none !important;
  }
}

</style>

</head>


<body>


<div class="machine">


  <!-- ========================================================
       HEADER
  ========================================================= -->

  <div class="header">


    <div class="logoBox">

      <img
        alt="MX Club logo"
        src="data:image/png;base64,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"
      >

    </div>


    <div class="titleArea">

      <div class="club">
        MECHATRONIX CLUB
      </div>

      <div class="title">
        RESCUE RENE
      </div>

      <div class="subtitle">
        BATEBRONIA // OCEAN RESCUE RACE
      </div>

    </div>


    <div class="headerBadge">

      MX CLUB<br>

      2 PLAYER<br>

      RESCUE GAME

    </div>


  </div>


  <!-- ========================================================
       STATUS
  ========================================================= -->

  <div class="statusBar">


    <div
      class="playerStatus"
      id="p1Top"
    >
      PLAYER 1 // READY
    </div>


    <div id="countdown">
      READY
    </div>


    <div
      class="playerStatus right"
      id="p2Top"
    >
      PLAYER 2 // READY
    </div>


  </div>


  <!-- ========================================================
       OCEAN
  ========================================================= -->

  <div class="ocean">


    <div class="reflection">
    </div>


    <div class="wave">
    </div>


    <div class="raceStage">


      <div class="lightRay r1">
      </div>


      <div class="lightRay r2">
      </div>


      <div class="lightRay r3">
      </div>


      <div class="bubbles">
      </div>


      <div class="rescueLine">
      </div>


      <div class="rescueLabel">
        RESCUE POINT
      </div>


      <!-- ====================================================
           RENE
      ===================================================== -->

      <div class="reneArea">


        <div class="rene">


          <div class="reneHead">
          </div>


          <div class="reneHair">
          </div>


          <div class="reneBody">
          </div>


          <div class="reneArm left">
          </div>


          <div class="reneArm right">
          </div>


          <div class="lifeRing">
          </div>


        </div>


        <div class="reneName">
          RENE
        </div>


      </div>


      <!-- ====================================================
           PLAYER 1 LANE
      ===================================================== -->

      <div class="lane lane1">


        <div class="laneTitle">
          PLAYER 1
        </div>


        <div
          class="diver p1"
          id="diver1"
        >


          <div class="waterTrail">
            &lt;&lt;&lt;
          </div>


          <div class="sprite">


            <div class="flipper1">
            </div>


            <div class="flipper2">
            </div>


            <div class="calf1">
            </div>


            <div class="calf2">
            </div>


            <div class="thigh1">
            </div>


            <div class="thigh2">
            </div>


            <div class="tank">
            </div>


            <div class="armBack">
            </div>


            <div class="torso">
            </div>


            <div class="belt">
            </div>


            <div class="armUpper">
            </div>


            <div class="armLower">
            </div>


            <div class="diverHead">
            </div>


            <div class="diverHair">
            </div>


            <div class="diverMask">
            </div>


            <div class="snorkel">
            </div>


          </div>


          <div
            class="distance"
            id="distance1"
          >
            60M TO RENE
          </div>


        </div>


      </div>


      <!-- ====================================================
           PLAYER 2 LANE
      ===================================================== -->

      <div class="lane lane2">


        <div class="laneTitle">
          PLAYER 2
        </div>


        <div
          class="diver p2"
          id="diver2"
        >


          <div class="waterTrail">
            &lt;&lt;&lt;
          </div>


          <div class="sprite">


            <div class="flipper1">
            </div>


            <div class="flipper2">
            </div>


            <div class="calf1">
            </div>


            <div class="calf2">
            </div>


            <div class="thigh1">
            </div>


            <div class="thigh2">
            </div>


            <div class="tank">
            </div>


            <div class="armBack">
            </div>


            <div class="torso">
            </div>


            <div class="belt">
            </div>


            <div class="armUpper">
            </div>


            <div class="armLower">
            </div>


            <div class="diverHead">
            </div>


            <div class="diverHair">
            </div>


            <div class="diverMask">
            </div>


            <div class="snorkel">
            </div>


          </div>


          <div
            class="distance"
            id="distance2"
          >
            60M TO RENE
          </div>


        </div>


      </div>


      <!-- ====================================================
           DECORATION
      ===================================================== -->

      <div class="fish fish1">
        &gt;&lt;(((°&gt;
      </div>


      <div class="fish fish2">
        &gt;&lt;(((*&gt;
      </div>


      <!-- ====================================================
           LARGE WINNER BANNER
      ===================================================== -->

      <div id="winnerBanner">


        <div class="winnerTop">
          MISSION SUCCESS
        </div>


        <div
          class="winnerMain"
          id="winnerMain"
        >
          PLAYER 1 RESCUED RENE!
        </div>


        <div class="winnerSub">
          RENE IS SAFE // RESCUE COMPLETE
        </div>


      </div>


    </div>


    <div class="seabed">
    </div>


  </div>


  <!-- ========================================================
       BOTTOM INFORMATION
  ========================================================= -->

  <div class="infoRow">


    <div class="playerInfo">


      <div class="infoTop">


        <span class="playerName1">
          PLAYER 1 RESCUE DIVER
        </span>


        <span
          class="distanceBottom"
          id="bottomDistance1"
        >
          60M AWAY
        </span>


      </div>


      <div class="progress">


        <div
          class="progressFill"
          id="bar1"
        >
        </div>


      </div>


    </div>


    <div class="playerInfo">


      <div class="infoTop">


        <span class="playerName2">
          PLAYER 2 RESCUE DIVER
        </span>


        <span
          class="distanceBottom"
          id="bottomDistance2"
        >
          60M AWAY
        </span>


      </div>


      <div class="progress">


        <div
          class="progressFill"
          id="bar2"
        >
        </div>


      </div>


    </div>


  </div>


  <!-- ========================================================
       CONTROLS
  ========================================================= -->

  <div class="controls">


    <button
      type="button"
      id="startButton"
    >
      START RESCUE
    </button>


    <button
      type="button"
      id="resetButton"
    >
      RESET MISSION
    </button>


  </div>


  <div
    class="footer"
    id="connection"
  >
    CONNECTED TO ESP32 // WAITING FOR MISSION
  </div>


</div>


<script>

// ============================================================
// GAME SETTINGS
// ============================================================

const TARGET =
  60;


// ============================================================
// ELEMENTS
// ============================================================

const diver1 =
  document.getElementById(
    "diver1"
  );


const diver2 =
  document.getElementById(
    "diver2"
  );


const distance1 =
  document.getElementById(
    "distance1"
  );


const distance2 =
  document.getElementById(
    "distance2"
  );


const bottomDistance1 =
  document.getElementById(
    "bottomDistance1"
  );


const bottomDistance2 =
  document.getElementById(
    "bottomDistance2"
  );


const bar1 =
  document.getElementById(
    "bar1"
  );


const bar2 =
  document.getElementById(
    "bar2"
  );


const countdown =
  document.getElementById(
    "countdown"
  );


const p1Top =
  document.getElementById(
    "p1Top"
  );


const p2Top =
  document.getElementById(
    "p2Top"
  );


const winnerBanner =
  document.getElementById(
    "winnerBanner"
  );


const winnerMain =
  document.getElementById(
    "winnerMain"
  );


const connection =
  document.getElementById(
    "connection"
  );


// ============================================================
// LAST SCORE FOR SWIM ANIMATION
// ============================================================

let previousP1 = 0;

let previousP2 = 0;

let swimTimer1 = null;

let swimTimer2 = null;


// ============================================================
// SWIM ANIMATION TRIGGER
// ============================================================

function animateSwim(
  diver,
  player
) {

  diver
    .classList
    .add(
      "swimming"
    );


  if (
    player === 1
  ) {

    clearTimeout(
      swimTimer1
    );


    swimTimer1 =
      setTimeout(
        function() {

          diver
            .classList
            .remove(
              "swimming"
            );

        },
        220
      );
  }


  else {

    clearTimeout(
      swimTimer2
    );


    swimTimer2 =
      setTimeout(
        function() {

          diver
            .classList
            .remove(
              "swimming"
            );

        },
        220
      );
  }
}


// ============================================================
// MOVE DIVER
// ============================================================

function moveDiver(
  diver,
  score,
  player
) {

  const percent =
    Math.min(
      100,
      score /
      TARGET *
      100
    );


  // 0 score:
  // player at edge
  //
  // full score:
  // player reaches center
  //
  // 41.5 keeps diver next to Rene,
  // not directly over him.

  const travel =
    percent *
    0.415;


  if (
    player === 1
  ) {

    diver.style.left =
      travel + "%";
  }


  else {

    diver.style.right =
      travel + "%";
  }
}


// ============================================================
// UPDATE DISPLAY
// ============================================================

function updateDisplay(
  data
) {

  const p1 =
    Number(
      data.p1
    ) || 0;


  const p2 =
    Number(
      data.p2
    ) || 0;


  const remaining1 =
    Math.max(
      0,
      TARGET - p1
    );


  const remaining2 =
    Math.max(
      0,
      TARGET - p2
    );


  const percent1 =
    Math.min(
      100,
      p1 /
      TARGET *
      100
    );


  const percent2 =
    Math.min(
      100,
      p2 /
      TARGET *
      100
    );


  // ==========================================================
  // MOVE DIVERS
  // ==========================================================

  moveDiver(
    diver1,
    p1,
    1
  );


  moveDiver(
    diver2,
    p2,
    2
  );


  // ==========================================================
  // DISTANCE
  // ==========================================================

  distance1.textContent =
    remaining1 +
    "M TO RENE";


  distance2.textContent =
    remaining2 +
    "M TO RENE";


  bottomDistance1.textContent =
    remaining1 +
    "M AWAY";


  bottomDistance2.textContent =
    remaining2 +
    "M AWAY";


  // ==========================================================
  // PROGRESS BAR
  // ==========================================================

  bar1.style.width =
    percent1 +
    "%";


  bar2.style.width =
    percent2 +
    "%";


  // ==========================================================
  // PLAYER PRESSED
  // ==========================================================

  if (
    p1 >
    previousP1
  ) {

    animateSwim(
      diver1,
      1
    );
  }


  if (
    p2 >
    previousP2
  ) {

    animateSwim(
      diver2,
      2
    );
  }


  previousP1 =
    p1;


  previousP2 =
    p2;


  // ==========================================================
  // DEFAULT WINNER HIDDEN
  // ==========================================================

  winnerBanner.style.display =
    "none";


  winnerBanner
    .classList
    .remove(
      "p1win",
      "p2win"
    );


  // ==========================================================
  // IDLE
  // ==========================================================

  if (
    data.state ===
    "IDLE"
  ) {

    countdown.textContent =
      "READY";


    p1Top.textContent =
      "PLAYER 1 // READY";


    p2Top.textContent =
      "PLAYER 2 // READY";


    connection.textContent =
      "CONNECTED TO ESP32 // WAITING FOR MISSION";
  }


  // ==========================================================
  // COUNTDOWN
  // ==========================================================

  else if (
    data.state ===
    "COUNTDOWN"
  ) {

    countdown.textContent =
      data.countdown;


    p1Top.textContent =
      "PLAYER 1 // GET READY";


    p2Top.textContent =
      "PLAYER 2 // GET READY";


    connection.textContent =
      "RESCUE MISSION STARTING";
  }


  // ==========================================================
  // ACTIVE
  // ==========================================================

  else if (
    data.state ===
    "RACING"
  ) {

    countdown.textContent =
      "RESCUE!";


    p1Top.textContent =
      "PLAYER 1 // SWIM!";


    p2Top.textContent =
      "PLAYER 2 // SWIM!";


    connection.textContent =
      "MISSION ACTIVE // PRESS FASTER";
  }


  // ==========================================================
  // FINISHED
  // ==========================================================

  else {

    countdown.textContent =
      "SAFE!";


    winnerBanner.style.display =
      "block";


    winnerMain.textContent =
      "PLAYER " +
      data.winner +
      " RESCUED RENE!";


    if (
      data.winner === 1
    ) {

      winnerBanner
        .classList
        .add(
          "p1win"
        );


      p1Top.textContent =
        "PLAYER 1 // RESCUER";


      p2Top.textContent =
        "PLAYER 2 // MISSION COMPLETE";
    }


    else {

      winnerBanner
        .classList
        .add(
          "p2win"
        );


      p1Top.textContent =
        "PLAYER 1 // MISSION COMPLETE";


      p2Top.textContent =
        "PLAYER 2 // RESCUER";
    }


    connection.textContent =
      "MISSION SUCCESS // RENE IS SAFE";
  }
}


// ============================================================
// GET ESP32 STATE
// ============================================================

async function updateGame() {

  try {

    const response =
      await fetch(
        "/state?t=" +
        Date.now(),
        {

          cache:
            "no-store"
        }
      );


    const data =
      await response.json();


    updateDisplay(
      data
    );
  }


  catch (
    error
  ) {

    connection.textContent =
      "ESP32 CONNECTION LOST";
  }
}


// ============================================================
// START
// ============================================================

document
  .getElementById(
    "startButton"
  )
  .addEventListener(
    "click",
    async function() {

      await fetch(
        "/start",
        {

          method:
            "POST"
        }
      );


      updateGame();
    }
  );


// ============================================================
// RESET
// ============================================================

document
  .getElementById(
    "resetButton"
  )
  .addEventListener(
    "click",
    async function() {

      await fetch(
        "/reset",
        {

          method:
            "POST"
        }
      );


      previousP1 =
        0;


      previousP2 =
        0;


      updateGame();
    }
  );


// ============================================================
// LIVE REFRESH
// ============================================================

setInterval(
  updateGame,
  55
);


updateGame();

</script>


</body>

</html>

)HTML";


// ============================================================
// ROOT PAGE
// ============================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    PAGE
  );
}


// ============================================================
// START ENDPOINT
// ============================================================

void handleStart() {

  startRace();


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


// ============================================================
// RESET ENDPOINT
// ============================================================

void handleReset() {

  resetGame();


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


// ============================================================
// STATE ENDPOINT
// ============================================================

void handleState() {

  String stateName;


  switch (
    gameState
  ) {

    case GAME_IDLE:

      stateName =
        "IDLE";

      break;


    case GAME_COUNTDOWN:

      stateName =
        "COUNTDOWN";

      break;


    case GAME_RACING:

      stateName =
        "RACING";

      break;


    default:

      stateName =
        "FINISHED";

      break;
  }


  String json;


  json.reserve(
    180
  );


  json +=
    "{";


  json +=
    "\"state\":\"";


  json +=
    stateName;


  json +=
    "\",";


  json +=
    "\"p1\":";


  json +=
    String(
      player1Score
    );


  json +=
    ",";


  json +=
    "\"p2\":";


  json +=
    String(
      player2Score
    );


  json +=
    ",";


  json +=
    "\"winner\":";


  json +=
    String(
      winner
    );


  json +=
    ",";


  json +=
    "\"countdown\":";


  json +=
    String(
      getCountdownNumber()
    );


  json +=
    "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(
    115200
  );


  delay(
    300
  );


  Serial.println();

  Serial.println(
    "============================"
  );

  Serial.println(
    "MX CLUB // RESCUE RENE"
  );

  Serial.println(
    "============================"
  );


  // ==========================================================
  // BUTTONS
  // ==========================================================

  pinMode(
    PLAYER1_PIN,
    INPUT_PULLUP
  );


  pinMode(
    PLAYER2_PIN,
    INPUT_PULLUP
  );


  lastP1 =
    digitalRead(
      PLAYER1_PIN
    );


  lastP2 =
    digitalRead(
      PLAYER2_PIN
    );


  Serial.println(
    "PLAYER 1 = GPIO25"
  );


  Serial.println(
    "PLAYER 2 = GPIO26"
  );


  // ==========================================================
  // WIFI
  // ==========================================================

  WiFi.mode(
    WIFI_AP
  );


  bool started =
    WiFi.softAP(
      WIFI_NAME,
      WIFI_PASSWORD
    );


  if (
    started
  ) {

    Serial.println(
      "WIFI STARTED"
    );
  }


  else {

    Serial.println(
      "WIFI FAILED"
    );
  }


  IPAddress IP =
    WiFi.softAPIP();


  Serial.println();


  Serial.print(
    "WiFi: "
  );


  Serial.println(
    WIFI_NAME
  );


  Serial.print(
    "Password: "
  );


  Serial.println(
    WIFI_PASSWORD
  );


  Serial.print(
    "Open: http://"
  );


  Serial.println(
    IP
  );


  // ==========================================================
  // SERVER
  // ==========================================================

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  server.on(
    "/state",
    HTTP_GET,
    handleState
  );


  server.on(
    "/start",
    HTTP_POST,
    handleStart
  );


  server.on(
    "/reset",
    HTTP_POST,
    handleReset
  );


  server.begin();


  Serial.println();

  Serial.println(
    "RESCUE SERVER READY"
  );
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  server.handleClient();


  updateCountdown();


  updateButtons();


  delay(
    1
  );
}
