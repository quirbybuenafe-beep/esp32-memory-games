#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// MX CLUB - RESCUE RENE: OCEAN RESCUE + MAMA SOUND
//
// P1: GPIO25 -> button -> GND
// P2: GPIO26 -> button -> GND
//
// WiFi: MX-CLUB-RESCUE
// Pass: mxclub123
// Open: http://192.168.4.1
//
// First to 60 presses rescues Rene.
// Uploaded MAMA sound plays:
//   1) when START RESCUE MISSION is pressed
//   2) when Rene is rescued
// ============================================================

const char* WIFI_NAME = "MX-CLUB-RESCUE";
const char* WIFI_PASSWORD = "REDACTED_CONFIG_VALUE";

IPAddress localIP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

#define PLAYER1_PIN 25
#define PLAYER2_PIN 26
#define TARGET_DISTANCE 60

const unsigned long PRESS_DEBOUNCE_MS = 7;
const unsigned long RELEASE_DEBOUNCE_MS = 9;

enum GameState {
  GAME_IDLE,
  GAME_COUNTDOWN,
  GAME_RACING,
  GAME_FINISHED
};

GameState gameState = GAME_IDLE;

int player1Score = 0;
int player2Score = 0;
int winner = 0;

unsigned long countdownStarted = 0;

struct RaceButton {
  uint8_t pin;
  bool rawPressed;
  bool stablePressed;
  unsigned long changedAt;
};

RaceButton p1Button = {PLAYER1_PIN,false,false,0};
RaceButton p2Button = {PLAYER2_PIN,false,false,0};

void resetButton(RaceButton &b) {
  bool p = digitalRead(b.pin) == LOW;
  b.rawPressed = p;
  b.stablePressed = p;
  b.changedAt = millis();
}

void resetGame() {
  player1Score = 0;
  player2Score = 0;
  winner = 0;
  gameState = GAME_IDLE;
  resetButton(p1Button);
  resetButton(p2Button);
}

void startRace() {
  player1Score = 0;
  player2Score = 0;
  winner = 0;
  countdownStarted = millis();
  gameState = GAME_COUNTDOWN;
  resetButton(p1Button);
  resetButton(p2Button);
  Serial.println("3... 2... 1...");
}

void finishRace(int player) {
  if (gameState != GAME_RACING) return;
  winner = player;
  gameState = GAME_FINISHED;
  Serial.print("PLAYER ");
  Serial.print(player);
  Serial.println(" RESCUED RENE!");
}

void playerPressed(int player) {
  if (gameState != GAME_RACING) return;

  if (player == 1) {
    if (player1Score < TARGET_DISTANCE) player1Score++;
    if (player1Score >= TARGET_DISTANCE) finishRace(1);
  } else {
    if (player2Score < TARGET_DISTANCE) player2Score++;
    if (player2Score >= TARGET_DISTANCE) finishRace(2);
  }
}

void updateOneButton(RaceButton &b, int player) {
  unsigned long now = millis();
  bool p = digitalRead(b.pin) == LOW;

  if (p != b.rawPressed) {
    b.rawPressed = p;
    b.changedAt = now;
  }

  if (b.rawPressed != b.stablePressed) {
    unsigned long waitMs = b.rawPressed ? PRESS_DEBOUNCE_MS : RELEASE_DEBOUNCE_MS;

    if (now - b.changedAt >= waitMs) {
      b.stablePressed = b.rawPressed;
      if (b.stablePressed) playerPressed(player);
    }
  }
}

void updateButtons() {
  updateOneButton(p1Button,1);
  updateOneButton(p2Button,2);
}

void updateGame() {
  if (gameState == GAME_COUNTDOWN && millis() - countdownStarted >= 3000) {
    gameState = GAME_RACING;
    resetButton(p1Button);
    resetButton(p2Button);
    Serial.println("RESCUE!");
  }
}

int countdownValue() {
  if (gameState != GAME_COUNTDOWN) return 0;
  unsigned long e = millis() - countdownStarted;
  if (e < 1000) return 3;
  if (e < 2000) return 2;
  if (e < 3000) return 1;
  return 0;
}

const char PAGE[] PROGMEM =
R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Rescue Rene Ocean Rescue</title>
<style>
*{box-sizing:border-box}
html,body{margin:0;min-height:100%;font-family:monospace;background:#010914;color:#effcff}
body{padding:8px}
.game{max-width:1100px;margin:auto;border:4px solid #effcff;background:#031523;box-shadow:0 0 30px #005274}
header{padding:12px;text-align:center;border-bottom:4px solid #effcff;background:#061d31}
header b{display:block;color:#9eeeff;letter-spacing:4px;font-size:12px}
h1{margin:4px 0;font-size:clamp(38px,8vw,76px);text-shadow:5px 5px #075375}
header small{opacity:.75}
.status{display:grid;grid-template-columns:1fr auto 1fr;gap:10px;padding:9px 12px;align-items:center;border-bottom:4px solid #effcff}
#state{font-size:clamp(24px,5vw,44px);font-weight:900;color:#7ee9ff;border:3px solid #7ee9ff;padding:5px 14px;text-align:center}
.r{text-align:right}
.p1{color:#20dcff} .p2{color:#ffb238}
.ocean{position:relative;height:580px;overflow:hidden;background:linear-gradient(#2bb8df,#0872a3 35%,#03496d 68%,#022f49)}
.ocean:before{content:"";position:absolute;inset:0;background:radial-gradient(circle,#fff8 0 2px,transparent 3px);background-size:100px 120px;animation:bubbles 5s linear infinite}
@keyframes bubbles{to{background-position:0 -120px}}
.surface{height:32px;background:repeating-linear-gradient(90deg,#91efff 0 30px,#42cdeb 30px 60px,#148caf 60px 90px);animation:wave 1.2s linear infinite}
@keyframes wave{to{transform:translateX(90px)}}
.midline{position:absolute;left:50%;top:40px;bottom:40px;width:5px;background:repeating-linear-gradient(#ffe45d 0 16px,#173b52 16px 32px)}
.rene{position:absolute;left:50%;top:200px;transform:translateX(-50%);text-align:center;z-index:5}
canvas{image-rendering:pixelated;image-rendering:crisp-edges}
#rene{width:150px;height:150px}
.tag{padding:5px 9px;background:#03111b;border:3px solid #ffe45d;color:#ffe45d;font-weight:900}
.lane{position:absolute;left:2%;right:2%;height:170px;border-top:3px dashed #ffffff77;border-bottom:3px dashed #ffffff22}
#lane1{top:70px} #lane2{bottom:70px}
.diver{position:absolute;top:22px;width:180px;transition:left .055s linear,right .055s linear}
#d1{left:0} #d2{right:0}
.diver canvas{width:160px;height:105px}
.dist{text-align:center;padding:4px;background:#03111b;border:2px solid currentColor;font-weight:900}
#d1 .dist{color:#20dcff} #d2 .dist{color:#ffb238}
.info{display:grid;grid-template-columns:1fr 1fr;border-top:4px solid #effcff}
.card{padding:12px} .card:first-child{border-right:4px solid #effcff}
.meter{height:24px;border:3px solid #effcff;background:#02070c;margin-top:8px}
.fill{height:100%;width:0;transition:width .055s linear}
#b1{background:#20dcff} #b2{background:#ffb238}
.controls{display:flex;gap:10px;justify-content:center;flex-wrap:wrap;padding:14px;border-top:4px solid #effcff}
button{padding:14px 22px;min-width:220px;font:900 16px monospace;border:3px solid #effcff;background:#0b3452;color:#fff;box-shadow:5px 5px #01070b}
button:active{transform:translate(3px,3px);box-shadow:2px 2px #01070b}
#win{display:none;position:absolute;inset:90px 4%;z-index:20;background:#020b14ee;border:7px solid #59f29a;text-align:center;padding:24px}
#win h2{font-size:clamp(34px,7vw,70px);margin:8px 0;color:#59f29a}
#winRene{width:180px;height:180px}
@media(max-width:700px){
  .status{grid-template-columns:1fr;text-align:center}
  .r{text-align:center}
  .ocean{height:520px}
  .diver{width:135px}
  .diver canvas{width:125px;height:82px}
  .info{grid-template-columns:1fr}
  .card:first-child{border-right:0;border-bottom:4px solid #effcff}
}
</style>
</head>
<body>

<div class="game">
<header>
  <b>MX CLUB ARCADE</b>
  <h1>RESCUE RENE</h1>
  <small>OCEAN RESCUE // FIRST DIVER TO 60M WINS</small>
</header>

<div class="status">
  <strong class="p1">PLAYER 1 // GPIO25</strong>
  <div id="state">READY</div>
  <strong class="p2 r">PLAYER 2 // GPIO26</strong>
</div>

<div class="ocean">
  <div class="surface"></div>
  <div class="midline"></div>

  <div class="rene">
    <canvas id="rene" width="40" height="40"></canvas>
    <div class="tag" id="reneTag">RENE // NEEDS HELP</div>
  </div>

  <div class="lane" id="lane1">
    <div class="diver" id="d1">
      <canvas id="c1" width="64" height="42"></canvas>
      <div class="dist" id="dist1">60M TO RENE</div>
    </div>
  </div>

  <div class="lane" id="lane2">
    <div class="diver" id="d2">
      <canvas id="c2" width="64" height="42"></canvas>
      <div class="dist" id="dist2">60M TO RENE</div>
    </div>
  </div>

  <div id="win">
    <div>MISSION SUCCESS</div>
    <h2 id="winText">PLAYER 1 RESCUED RENE!</h2>
    <canvas id="winRene" width="40" height="40"></canvas>
    <div>RENE IS SAFE!</div>
  </div>
</div>

<div class="info">
  <div class="card">
    <b class="p1">PLAYER 1</b>
    <span id="m1" style="float:right">60M AWAY</span>
    <div class="meter"><div class="fill" id="b1"></div></div>
  </div>
  <div class="card">
    <b class="p2">PLAYER 2</b>
    <span id="m2" style="float:right">60M AWAY</span>
    <div class="meter"><div class="fill" id="b2"></div></div>
  </div>
</div>

<div class="controls">
  <button id="start">START RESCUE MISSION</button>
  <button id="reset">RESET MISSION</button>
</div>
</div>

<audio id="mama" preload="auto" src="data:audio/mpeg;base64,
)HTML"
"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"
"KqHnQ0DUNCUtKsh13Iho9ypLctwdDR7/yoCJM8JB0SgqAsP4isGRBNBGCUaSIqdOj7ctVMfWk5iYU5hDmEOYTQonzITCARhNTCoSs/77CQj/8zjE5xZA+ubW"
"0ZJYhWGZMWprHGoys/qqHVMJGolqdDTf070HhcEJN0t0LdlZFTwriVx2CsfVTEFNRTMuMTAwVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
"VVVVVVVVVVX/8zjE7Re5Ur2eMMygVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVOVZVTEFNRTMuMTAwVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
"VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVX/8zjE7RYKafwAYE7cVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
"VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVX/8zjEowCQBAAA0AAAVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
"VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVU="
R"HTML(
"></audio>

<script>
const d1=document.getElementById('d1'),d2=document.getElementById('d2');
const state=document.getElementById('state'),reneTag=document.getElementById('reneTag');
const dist1=document.getElementById('dist1'),dist2=document.getElementById('dist2');
const m1=document.getElementById('m1'),m2=document.getElementById('m2');
const b1=document.getElementById('b1'),b2=document.getElementById('b2');
const win=document.getElementById('win'),winText=document.getElementById('winText');
let lastState=null,frame=0;

const playReneSound=()=>{
  const a=document.getElementById('mama');
  try{a.pause();a.currentTime=0;const p=a.play();if(p)p.catch(()=>{})}catch(e){}
}

const C=(id)=>{const c=document.getElementById(id),x=c.getContext('2d');x.imageSmoothingEnabled=false;return [c,x]}
const px=(x,a,b,c,d,k)=>{x.fillStyle=k;x.fillRect(a,b,c,d)}

const diver=(id,p,f)=>{
  const [c,x]=C(id);x.clearRect(0,0,c.width,c.height);
  const flip=p===2,main=p===1?'#20dcff':'#ffb238',dark=p===1?'#075e75':'#945400';
  x.save();if(flip){x.translate(c.width,0);x.scale(-1,1)}
  px(x,2,7+f%3,2,2,'#dffcff');px(x,8,3+(f+1)%3,1,1,'#dffcff');
  px(x,4,28+f%2,10,4,dark);px(x,0,30+f%2,7,3,main);
  px(x,14,11,8,17,'#b6c5d8');px(x,22,13,19,16,main);
  px(x,39,6,12,12,'#e9a16d');px(x,42,4,7,3,'#4f2c23');
  px(x,43,8,11,5,'#172b3c');px(x,45,9,7,3,'#9bf6ff');
  px(x,35,25,15,4,'#e9a16d');px(x,25,14,10,2,'#fff');
  x.restore();
}

const rene=(id,safe,f)=>{
  const [c,x]=C(id);x.clearRect(0,0,c.width,c.height);const bob=safe?0:f%2;
  px(x,13,5+bob,14,4,'#4f2c23');px(x,11,8+bob,18,5,'#4f2c23');
  px(x,13,11+bob,14,10,'#e9a16d');px(x,16,14+bob,2,2,'#101820');px(x,23,14+bob,2,2,'#101820');
  px(x,11,21+bob,18,10,safe?'#59f29a':'#ffe45d');px(x,15,22+bob,10,2,'#fff');
  if(safe){
    px(x,5,9,4,10,'#e9a16d');px(x,31,9,4,10,'#e9a16d');
    px(x,2,6,5,1,'#fff36b');px(x,4,4,1,5,'#fff36b');
    px(x,33,6,5,1,'#fff36b');px(x,35,4,1,5,'#fff36b');
  }else{
    px(x,7,22+bob,5,4,'#e9a16d');px(x,28,22+bob,5,4,'#e9a16d');
  }
  px(x,13,31+bob,6,6,'#25355d');px(x,22,31+bob,6,6,'#25355d');
}

const move=(el,score,p,target)=>{
  const travel=Math.min(100,score/target*100)*.385;
  if(p===1)el.style.left=travel+'%';else el.style.right=travel+'%';
}

const render=(data)=>{
  const p1=Number(data.p1)||0,p2=Number(data.p2)||0,t=Number(data.target)||60;
  const r1=Math.max(0,t-p1),r2=Math.max(0,t-p2);
  const justWon=lastState!==null && data.state==='FINISHED' && lastState!=='FINISHED';

  move(d1,p1,1,t);move(d2,p2,2,t);
  dist1.textContent=r1+'M TO RENE';dist2.textContent=r2+'M TO RENE';
  m1.textContent=r1+'M AWAY';m2.textContent=r2+'M AWAY';
  b1.style.width=Math.min(100,p1/t*100)+'%';b2.style.width=Math.min(100,p2/t*100)+'%';

  win.style.display='none';

  if(data.state==='IDLE'){
    state.textContent='READY';reneTag.textContent='RENE // NEEDS HELP';
  }else if(data.state==='COUNTDOWN'){
    state.textContent=data.countdown>0?data.countdown:'GO!';reneTag.textContent='RENE // HOLD ON!';
  }else if(data.state==='RACING'){
    state.textContent='RESCUE!';reneTag.textContent='RENE // NEEDS HELP';
  }else{
    state.textContent='SAFE!';reneTag.textContent='RENE // SAFE';
    win.style.display='block';
    winText.textContent='PLAYER '+data.winner+' RESCUED RENE!';
    if(justWon)playReneSound();
  }

  lastState=data.state;
}

const poll=async()=>{
  try{
    const r=await fetch('/state?t='+Date.now(),{cache:'no-store'});
    render(await r.json());
  }catch(e){state.textContent='CONNECTION LOST'}
}

document.getElementById('start').onclick=async()=>{
  playReneSound();
  await fetch('/start',{method:'POST'});
  lastState=null;
  poll();
};

document.getElementById('reset').onclick=async()=>{
  await fetch('/reset',{method:'POST'});
  lastState=null;
  poll();
};

setInterval(()=>{
  frame++;
  diver('c1',1,frame);diver('c2',2,frame);
  const safe=state.textContent==='SAFE!';
  rene('rene',safe,frame);rene('winRene',true,frame);
},220);

diver('c1',1,0);diver('c2',2,0);rene('rene',false,0);rene('winRene',true,0);
setInterval(poll,55);
poll();
</script>
</body>
</html>
)HTML";

void handleRoot(){
  server.send_P(200,"text/html",PAGE);
}

void handleStart(){
  startRace();
  server.send(200,"text/plain","OK");
}

void handleReset(){
  resetGame();
  server.send(200,"text/plain","OK");
}

void handleState(){
  const char* s="FINISHED";

  if(gameState==GAME_IDLE)s="IDLE";
  else if(gameState==GAME_COUNTDOWN)s="COUNTDOWN";
  else if(gameState==GAME_RACING)s="RACING";

  char json[220];

  snprintf(
    json,
    sizeof(json),
    "{\"state\":\"%s\",\"p1\":%d,\"p2\":%d,\"target\":%d,\"winner\":%d,\"countdown\":%d}",
    s,
    player1Score,
    player2Score,
    TARGET_DISTANCE,
    winner,
    countdownValue()
  );

  server.sendHeader("Cache-Control","no-store");
  server.send(200,"application/json",json);
}

void handleNotFound(){
  server.send(404,"text/plain","NOT FOUND");
}

void setup(){
  Serial.begin(115200);
  delay(300);

  pinMode(PLAYER1_PIN,INPUT_PULLUP);
  pinMode(PLAYER2_PIN,INPUT_PULLUP);

  resetButton(p1Button);
  resetButton(p2Button);

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAPConfig(localIP,gateway,subnet);

  bool ok=WiFi.softAP(WIFI_NAME,WIFI_PASSWORD);

  server.on("/",HTTP_GET,handleRoot);
  server.on("/state",HTTP_GET,handleState);
  server.on("/start",HTTP_POST,handleStart);
  server.on("/reset",HTTP_POST,handleReset);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println();
  Serial.println("===============================");
  Serial.println("RESCUE RENE OCEAN + MAMA SOUND");
  Serial.println("===============================");
  Serial.println(ok ? "WIFI READY" : "WIFI FAILED");
  Serial.print("OPEN: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("P1 GPIO25 -> BUTTON -> GND");
  Serial.println("P2 GPIO26 -> BUTTON -> GND");
}

void loop(){
  updateButtons();
  updateGame();

  server.handleClient();

  updateButtons();
  updateGame();

  delay(1);
}
