#include <WiFi.h>
#include <WebServer.h>

#define FAN_PWM 33

const char* ssid     = "SmartCoolingSystem";
const char* password = "12345678";

WebServer server(80);

int  fanSpeed  = 0;
bool ecoMode   = false;
bool fanRunning = false;
unsigned long fanStartTime = 0;
unsigned long totalRunTime = 0;

void updateRuntime() {
  if (fanSpeed > 0 && !fanRunning) {
    fanRunning    = true;
    fanStartTime  = millis();
  }
  if (fanSpeed == 0 && fanRunning) {
    fanRunning   = false;
    totalRunTime += millis() - fanStartTime;
  }
}

unsigned long getCurrentRuntime() {
  return fanRunning ? totalRunTime + (millis() - fanStartTime) : totalRunTime;
}

void applyFan() {
  updateRuntime();
  int actual = fanSpeed;
  if (ecoMode && actual > 180) actual = 180;
  ledcWrite(FAN_PWM, map(actual, 0, 255, 255, 0));
  Serial.printf("Speed: %d | Eco: %s\n", actual, ecoMode ? "ON" : "OFF");
}

void handleRoot() {
  const char* page = R"rawhtml(
<!DOCTYPE html>
<html lang="en" data-theme="light">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Cooling System</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Barlow:wght@300;400;500&display=swap" rel="stylesheet">
<style>
*{box-sizing:border-box;margin:0;padding:0}

html[data-theme="light"]{
  --bg:#dbeeff;
  --card:#fff;
  --border:#b8d8f5;
  --text:#1a2a3a;
  --text-muted:#8aaecc;
  --text-dim:#6a8faf;
  --text-dark:#2a4a6a;
  --accent:#1877c8;
  --accent-bg:#e8f3fd;
  --accent-text:#0c447c;
  --input-track:#c8def0;
  --preset-bg:#f0f7ff;
  --row-border:#e8f0f8;
  --blade:#378ADD;
  --hub:#185FA5;
  --svg-ring:#d0e8f8;
  --rpm-color:#a0bfd8;
  --eco-off-bg:#f0f5fa;
  --toggle-bg:#1877c8;
  --toggle-icon:"☀";
}
html[data-theme="dark"]{
  --bg:#0a0f1a;
  --card:#111827;
  --border:#1e3050;
  --text:#e0eaf8;
  --text-muted:#4a6a8a;
  --text-dim:#3a5a7a;
  --text-dark:#a0c0e0;
  --accent:#378ADD;
  --accent-bg:#0d1f35;
  --accent-text:#85B7EB;
  --input-track:#1e3050;
  --preset-bg:#0d1624;
  --row-border:#1a2a3a;
  --blade:#185FA5;
  --hub:#0C447C;
  --svg-ring:#1a2f4a;
  --rpm-color:#3a5a7a;
  --eco-off-bg:#0d1624;
  --toggle-bg:#253555;
  --toggle-icon:"☽";
}

body{font-family:'Barlow',sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:20px 16px;transition:background .25s,color .25s}
.wrap{max-width:460px;margin:0 auto;display:grid;gap:14px}

.header{display:flex;align-items:center;justify-content:space-between;background:var(--card);border:0.5px solid var(--border);border-radius:12px;padding:14px 18px;transition:background .25s,border-color .25s}
.header h1{font-size:16px;font-weight:500;color:var(--text);letter-spacing:.02em}
.header p{font-size:11px;color:var(--text-dim);margin-top:2px;letter-spacing:.08em;text-transform:uppercase}
.header-right{display:flex;align-items:center;gap:10px}
.dot{width:8px;height:8px;border-radius:50%;background:var(--accent);display:inline-block;margin-right:6px;animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.35}}
.online{font-family:'Share Tech Mono',monospace;font-size:11px;color:var(--accent);display:flex;align-items:center}

.theme-btn{width:32px;height:32px;border-radius:50%;background:var(--toggle-bg);border:0.5px solid var(--border);cursor:pointer;font-size:14px;display:flex;align-items:center;justify-content:center;transition:all .2s;color:var(--text);flex-shrink:0}
.theme-btn:hover{border-color:var(--accent)}

.fan-section{background:var(--card);border:0.5px solid var(--border);border-radius:16px;display:flex;flex-direction:column;align-items:center;padding:20px 0 14px;transition:background .25s,border-color .25s}
.ring-wrap{position:relative;width:200px;height:200px}
.ring-wrap svg{width:100%;height:100%}
.blades-g{transform-origin:100px 100px}
.rpm-display{text-align:center;margin-top:10px}
.rpm-num{font-family:'Share Tech Mono',monospace;font-size:32px;color:var(--text)}
.rpm-sub{font-size:11px;color:var(--text-muted);letter-spacing:.12em;text-transform:uppercase;margin-top:3px}

.metrics{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
.met{background:var(--card);border:0.5px solid var(--border);border-radius:10px;padding:13px;transition:background .25s,border-color .25s}
.met-label{font-size:10px;letter-spacing:.08em;text-transform:uppercase;color:var(--text-muted);margin-bottom:5px}
.met-val{font-family:'Share Tech Mono',monospace;font-size:20px;color:var(--text)}
.met-val.blue{color:var(--accent)}
.met-val small{font-size:11px;color:var(--text-muted);margin-left:1px}

.ctrl{background:var(--card);border:0.5px solid var(--border);border-radius:14px;padding:18px;transition:background .25s,border-color .25s}
.ctrl-head{display:flex;align-items:center;justify-content:space-between;margin-bottom:14px}
.ctrl-title{font-size:11px;letter-spacing:.1em;text-transform:uppercase;color:var(--text-muted)}
.badge{font-family:'Share Tech Mono',monospace;font-size:12px;background:var(--accent-bg);border:0.5px solid var(--accent);border-radius:6px;padding:3px 10px;color:var(--accent)}
.eco-badge{font-family:'Share Tech Mono',monospace;font-size:11px;border-radius:6px;padding:3px 10px}
.eco-badge.on{background:var(--accent-bg);border:0.5px solid var(--accent);color:var(--accent)}
.eco-badge.off{background:var(--eco-off-bg);border:0.5px solid var(--border);color:var(--text-muted)}

input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:5px;border-radius:3px;outline:none;border:none;cursor:pointer;background:linear-gradient(to right,var(--accent) var(--p,0%),var(--input-track) var(--p,0%));transition:background .25s}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:var(--card);border:2px solid var(--accent);cursor:pointer}
.ticks{display:flex;justify-content:space-between;margin-top:5px}
.tick{font-family:'Share Tech Mono',monospace;font-size:10px;color:var(--text-muted)}

.presets{display:grid;grid-template-columns:repeat(5,1fr);gap:7px;margin-top:14px}
.preset{background:var(--preset-bg);border:0.5px solid var(--border);border-radius:7px;padding:9px 4px;font-family:'Barlow',sans-serif;font-size:11px;color:var(--text-dim);cursor:pointer;text-align:center;transition:all .12s;letter-spacing:.04em}
.preset:hover{border-color:var(--accent);color:var(--accent)}
.preset.active{background:var(--accent-bg);border-color:var(--accent);color:var(--accent-text)}
.preset.danger{background:#1f0808;border-color:#7a2020;color:#e08080}
html[data-theme="light"] .preset.danger{background:#fff0f0;border-color:#e05555;color:#a32d2d}
.preset.danger:hover{border-color:#c0392b;color:#c0392b}

.dir-row{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:12px}
.dir{background:var(--preset-bg);border:0.5px solid var(--border);border-radius:8px;padding:11px;font-size:11px;letter-spacing:.06em;text-transform:uppercase;color:var(--text-dim);cursor:pointer;text-align:center;transition:all .12s}
.dir:hover{border-color:var(--accent);color:var(--accent)}
.dir.active{background:var(--accent-bg);border-color:var(--accent);color:var(--accent-text)}

.info-row{background:var(--card);border:0.5px solid var(--border);border-radius:10px;padding:12px 16px;transition:background .25s,border-color .25s}
.info-item{display:flex;justify-content:space-between;align-items:center;padding:5px 0;border-bottom:0.5px solid var(--row-border);font-size:12px}
.info-item:last-child{border-bottom:none}
.info-key{color:var(--text-muted);font-size:11px;letter-spacing:.05em}
.info-val{font-family:'Share Tech Mono',monospace;font-size:11px;color:var(--text-dark)}
</style>
</head>
<body>
<div class="wrap">

<div class="header">
  <div>
    <h1>Smart Cooling System</h1>
    <p>ESP32 &mdash; SoftAP Dashboard</p>
  </div>
  <div class="header-right">
    <div class="online"><span class="dot"></span>Online</div>
    <button class="theme-btn" id="theme-btn" onclick="toggleTheme()" title="Toggle dark/light mode">&#9790;</button>
  </div>
</div>

<div class="fan-section">
  <div class="ring-wrap">
    <svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
      <circle cx="100" cy="100" r="90" fill="none" stroke="#d0e8f8" stroke-width="1" id="svg-ring"/>
      <circle cx="100" cy="100" r="90" fill="none" stroke="#1877c8" stroke-width="3"
        stroke-dasharray="565" id="arc" stroke-dashoffset="565"
        stroke-linecap="round" transform="rotate(-90 100 100)" style="transition:stroke-dashoffset .35s"/>
      <g id="blades" class="blades-g">
        <ellipse cx="100" cy="64" rx="15" ry="30" fill="#378ADD" opacity=".85" transform="rotate(0 100 100)" id="b1"/>
        <ellipse cx="100" cy="64" rx="15" ry="30" fill="#378ADD" opacity=".85" transform="rotate(90 100 100)" id="b2"/>
        <ellipse cx="100" cy="64" rx="15" ry="30" fill="#378ADD" opacity=".85" transform="rotate(180 100 100)" id="b3"/>
        <ellipse cx="100" cy="64" rx="15" ry="30" fill="#378ADD" opacity=".85" transform="rotate(270 100 100)" id="b4"/>
        <circle cx="100" cy="100" r="11" fill="#fff" stroke="#185FA5" stroke-width="1.5" id="hub"/>
      </g>
    </svg>
  </div>
  <div class="rpm-display">
    <div class="rpm-num" id="rpm-big">0 <span id="rpm-unit" style="font-size:15px;color:#a0bfd8">RPM</span></div>
    <div class="rpm-sub">estimated speed</div>
  </div>
</div>

<div class="metrics">
  <div class="met">
    <div class="met-label">Speed</div>
    <div class="met-val blue" id="m-spd">0<small>%</small></div>
  </div>
  <div class="met">
    <div class="met-label">PWM</div>
    <div class="met-val" id="m-pwm">255<small>/255</small></div>
  </div>
  <div class="met">
    <div class="met-label">Runtime</div>
    <div class="met-val" id="m-time" style="font-size:14px">00:00:00</div>
  </div>
</div>

<div class="ctrl">
  <div class="ctrl-head">
    <span class="ctrl-title">Speed control</span>
    <span class="badge" id="pct-badge">0%</span>
  </div>
  <input type="range" id="slider" min="0" max="255" value="0" step="1" oninput="onSlider(this.value)">
  <div class="ticks"><span class="tick">0</span><span class="tick">64</span><span class="tick">128</span><span class="tick">192</span><span class="tick">255</span></div>
  <div class="presets">
    <div class="preset active" onclick="setSpeed(0)">Off</div>
    <div class="preset" onclick="setSpeed(60)">Silent</div>
    <div class="preset" onclick="setSpeed(120)">Normal</div>
    <div class="preset" onclick="setSpeed(190)">High</div>
    <div class="preset danger" onclick="setSpeed(255)">Turbo</div>
  </div>
</div>

<div class="ctrl">
  <div class="ctrl-head">
    <span class="ctrl-title">Eco mode</span>
    <span class="eco-badge off" id="eco-badge">Off</span>
  </div>
  <div class="dir-row">
    <div class="dir active" id="eco-off" onclick="setEco(false)">&#9632; Normal</div>
    <div class="dir" id="eco-on" onclick="setEco(true)">&#9675; Eco (max 70%)</div>
  </div>
</div>

<div class="info-row">
  <div class="info-item"><span class="info-key">Device</span><span class="info-val">ESP32 SoftAP</span></div>
  <div class="info-item"><span class="info-key">Network</span><span class="info-val">SmartCoolingSystem</span></div>
  <div class="info-item"><span class="info-key">IP</span><span class="info-val">192.168.4.1</span></div>
  <div class="info-item"><span class="info-key">Firmware</span><span class="info-val">v2.0 — J. Robellon</span></div>
</div>

</div>
<script>
let angle=0, curSpeed=0, rafId;
const blades=document.getElementById('blades');
const arc=document.getElementById('arc');
const html=document.documentElement;

const LIGHT={arc:'#1877c8',blade:'#378ADD',ring:'#d0e8f8',hub:'#fff',hubStroke:'#185FA5',rpmUnit:'#a0bfd8'};
const DARK={arc:'#378ADD',blade:'#185FA5',ring:'#1a2f4a',hub:'#111827',hubStroke:'#0C447C',rpmUnit:'#3a5a7a'};

function applyThemeColors(t){
  const c=t==='dark'?DARK:LIGHT;
  arc.style.stroke=c.arc;
  document.getElementById('svg-ring').style.stroke=c.ring;
  ['b1','b2','b3','b4'].forEach(id=>document.getElementById(id).setAttribute('fill',c.blade));
  document.getElementById('hub').setAttribute('fill',c.hub);
  document.getElementById('hub').setAttribute('stroke',c.hubStroke);
  document.getElementById('rpm-unit').style.color=c.rpmUnit;
  document.getElementById('theme-btn').textContent=t==='dark'?'\u2600':'\u263D';
}

function toggleTheme(){
  const t=html.getAttribute('data-theme')==='dark'?'light':'dark';
  html.setAttribute('data-theme',t);
  localStorage.setItem('theme',t);
  applyThemeColors(t);
}

(function(){
  const saved=localStorage.getItem('theme')||(window.matchMedia('(prefers-color-scheme: dark)').matches?'dark':'light');
  html.setAttribute('data-theme',saved);
  applyThemeColors(saved);
})();

function updateUI(raw){
  curSpeed=raw;
  const pct=Math.round((raw/255)*100);
  const pwm=255-raw;
  const rpm=Math.round(raw*11.76);
  document.getElementById('m-spd').innerHTML=pct+'<small>%</small>';
  document.getElementById('m-pwm').innerHTML=pwm+'<small>/255</small>';
  document.getElementById('rpm-big').innerHTML=rpm+' <span id="rpm-unit" style="font-size:15px;color:'+(html.getAttribute('data-theme')==='dark'?'#3a5a7a':'#a0bfd8')+'">RPM</span>';
  document.getElementById('pct-badge').textContent=pct+'%';
  document.getElementById('slider').value=raw;
  document.getElementById('slider').style.setProperty('--p',pct+'%');
  arc.setAttribute('stroke-dashoffset',Math.round(565-(565*(raw/255))));
  document.querySelectorAll('.preset').forEach(b=>{
    const v=parseInt(b.getAttribute('onclick').match(/\d+/)?.[0]??'-1');
    b.classList.toggle('active',!b.classList.contains('danger')&&v===raw);
  });
}

function onSlider(v){updateUI(parseInt(v));fetch('/speed?value='+v);}
function setSpeed(v){updateUI(v);fetch('/speed?value='+v);}

function setEco(on){
  document.getElementById('eco-on').classList.toggle('active',on);
  document.getElementById('eco-off').classList.toggle('active',!on);
  const badge=document.getElementById('eco-badge');
  badge.textContent=on?'On':'Off';
  badge.className='eco-badge '+(on?'on':'off');
  fetch('/eco?value='+(on?1:0));
}

function pollRuntime(){
  fetch('/runtime').then(r=>r.text()).then(t=>{
    document.getElementById('m-time').textContent=t;
  });
}
setInterval(pollRuntime,1000);

function spin(){
  if(curSpeed>0) angle=(angle+curSpeed*0.035)%360;
  blades.style.transform='rotate('+angle+'deg)';
  rafId=requestAnimationFrame(spin);
}
rafId=requestAnimationFrame(spin);
updateUI(0);
</script>
</body>
</html>
)rawhtml";
  server.send(200, "text/html", page);
}

void handleSpeed() {
  if (server.hasArg("value")) {
    fanSpeed = constrain(server.arg("value").toInt(), 0, 255);
    applyFan();
  }
  server.send(200, "text/plain", "OK");
}

void handleEco() {
  if (server.hasArg("value")) {
    ecoMode = server.arg("value").toInt() == 1;
    applyFan();
  }
  server.send(200, "text/plain", "OK");
}

void handleRuntime() {
  updateRuntime();
  unsigned long s = getCurrentRuntime() / 1000;
  char buf[12];
  sprintf(buf, "%02lu:%02lu:%02lu", s/3600, (s%3600)/60, s%60);
  server.send(200, "text/plain", buf);
}

void setup() {
  Serial.begin(115200);
  ledcAttach(FAN_PWM, 25000, 8);
  ledcWrite(FAN_PWM, 255);

  WiFi.softAP(ssid, password);
  Serial.println("Started — connect to: SmartCoolingSystem / 12345678");
  Serial.println("Dashboard: http://192.168.4.1");

  server.on("/",        handleRoot);
  server.on("/speed",   handleSpeed);
  server.on("/eco",     handleEco);
  server.on("/runtime", handleRuntime);
  server.begin();
}

void loop() {
  server.handleClient();
}
