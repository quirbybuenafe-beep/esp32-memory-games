#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// MX CLUB - RESCUE RENE
//
// EXACT GENERATED PIXEL SPRITES
//
// P1 BUTTON:
// GPIO25 -> BUTTON -> GND
//
// P2 BUTTON:
// GPIO26 -> BUTTON -> GND
//
// WiFi:
// MX-CLUB-RESCUE
//
// Password:
// mxclub123
//
// Browser:
// http://192.168.4.1
// ============================================================

const char* WIFI_NAME = "MX-CLUB-RESCUE";
const char* WIFI_PASSWORD = "REDACTED_CONFIG_VALUE";

WebServer server(80);

#define PLAYER1_PIN 25
#define PLAYER2_PIN 26

#define TARGET_DISTANCE 60

const unsigned long DEBOUNCE_MS = 18;

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

unsigned long lastP1Press = 0;
unsigned long lastP2Press = 0;

bool lastP1 = HIGH;
bool lastP2 = HIGH;


// ============================================================
// PLAYER 1 EXACT ANIMATED PIXEL SPRITE
// ============================================================

const char P1_ASSET[] PROGMEM =
  "data:image/webp;base64,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";


// ============================================================
// PLAYER 2 EXACT ANIMATED PIXEL SPRITE
// ============================================================

const char P2_ASSET[] PROGMEM =
  "data:image/webp;base64,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";


// ============================================================
// RENE DROWNING ANIMATION
// ============================================================

const char RENE_DROWN_ASSET[] PROGMEM =
  "data:image/webp;base64,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";


// ============================================================
// RENE RESCUED SPRITE
// ============================================================

const char RENE_SAFE_ASSET[] PROGMEM =
  "data:image/webp;base64,UklGRqwCAABXRUJQVlA4TKACAAAvO8AOEL/AqLatVA8d04AcdKEiEQjDYub2XWsoaiSFOYoXn"
  "PDFMxIQgoLGKratVLhk4JsoHpUyRLDnVkBn/gMAAjDgH2WOMcchiVce27gqwiReDrCjDYzU2/KZWcLL7P7H/o+axO+Grovqy"
  "IZmkG3bVtBW13AQQ20ElMr//6rEaPt039aO6P8EtP/qL7jEaIxefyTIg/k/gswF9+sCQp3GL6CeCHzquHGtOrwO4zXDHS8Y2"
  "72DpMvqC9Z7MK6DF3BP0J8ybqBe8wiAng3Mt1++1sgKc6zB23iTJJ2bFbsS1cZ2szn5U89Z6iBV1272zLGdDLOkoAizRepwl"
  "1tn+VOR6m2O8CuGOUq9VQuSapRgXhWru0sRTryNR0G1xsYd48oc4U3suoec6HEL5uYoHaDOneAM3t1sbj6eW8sa4Wk47oJ/d"
  "daXvt/o/cPWGGy4aqf3mOtXZ13m+Iw8IfIcXFhnzF21sXe8kV5ACy/kDA+GhjBLug62g5eMUF98A3VwEsfw+pKcNOqqo4UkB"
  "VdfVwZXwBg/m9SzuhxzMLl6ylXZkBaI1Tr7Dyd2Oo+pu6bgz317c4Qydf3Y2HZOwE5DftJSYJFndAsn8BvjmiRlpcOH274d4"
  "pJKTpJncwvf53zoFNlncLCUpSSpyBis6yTkLZR0B51ulhJukOEBA+lfKPmgzGc7HHwhKQuek6SlZCVJmThJSt2C3xo7tP2jt"
  "ZCBZZJEypKScmE56FDWC95bKECejiRJ+QjPQ8lAsHbhtrM1xqACfCT1APMN2NnO7R08ZeWCNHVWcNfQA2/46WeScpLym+ekn"
  "yl4c+3GnXb0g340rWWlQNYKP9I+uNbfuYVRG4TTwOjV2Xba1XsHfN/Av29scGh37tCu3qHd/beAHa75Lw8=";


// ============================================================
// ANIMATED FISH
// ============================================================

const char FISH_R_ASSET[] PROGMEM =
  "data:image/webp;base64,UklGRkwDAABXRUJQVlA4WAoAAAASAAAAIwAAGwAAQU5JTQYAAAD/////AABBTk1G4AAAAAYAA"
  "AEAAAsAABYAAKAAAAJWUDhMyAAAAC8LgAUQf6AokqTm7siZX/7jFl/oQAYIURPJVvOTg199sgF6agwgPCpBAjOKIklq7hUV3"
  "ME3WkcDArCUq+Y/AMD76ywJhEvYl9u3uij1ioIsqn+jM5qr06Yhzk4+A6fZtq3Kk44fJDps4LCAruGWNdLc2rlIJAY4C+BVF"
  "2AIGmQfIaL/E4CpAtxW31z1l39eVx90MFV+3iYPQLbf1jj46+0MRt7b6wnG0Pe8V7szuyNQ6XZ8D3u1mfDNB/HkdrmoaWTb/"
  "SNsIxEAQU5NRhwBAAAEAAABAAASAAAWAACgAAADVlA4TAQBAAAvEoAFEH+gKJKk5u7ImV/+4xZf6EAGCFETyVbzk4NffbIBe"
  "moMIDwqQQIziiJJau4VFdzBN1pHAwKwlKvmPwDA++ssCYRL2Jfbt7oo9YqCLKp/ozOaq9OmIc5OPgO41bYtax6s8q/C3Tt3d"
  "+gZwXqH1j9Ki+ArREqXBcIAGSBlFnAdIaL/E4Dfy/c/kaef9r94/MXJxU//m0oV2N/VWi3TA3B7D02RHlt5iFV4mtfjRZ6DG"
  "DXNyOJ1Z+bFxr5DslDvFqzYWHJEFmT6RXxEqHq5YMc6FfpyrJos2LG2qfp8vFh4eLE0VzNcLPws2JzQDOnCzwOALqXuc3wNJ"
  "lM8fGtz6rgAAEFOTUYUAQAABAAAAQAAEQAAFgAAoAAAAFZQOEz7AAAALxGABRB/oCiSpObuyJlf5o1M7CAOBxhRE8lW85OJ/"
  "4kGYkdNhX8FGPgSmFETSYpE9CpAABkKiPDv6Lurmv8AAADw/zarg3iEfnl/+4jSThRkUd2llqfzgnTzbgEU29qWVQ+WdAba+"
  "BLS3d11AD6Bn+oSaf8fSfAlXDMkEpEhkP5Ftut3ChH9nwD89yz7ib+/f9K8/4eH8pe/8kpgmkpL8fhWLK7BK6dLFKl/JUMsi"
  "xrt+aSI0cP10mf2sFE7Gep1FYmN6siIqLh17AcLt45xutuQ6Abr2MQ8brD9+ilWmDIXlt3JYkcEST8nBoAOU1zLABi6zFKE7"
  "wunCAAA";


const char FISH_L_ASSET[] PROGMEM =
  "data:image/webp;base64,UklGRoQDAABXRUJQVlA4WAoAAAASAAAAIwAAGwAAQU5JTQYAAAD/////AABBTk1G7gAAAAUAA"
  "AEAAA4AABYAAKAAAAJWUDhM1gAAAC8OgAUQh6C0tp0mzUblxLIfa7EQvs2A7qyhKLLVhuICDUjFWgT05BQDnFJHbSSpDV0Q+"
  "+7pBZkqlGoU2fkPAAAA55+COpFPJzJXcVwd0n7b5muszRvJcShhSd59C3AYSZLTrM79sSdY5Z8uJUwGEf2fADwo1SWZdE11r"
  "aS69LYKdcWkki5IgnCxVHgwI+JCJcnxP8HmzAFkAojpbmMAWwC+my2aDTYGvNt/RjMuMhDdHzstGzq8t2XdZr03HcjvkDTb6"
  "2gHUHtvw0kgAgAyOTPExXSPOAFBTk1GXAEAAAIAAAEAABkAABYAAKAAAANWUDhMQwEAAC8ZgAUQd6A0stWIy6Lw9EdZtHXEx"
  "UdnaERVbZuSoYUMOoolhQ6+FPDFG5aaRlKg608ABgg+cYgXQvXOfwAAAP6fXI3I0xLWKnIdCPva8nrTrlNFSXOAY23b8egNu"
  "ywgrW2n0phhN/YO/lRj2/imtG3bRjUnrGYbv86ZJUT0n0DSxjX4W8A/KqmqquLSOVNSUoJ0P5FIJJfKGxo5Qgs2qvfnnsXJw"
  "wJIZRNawm2Lq1ksMAcBvE1Y29+mA+CY0IJh42FBOit1KQW8U3JBY6dgMOcEpXib2mvEje2azaelDsI4P4GTJQVzpAkKAcXQh"
  "EJD2JTPoRTvME7Q2y7dvF8+MpWkTi8uduAaC3+OjAAjy4uoXRCtDJG8O2ixdrOQC5EgYWtiMXka5AJlSssVGvu9ZwN1Y2/k5"
  "TnCA1pGT0oKm66KegQAAEFOTUb+AAAABAAAAQAAEgAAFgAAoAAAAFZQOEzmAAAALxKABRCHoLS2nSbNRuXEsh9rsRC+zYDur"
  "KEostWG4gINSMVaBPTkFAOcUkdtJKkNXRD77ukFmSqUahTZ+Q8AAADnn4I6kU8nMldxXB3Sftvma6zNG8lxKGFJ3n0LBoeRJ"
  "DXOCe7QAe/yT9eWFMFsRP8nwH7e9nd7gb+gvdOO/X9cvnCgKh4E6nN08skGY0q4QXgY1JQ6RdONmtON+B5SapRltejII6W2F"
  "OUomuty0ZP6KUoVHdIVfG6km6To0CnXbSBvVZ3ROU1vUytG8+wDyndzo62bG0QOkOYJ54RTc57DnjY/1ys=";


// ============================================================
// MX CLUB LOGO
// ============================================================

const char LOGO_ASSET[] PROGMEM =
  "data:image/webp;base64,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"
  "m/bHgpzgflQAUCudKzibXbP6q6Nye4jNr5ZUxLRKtFIDqhvxOYaa0s8L+h9q6warB/+/edTJZvaY+vw9fKiT5+eCf7/JZd89"
  "zKRUIVLlhGjjDKxMGvSOTE3tAee/+cn+/TeXrJYBFWjP7beWc5CnarOzZNxHnMbO7FSVRpKMu/405t5eS1bXXVozzI7UF2e6"
  "5wTXQKjqvXPzNw/rOma3qut9XWcl5sdhgMY4TjFtO5mOqmZ+MGX3uQpwJ7ym4qslvWOP/3oOaTsFZ0tx1iFOgOdNSZgSipDN"
  "cPq9x8Nt96q3E+39ZPnzS0nR6LnO11RxMzfU96t35EPr7A+uDc/q/d62TknShYCBB5N+kjXl/zXh/e2e9o5+bV5A1hmCq2xq"
  "iqnC7f2uec6g2ZkWrUiSnK8EnWrverdR1DHZ43eP7b3ZZ53T23eH5rntWlEOW7j9akPH9f//4/nrcNj0/OF2/3dLqNmD0MAM"
  "B+sdKUw5/bpf/3TzldPfrWkZViFJWpTncltpxfuxvcy5Xl/rKEThFDt/zoaXYYswFbE5Oja8qluD57bSlU1U2tl9Owm0IJRL"
  "r5VoZ9PVmXSvs7fFUzMaC6vNdTL62v3e8Ptafz53tdtoZjW2OVBVZOlSbuud3a02kztdXnijq3Hc/zptyu7gIJF7f8dgp492"
  "9nYIrPgOhdkhprWXRcIxJrTcwQWN19a+L6Uxmb33D/AefXuLdGCwpJgj6kXtm3Wfz81t9vSzuiycBMoUgf1orff+ePUdVZeu"
  "W2yqJr2tDu+cvWy67xjbqu7wjMBcco6nuCu7T72T+kVn0utRlqZ2ZOuAm1rrjZ0G3TM3qemRVFFP/KP6VU7z4m2MSZllZjJE"
  "uWqKYLZl5eq26st8ypdRmkZZPTJj986NB7VnVWVxexvZeKsK3dPtKUwWMYFBNb68r8S6KRUwXGtef89hTvjtlCrOANuLa0QI"
  "Q2pRrFZgElDzz09c9GMicGbbvPOYvI1jm0gVanr4UiWrHE1Zkr5Zp8bsNFlJK+ajtzLl6Jt1zPCcwqrM0A3wKvQfOfm/Zqa4"
  "+xv/6p0sgwyMfxWPQVoBTLQWwsqduc+V6NhUbklyofxSNSePb/voOjc+HoY50mjEbxBy9rPbfDlLl49iajG9+Shvt5+pagi9"
  "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";


// ============================================================
// GAME FUNCTIONS
// ============================================================

void resetGame() {

  player1Score = 0;

  player2Score = 0;

  winner = 0;

  gameState = GAME_IDLE;


  lastP1 =
    digitalRead(
      PLAYER1_PIN
    );


  lastP2 =
    digitalRead(
      PLAYER2_PIN
    );
}


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
}


void playerPressed(
  int player
) {

  if (
    gameState !=
    GAME_RACING
  ) {

    return;
  }


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
      "P1 = "
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
    }
  }


  else {

    if (
      player2Score <
      TARGET_DISTANCE
    ) {

      player2Score++;
    }


    Serial.print(
      "P2 = "
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
    }
  }
}


void updateCountdown() {

  if (
    gameState !=
    GAME_COUNTDOWN
  ) {

    return;
  }


  if (
    millis() -
    countdownStarted >=
    3000
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
  }
}


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
// WEB PAGE
//
// IMPORTANT:
// Plain escaped C++ strings.
// Arduino IDE 1.8.19 safe.
// ============================================================

const char PAGE[] PROGMEM =

"<!doctype html>\n"

"<html>\n"

"<head>\n"

"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"

"<title>MX Club - Rescue Rene</title>\n"

"<style>\n"

"*{box-sizing:border-box}\n"

":root{"
"--ink:#eefbff;"
"--deep:#020b13;"
"--panel:#061b2b;"
"--p1:#20dcff;"
"--p2:#ffb238;"
"--rene:#ffe45d;"
"--safe:#59f29a"
"}\n"

"html,body{"
"margin:0;"
"min-height:100%;"
"background:#010712;"
"color:var(--ink);"
"font-family:\"Courier New\",monospace"
"}\n"

"body{padding:8px}\n"

".game{"
"width:min(1500px,100%);"
"margin:auto;"
"border:4px solid var(--ink);"
"background:var(--deep)"
"}\n"

".header{"
"display:grid;"
"grid-template-columns:96px 1fr 120px;"
"gap:14px;"
"align-items:center;"
"padding:12px 16px;"
"background:#061522;"
"border-bottom:4px solid var(--ink)"
"}\n"

".logo{"
"width:80px;"
"height:80px;"
"object-fit:contain;"
"background:#fff;"
"border:3px solid #9d7bc7;"
"image-rendering:pixelated"
"}\n"

".head{text-align:center;min-width:0}\n"

".club{"
"font-size:13px;"
"font-weight:900;"
"letter-spacing:5px;"
"color:#ceb8ee"
"}\n"

"h1{"
"margin:3px 0 0;"
"font-size:clamp(38px,6vw,76px);"
"line-height:.95;"
"text-shadow:5px 5px #07506f"
"}\n"

".sub{"
"margin-top:8px;"
"font-size:11px;"
"letter-spacing:2px;"
"opacity:.75"
"}\n"

".badge{"
"justify-self:end;"
"padding:9px;"
"border:3px solid var(--safe);"
"color:var(--safe);"
"font-size:11px;"
"font-weight:900;"
"text-align:center"
"}\n"

".status{"
"display:grid;"
"grid-template-columns:1fr auto 1fr;"
"gap:12px;"
"align-items:center;"
"padding:9px 16px;"
"background:var(--panel);"
"border-bottom:4px solid var(--ink);"
"font-size:13px;"
"font-weight:900"
"}\n"

".status .right{text-align:right}\n"

"#countdown{"
"min-width:220px;"
"padding:7px 14px;"
"text-align:center;"
"border:3px solid #7ee9ff;"
"background:#020b13;"
"color:#7ee9ff;"
"font-size:clamp(28px,4vw,48px)"
"}\n"

".ocean{"
"position:relative;"
"overflow:hidden;"
"border-bottom:4px solid var(--ink);"
"background:linear-gradient("
"#179bc8 0 18%,"
"#0e7ead 31%,"
"#075f88 55%,"
"#043f62 78%,"
"#022b45 100%"
")"
"}\n"

".surface{"
"height:34px;"
"width:110%;"
"margin-left:-60px;"
"background:repeating-linear-gradient("
"90deg,"
"#76e5ff 0 28px,"
"#3bc7ed 28px 56px,"
"#1487af 56px 84px"
");"
"clip-path:polygon("
"0 50%,"
"5% 12%,"
"10% 54%,"
"15% 10%,"
"20% 54%,"
"25% 12%,"
"30% 54%,"
"35% 10%,"
"40% 54%,"
"45% 12%,"
"50% 54%,"
"55% 10%,"
"60% 54%,"
"65% 12%,"
"70% 54%,"
"75% 10%,"
"80% 54%,"
"85% 12%,"
"90% 54%,"
"95% 10%,"
"100% 50%,"
"100% 100%,"
"0 100%"
");"
"animation:wave 1.2s steps(6) infinite"
"}\n"

"@keyframes wave{"
"to{transform:translateX(84px)}"
"}\n"

".stage{"
"position:relative;"
"height:610px;"
"overflow:hidden"
"}\n"

".bubbles{"
"position:absolute;"
"inset:0;"
"opacity:.28;"
"pointer-events:none;"
"background-image:"
"radial-gradient(circle,#fff 0 2px,transparent 3px),"
"radial-gradient(circle,#fff 0 3px,transparent 4px);"
"background-size:110px 140px,170px 190px;"
"animation:bubbles 6s linear infinite"
"}\n"

"@keyframes bubbles{"
"to{background-position:0 -140px,0 -190px}"
"}\n"

".ray{"
"position:absolute;"
"top:-30px;"
"width:90px;"
"height:520px;"
"background:linear-gradient(rgba(255,255,255,.14),transparent);"
"transform:skewX(-10deg);"
"opacity:.25"
"}\n"

".r1{left:15%}\n"
".r2{left:47%;width:60px}\n"
".r3{left:78%}\n"

".center{"
"position:absolute;"
"left:50%;"
"top:35px;"
"bottom:30px;"
"width:5px;"
"transform:translateX(-50%);"
"background:repeating-linear-gradient("
"to bottom,"
"var(--rene) 0 16px,"
"#17374b 16px 32px"
");"
"opacity:.72"
"}\n"

".rescuePoint{"
"position:absolute;"
"left:50%;"
"top:28px;"
"transform:translateX(-50%);"
"z-index:4;"
"padding:5px 12px;"
"border:3px solid var(--rene);"
"background:#03101a;"
"color:var(--rene);"
"font-size:12px;"
"font-weight:900"
"}\n"

".reneWrap{"
"position:absolute;"
"left:50%;"
"top:228px;"
"width:170px;"
"transform:translateX(-50%);"
"z-index:10;"
"text-align:center;"
"animation:reneFloat 1.1s ease-in-out infinite"
"}\n"

"@keyframes reneFloat{"
"50%{"
"transform:translateX(-50%) translateY(7px)"
"}"
"}\n"

".reneImg{"
"width:140px;"
"height:140px;"
"object-fit:contain;"
"image-rendering:pixelated;"
"filter:drop-shadow(4px 5px 0 rgba(0,0,0,.25))"
"}\n"

"#reneSafe{display:none}\n"

".reneName{"
"display:inline-block;"
"margin-top:4px;"
"padding:5px 10px;"
"border:3px solid var(--rene);"
"background:#03101a;"
"color:var(--rene);"
"font-size:13px;"
"font-weight:900"
"}\n"

".lane{"
"position:absolute;"
"left:3%;"
"right:3%;"
"height:175px;"
"border-top:3px dashed rgba(255,255,255,.42);"
"border-bottom:3px dashed rgba(255,255,255,.17)"
"}\n"

".lane1{top:82px}\n"

".lane2{bottom:82px}\n"

".laneLabel{"
"position:absolute;"
"top:-34px;"
"padding:5px 9px;"
"border:2px solid currentColor;"
"background:#03101a;"
"font-size:12px;"
"font-weight:900"
"}\n"

".lane1 .laneLabel{"
"left:0;"
"color:var(--p1)"
"}\n"

".lane2 .laneLabel{"
"right:0;"
"color:var(--p2)"
"}\n"

".diver{"
"position:absolute;"
"top:25px;"
"width:220px;"
"height:140px;"
"z-index:8;"
"transition:left .055s linear,right .055s linear"
"}\n"

".diver.p1{left:0}\n"

".diver.p2{right:0}\n"

".diverImg{"
"width:190px;"
"height:127px;"
"object-fit:contain;"
"image-rendering:pixelated;"
"filter:drop-shadow(4px 5px 0 rgba(0,0,0,.28))"
"}\n"

".diver.burst .diverImg{"
"animation:burst .18s steps(2) 2"
"}\n"

"@keyframes burst{"
"50%{transform:scale(1.07) translateY(-4px)}"
"}\n"

".trail{"
"position:absolute;"
"top:47px;"
"left:-12px;"
"color:rgba(255,255,255,.78);"
"font-size:20px;"
"font-weight:900;"
"letter-spacing:-5px"
"}\n"

".p2 .trail{"
"left:auto;"
"right:18px"
"}\n"

".distanceTag{"
"position:absolute;"
"left:20px;"
"top:113px;"
"min-width:145px;"
"padding:6px 8px;"
"text-align:center;"
"border:3px solid var(--p1);"
"background:#03101a;"
"color:var(--p1);"
"font-size:12px;"
"font-weight:900"
"}\n"

".p2 .distanceTag{"
"border-color:var(--p2);"
"color:var(--p2)"
"}\n"

".fish{"
"position:absolute;"
"width:78px;"
"height:60px;"
"object-fit:contain;"
"image-rendering:pixelated;"
"opacity:.95;"
"z-index:2"
"}\n"

".fishR{"
"left:-85px;"
"top:300px;"
"animation:fishR 13s linear infinite"
"}\n"

".fishL{"
"right:-85px;"
"top:475px;"
"animation:fishL 17s linear infinite"
"}\n"

"@keyframes fishR{"
"to{left:105%}"
"}\n"

"@keyframes fishL{"
"to{right:105%}"
"}\n"

".seabed{"
"height:40px;"
"border-top:5px solid #d6a860;"
"background:repeating-linear-gradient("
"90deg,"
"#ad7b42 0 28px,"
"#8b5c31 28px 56px"
")"
"}\n"

".info{"
"display:grid;"
"grid-template-columns:1fr 1fr;"
"border-bottom:4px solid var(--ink)"
"}\n"

".card{"
"padding:16px 18px;"
"background:#061b2b"
"}\n"

".card:first-child{"
"border-right:4px solid var(--ink)"
"}\n"

".cardTop{"
"display:flex;"
"justify-content:space-between;"
"gap:12px;"
"align-items:center;"
"font-size:13px;"
"font-weight:900"
"}\n"

".p1txt{color:var(--p1)}\n"

".p2txt{color:var(--p2)}\n"

".meter{"
"height:27px;"
"margin-top:12px;"
"border:3px solid var(--ink);"
"background:#020b14"
"}\n"

".fill{"
"width:0;"
"height:100%;"
"transition:width .055s linear"
"}\n"

"#bar1{background:var(--p1)}\n"

"#bar2{background:var(--p2)}\n"

"#winner{"
"display:none;"
"position:absolute;"
"left:3%;"
"right:3%;"
"top:50%;"
"transform:translateY(-50%);"
"z-index:50;"
"padding:42px 20px;"
"text-align:center;"
"background:rgba(2,10,18,.97);"
"border:8px solid var(--safe);"
"box-shadow:10px 10px 0 rgba(0,0,0,.42)"
"}\n"

"#winner.p1win{"
"border-color:var(--p1)"
"}\n"

"#winner.p2win{"
"border-color:var(--p2)"
"}\n"

"#winner .eyebrow{"
"color:var(--safe);"
"font-size:15px;"
"font-weight:900;"
"letter-spacing:5px"
"}\n"

"#winner .main{"
"margin-top:10px;"
"font-size:clamp(40px,7vw,90px);"
"line-height:1;"
"font-weight:900;"
"text-shadow:5px 5px #073a54"
"}\n"

"#winner .sub{"
"margin-top:14px;"
"color:var(--safe);"
"font-size:clamp(13px,2vw,22px);"
"font-weight:900;"
"letter-spacing:2px"
"}\n"

".controls{"
"display:flex;"
"flex-wrap:wrap;"
"justify-content:center;"
"gap:12px;"
"padding:16px;"
"background:#04131f"
"}\n"

"button{"
"min-width:230px;"
"min-height:58px;"
"padding:15px 22px;"
"border:3px solid var(--ink);"
"background:#082238;"
"color:var(--ink);"
"font:900 17px \"Courier New\",monospace;"
"cursor:pointer"
"}\n"

"button:active{"
"background:var(--ink);"
"color:#03101a"
"}\n"

".footer{"
"padding:10px;"
"text-align:center;"
"font-size:11px;"
"letter-spacing:1px;"
"opacity:.65"
"}\n"

"@media(max-width:760px){"

"body{padding:4px}"

".header{"
"grid-template-columns:72px 1fr;"
"padding:10px"
"}"

".logo{"
"width:64px;"
"height:64px"
"}"

".badge{display:none}"

".club{"
"font-size:9px;"
"letter-spacing:2px"
"}"

".sub{"
"font-size:9px;"
"letter-spacing:1px"
"}"

".status{"
"grid-template-columns:1fr;"
"text-align:center"
"}"

".status .right{text-align:center}"

"#countdown{min-width:0}"

".stage{height:560px}"

".lane{left:2%;right:2%}"

".diver{width:165px}"

".diverImg{"
"width:145px;"
"height:97px"
"}"

".distanceTag{"
"top:94px;"
"min-width:120px;"
"font-size:10px"
"}"

".reneImg{"
"width:105px;"
"height:105px"
"}"

".info{grid-template-columns:1fr}"

".card:first-child{"
"border-right:0;"
"border-bottom:4px solid var(--ink)"
"}"

"button{width:100%}"

"#winner{"
"left:2%;"
"right:2%;"
"padding:28px 10px"
"}"

"}\n"

"</style>\n"

"</head>\n"

"<body>\n"

"<div class=\"game\">\n"


// HEADER

"<div class=\"header\">\n"

"<img "
"id=\"logo\" "
"class=\"logo\" "
"alt=\"MX Club logo\">"

"<div class=\"head\">\n"

"<div class=\"club\">"
"MECHATRONIX CLUB"
"</div>\n"

"<h1>"
"RESCUE RENE"
"</h1>\n"

"<div class=\"sub\">"
"BATEBRONIA // OCEAN RESCUE RACE"
"</div>\n"

"</div>\n"

"<div class=\"badge\">"
"2 PLAYER<br>"
"RESCUE GAME"
"</div>\n"

"</div>\n"


// STATUS

"<div class=\"status\">\n"

"<div id=\"p1top\">"
"PLAYER 1 // READY"
"</div>\n"

"<div id=\"countdown\">"
"READY"
"</div>\n"

"<div "
"class=\"right\" "
"id=\"p2top\">"
"PLAYER 2 // READY"
"</div>\n"

"</div>\n"


// OCEAN

"<div class=\"ocean\">\n"

"<div class=\"surface\"></div>\n"

"<div class=\"stage\">\n"

"<div class=\"bubbles\"></div>\n"

"<div class=\"ray r1\"></div>\n"

"<div class=\"ray r2\"></div>\n"

"<div class=\"ray r3\"></div>\n"

"<div class=\"center\"></div>\n"

"<div class=\"rescuePoint\">"
"RESCUE POINT"
"</div>\n"


// RENE

"<div class=\"reneWrap\">\n"

"<img "
"id=\"reneDrown\" "
"class=\"reneImg\" "
"alt=\"Rene in the water\">"

"<img "
"id=\"reneSafe\" "
"class=\"reneImg\" "
"alt=\"Rene rescued\">"

"<div class=\"reneName\">"
"RENE"
"</div>\n"

"</div>\n"


// PLAYER 1

"<div class=\"lane lane1\">\n"

"<div class=\"laneLabel\">"
"PLAYER 1 RESCUE DIVER"
"</div>\n"

"<div "
"class=\"diver p1\" "
"id=\"diver1\">"

"<div class=\"trail\">"
"&lt;&lt;&lt;"
"</div>"

"<img "
"id=\"p1img\" "
"class=\"diverImg\" "
"alt=\"Player 1 rescue swimmer\">"

"<div "
"class=\"distanceTag\" "
"id=\"dist1\">"
"60M TO RENE"
"</div>"

"</div>\n"

"</div>\n"


// PLAYER 2

"<div class=\"lane lane2\">\n"

"<div class=\"laneLabel\">"
"PLAYER 2 RESCUE DIVER"
"</div>\n"

"<div "
"class=\"diver p2\" "
"id=\"diver2\">"

"<div class=\"trail\">"
"&gt;&gt;&gt;"
"</div>"

"<img "
"id=\"p2img\" "
"class=\"diverImg\" "
"alt=\"Player 2 rescue swimmer\">"

"<div "
"class=\"distanceTag\" "
"id=\"dist2\">"
"60M TO RENE"
"</div>"

"</div>\n"

"</div>\n"


// FISH

"<img "
"id=\"fishR\" "
"class=\"fish fishR\" "
"alt=\"\">"

"<img "
"id=\"fishL\" "
"class=\"fish fishL\" "
"alt=\"\">"


// WINNER BAR

"<div id=\"winner\">\n"

"<div class=\"eyebrow\">"
"MISSION SUCCESS"
"</div>"

"<div "
"class=\"main\" "
"id=\"winnerMain\">"
"PLAYER 1 RESCUED RENE!"
"</div>"

"<div class=\"sub\">"
"RENE IS SAFE // RESCUE COMPLETE"
"</div>"

"</div>\n"


"</div>\n"

"<div class=\"seabed\"></div>\n"

"</div>\n"


// BOTTOM INFO

"<div class=\"info\">\n"


"<div class=\"card\">\n"

"<div class=\"cardTop\">\n"

"<span class=\"p1txt\">"
"PLAYER 1 RESCUE DIVER"
"</span>"

"<span id=\"bottom1\">"
"60M AWAY"
"</span>"

"</div>\n"

"<div class=\"meter\">"

"<div "
"class=\"fill\" "
"id=\"bar1\">"
"</div>"

"</div>\n"

"</div>\n"


"<div class=\"card\">\n"

"<div class=\"cardTop\">\n"

"<span class=\"p2txt\">"
"PLAYER 2 RESCUE DIVER"
"</span>"

"<span id=\"bottom2\">"
"60M AWAY"
"</span>"

"</div>\n"

"<div class=\"meter\">"

"<div "
"class=\"fill\" "
"id=\"bar2\">"
"</div>"

"</div>\n"

"</div>\n"


"</div>\n"


// CONTROLS

"<div class=\"controls\">\n"

"<button "
"type=\"button\" "
"id=\"startBtn\">"
"START RESCUE"
"</button>"

"<button "
"type=\"button\" "
"id=\"resetBtn\">"
"RESET MISSION"
"</button>"

"</div>\n"


"<div "
"class=\"footer\" "
"id=\"connection\">"
"CONNECTED TO ESP32 // WAITING FOR MISSION"
"</div>\n"


"</div>\n"


// ============================================================
// JAVASCRIPT
// ============================================================

"<script>\n"

"const TARGET=60;\n"

"const $=id=>"
"document.getElementById(id);\n"


"const diver1=$(\"diver1\");\n"

"const diver2=$(\"diver2\");\n"

"const dist1=$(\"dist1\");\n"

"const dist2=$(\"dist2\");\n"

"const bottom1=$(\"bottom1\");\n"

"const bottom2=$(\"bottom2\");\n"

"const bar1=$(\"bar1\");\n"

"const bar2=$(\"bar2\");\n"

"const countdown=$(\"countdown\");\n"

"const p1top=$(\"p1top\");\n"

"const p2top=$(\"p2top\");\n"

"const winnerBox=$(\"winner\");\n"

"const winnerMain=$(\"winnerMain\");\n"

"const connection=$(\"connection\");\n"

"const reneDrown=$(\"reneDrown\");\n"

"const reneSafe=$(\"reneSafe\");\n"


"let old1=0;\n"

"let old2=0;\n"

"let timer1=null;\n"

"let timer2=null;\n"


// ============================================================
// LOAD EMBEDDED EXACT SPRITES
// ============================================================

"async function loadAsset(id,url){\n"

"const response="
"await fetch("
"url,"
"{cache:\"force-cache\"}"
");\n"

"const src="
"await response.text();\n"

"$(id).src=src;\n"

"}\n"


"Promise.all([\n"

"loadAsset("
"\"logo\","
"\"/asset-logo\""
"),\n"

"loadAsset("
"\"p1img\","
"\"/asset-p1\""
"),\n"

"loadAsset("
"\"p2img\","
"\"/asset-p2\""
"),\n"

"loadAsset("
"\"reneDrown\","
"\"/asset-rene\""
"),\n"

"loadAsset("
"\"reneSafe\","
"\"/asset-safe\""
"),\n"

"loadAsset("
"\"fishR\","
"\"/asset-fish-r\""
"),\n"

"loadAsset("
"\"fishL\","
"\"/asset-fish-l\""
")\n"

"]).catch(()=>{"
"connection.textContent="
"\"ASSET LOAD ERROR\";"
"});\n"


// ============================================================
// RENE
//
// Drowning animation is never restarted.
// We simply hide/show the two images.
// ============================================================

"function showReneSafe(safe){\n"

"reneDrown.style.display="
"safe?"
"\"none\":"
"\"block\";\n"

"reneSafe.style.display="
"safe?"
"\"block\":"
"\"none\";\n"

"}\n"


// ============================================================
// BUTTON BURST
// ============================================================

"function burst(diver,player){\n"

"diver.classList.remove("
"\"burst\""
");\n"

"void diver.offsetWidth;\n"

"diver.classList.add("
"\"burst\""
");\n"


"if(player===1){\n"

"clearTimeout(timer1);\n"

"timer1="
"setTimeout("
"function(){"
"diver.classList.remove("
"\"burst\""
");"
"},"
"360"
");\n"

"}else{\n"

"clearTimeout(timer2);\n"

"timer2="
"setTimeout("
"function(){"
"diver.classList.remove("
"\"burst\""
");"
"},"
"360"
");\n"

"}\n"

"}\n"


// ============================================================
// MOVE SWIMMERS
// ============================================================

"function moveDiver("
"diver,"
"score,"
"player"
"){\n"

"const pct="
"Math.min("
"100,"
"score/TARGET*100"
");\n"

"const travel="
"pct*0.385;\n"


"if(player===1){"

"diver.style.left="
"travel+\"%\";"

"}else{"

"diver.style.right="
"travel+\"%\";"

"}\n"

"}\n"


// ============================================================
// UPDATE UI
// ============================================================

"function render(data){\n"


"const p1="
"Number(data.p1)||0;\n"

"const p2="
"Number(data.p2)||0;\n"


"const rem1="
"Math.max("
"0,"
"TARGET-p1"
");\n"

"const rem2="
"Math.max("
"0,"
"TARGET-p2"
");\n"


"const pct1="
"Math.min("
"100,"
"p1/TARGET*100"
");\n"

"const pct2="
"Math.min("
"100,"
"p2/TARGET*100"
");\n"


"moveDiver("
"diver1,"
"p1,"
"1"
");\n"

"moveDiver("
"diver2,"
"p2,"
"2"
");\n"


"dist1.textContent="
"rem1+"
"\"M TO RENE\";\n"

"dist2.textContent="
"rem2+"
"\"M TO RENE\";\n"


"bottom1.textContent="
"rem1+"
"\"M AWAY\";\n"

"bottom2.textContent="
"rem2+"
"\"M AWAY\";\n"


"bar1.style.width="
"pct1+\"%\";\n"

"bar2.style.width="
"pct2+\"%\";\n"


"if(p1>old1){"

"burst("
"diver1,"
"1"
");"

"}\n"


"if(p2>old2){"

"burst("
"diver2,"
"2"
");"

"}\n"


"old1=p1;\n"

"old2=p2;\n"


"winnerBox.style.display="
"\"none\";\n"


"winnerBox.classList.remove("
"\"p1win\","
"\"p2win\""
");\n"


// IDLE

"if(data.state===\"IDLE\"){\n"

"showReneSafe(false);\n"

"countdown.textContent="
"\"READY\";\n"

"p1top.textContent="
"\"PLAYER 1 // READY\";\n"

"p2top.textContent="
"\"PLAYER 2 // READY\";\n"

"connection.textContent="
"\"CONNECTED TO ESP32 // WAITING FOR MISSION\";\n"

"}\n"


// COUNTDOWN

"else if(data.state===\"COUNTDOWN\"){\n"

"showReneSafe(false);\n"

"countdown.textContent="
"data.countdown;\n"

"p1top.textContent="
"\"PLAYER 1 // GET READY\";\n"

"p2top.textContent="
"\"PLAYER 2 // GET READY\";\n"

"connection.textContent="
"\"RESCUE MISSION STARTING\";\n"

"}\n"


// RACING

"else if(data.state===\"RACING\"){\n"

"showReneSafe(false);\n"

"countdown.textContent="
"\"RESCUE!\";\n"

"p1top.textContent="
"\"PLAYER 1 // SWIM!\";\n"

"p2top.textContent="
"\"PLAYER 2 // SWIM!\";\n"

"connection.textContent="
"\"MISSION ACTIVE // PRESS FASTER\";\n"

"}\n"


// FINISHED

"else{\n"

"showReneSafe(true);\n"

"countdown.textContent="
"\"SAFE!\";\n"

"winnerBox.style.display="
"\"block\";\n"


"winnerMain.textContent="
"\"PLAYER \"+"
"data.winner+"
"\" RESCUED RENE!\";\n"


"if(data.winner===1){\n"

"winnerBox.classList.add("
"\"p1win\""
");\n"

"p1top.textContent="
"\"PLAYER 1 // RESCUER\";\n"

"p2top.textContent="
"\"PLAYER 2 // FINISHED\";\n"

"}else{\n"

"winnerBox.classList.add("
"\"p2win\""
");\n"

"p1top.textContent="
"\"PLAYER 1 // FINISHED\";\n"

"p2top.textContent="
"\"PLAYER 2 // RESCUER\";\n"

"}\n"


"connection.textContent="
"\"MISSION SUCCESS // RENE IS SAFE\";\n"

"}\n"


"}\n"


// ============================================================
// ESP32 UPDATE
// ============================================================

"async function updateGame(){\n"

"try{\n"

"const response="
"await fetch("
"\"/state?t=\"+"
"Date.now(),"
"{cache:\"no-store\"}"
");\n"


"const data="
"await response.json();\n"


"render(data);\n"

"}catch(error){\n"

"connection.textContent="
"\"ESP32 CONNECTION LOST\";\n"

"}\n"

"}\n"


// START

"$(\"startBtn\").addEventListener("
"\"click\","
"async function(){\n"

"await fetch("
"\"/start\","
"{method:\"POST\"}"
");\n"

"old1=0;\n"

"old2=0;\n"

"updateGame();\n"

"}"
");\n"


// RESET

"$(\"resetBtn\").addEventListener("
"\"click\","
"async function(){\n"

"await fetch("
"\"/reset\","
"{method:\"POST\"}"
");\n"

"old1=0;\n"

"old2=0;\n"

"updateGame();\n"

"}"
");\n"


"setInterval("
"updateGame,"
"60"
");\n"


"updateGame();\n"


"</script>\n"

"</body>\n"

"</html>";


// ============================================================
// SEND ASSETS
// ============================================================

void sendAsset(
  const char* asset
) {

  server.send_P(
    200,
    "text/plain",
    asset
  );
}


// ============================================================
// HTTP
// ============================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    PAGE
  );
}


void handleStart() {

  startRace();


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


void handleReset() {

  resetGame();


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


void handleState() {

  const char* stateName =
    "FINISHED";


  if (
    gameState ==
    GAME_IDLE
  ) {

    stateName =
      "IDLE";
  }


  else if (
    gameState ==
    GAME_COUNTDOWN
  ) {

    stateName =
      "COUNTDOWN";
  }


  else if (
    gameState ==
    GAME_RACING
  ) {

    stateName =
      "RACING";
  }


  char json[180];


  snprintf(
    json,
    sizeof(json),

    "{\"state\":\"%s\","
    "\"p1\":%d,"
    "\"p2\":%d,"
    "\"winner\":%d,"
    "\"countdown\":%d}",

    stateName,
    player1Score,
    player2Score,
    winner,
    getCountdownNumber()
  );


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


  WiFi.mode(
    WIFI_AP
  );


  WiFi.softAP(
    WIFI_NAME,
    WIFI_PASSWORD
  );


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


  // Exact generated sprites.

  server.on(
    "/asset-logo",
    HTTP_GET,
    []() {

      sendAsset(
        LOGO_ASSET
      );
    }
  );


  server.on(
    "/asset-p1",
    HTTP_GET,
    []() {

      sendAsset(
        P1_ASSET
      );
    }
  );


  server.on(
    "/asset-p2",
    HTTP_GET,
    []() {

      sendAsset(
        P2_ASSET
      );
    }
  );


  server.on(
    "/asset-rene",
    HTTP_GET,
    []() {

      sendAsset(
        RENE_DROWN_ASSET
      );
    }
  );


  server.on(
    "/asset-safe",
    HTTP_GET,
    []() {

      sendAsset(
        RENE_SAFE_ASSET
      );
    }
  );


  server.on(
    "/asset-fish-r",
    HTTP_GET,
    []() {

      sendAsset(
        FISH_R_ASSET
      );
    }
  );


  server.on(
    "/asset-fish-l",
    HTTP_GET,
    []() {

      sendAsset(
        FISH_L_ASSET
      );
    }
  );


  server.begin();


  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "MX CLUB - RESCUE RENE"
  );

  Serial.println(
    "=========================="
  );


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
    WiFi.softAPIP()
  );


  Serial.println(
    "Player 1 = GPIO25"
  );


  Serial.println(
    "Player 2 = GPIO26"
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
