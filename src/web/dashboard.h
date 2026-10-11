#pragma once

const char DASHBOARD_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ECHOLOT · Sonar Navigation Suite</title>
<link rel="stylesheet" href="dashboard.css">
</head>
<body>
<div class="topbar">
  <div class="title">
    <h1>ECHOLOT</h1>
  </div>
  <div class="clocks">
    <div class="clock"><small>UTC</small><b id="utc">--:--:--</b></div>
    <div class="clock"><small>MET</small><b id="met">00:00:00</b></div>
  </div>
</div>
<main>
  <section class="panel" id="telem" data-label="TELEMETRY">
    <div class="stats">
      <div class="stat"><small>MODE</small><b id="mode">-</b></div>
      <div class="stat"><small>RANGE AHEAD</small><b id="dist">-</b></div>
      <div class="stat"><small>SPEED</small><b id="speed">--</b><small id="peak">PEAK --</small></div>
      <div class="stat"><small>DOWNLINK</small><b id="link" class="bad">ACQ</b></div>
    </div>
    <div class="bar"><i id="distbar"></i></div>
    <div class="scale"><span>0</span><span>50</span><span>100</span><span>150</span><span>200 CM</span></div>
  </section>
  <section class="panel" id="modes" data-label="FLIGHT DIRECTOR · MODE SELECT">
    <div class="modes">
      <button data-mode="default">Default</button>
      <button data-mode="default2">Default 2</button>
      <button data-mode="default3">Default 3</button>
      <button data-mode="default4">Default 4</button>
      <button data-mode="follow">Follow</button>
      <button data-mode="radar">Radar</button>
      <button data-mode="manual">Manual</button>
      <button data-mode="stop" id="stop">Abort</button>
    </div>
    <div class="tune">
      <label>DRIVE SPEED % <input type="number" id="drive" min="10" max="100" step="1" value="100"></label>
      <label>TURN SPEED % <input type="number" id="turn" min="10" max="100" step="1" value="100"></label>
      <label>STOP AT CM <input type="number" id="stopcm" min="6" max="40" step="1" value="12"></label>
      <label>90° TURN MS <input type="number" id="t90" min="200" max="6000" step="50" value="1200"></label>
    </div>
  </section>
  <section class="panel" id="radarp" data-label="NAV · SONAR SWEEP">
    <canvas id="radar" width="480" height="260"></canvas>
  </section>
  <section class="panel" id="drivep" data-label="GUIDANCE · MANUAL DRIVE">
    <div class="pad">
      <span></span><button data-drive="f">&#9650;</button><span></span>
      <button data-drive="l">&#9664;</button><button data-drive="s">&#9632;</button><button data-drive="r">&#9654;</button>
      <span></span><button data-drive="b">&#9660;</button><span></span>
    </div>
  </section>
  <section class="panel" id="comm" data-label="CAPCOM · VOICE LOOP">
    <button id="voice">&#127908; Start voice</button>
    <div id="heard">Say "rover" then: follow, default, radar, manual, stop, forward, back, left, right</div>
  </section>
  <section class="panel" id="mapp" data-label="NAV · DEFAULT 2 / 3 MAP">
    <canvas id="map" width="1000" height="560"></canvas>
    <div class="tune">
      <label>STEP
        <select id="step">
          <option>1</option><option>2</option><option>3</option><option selected>5</option>
          <option>10</option><option>15</option><option>20</option><option>30</option>
        </select>
      </label>
      <label>READS
        <select id="reads">
          <option>1</option><option selected>2</option><option>3</option>
        </select>
      </label>
      <span id="mapinfo">NO SCAN YET</span>
    </div>
  </section>
  <section class="panel" id="logp" data-label="EVENT LOG">
    <pre id="log"></pre>
  </section>
</main>
<footer>ECHOLOT · BUILT BY SIX FIRST-YEAR ENGINEERS IN 48 HOURS · LUT DEEPTECH HACKATHON 2026</footer>
<script src="dashboard.js"></script>
</body>
</html>
)rawliteral";

const char DASHBOARD_CSS[] = R"rawliteral(
:root {
  --bg: #0a0d0b;
  --panel: #111613;
  --panel2: #161d19;
  --line: #24302a;
  --grid: #13202f;
  --text: #dfe9e2;
  --dim: #7f9387;
  --green: #39ff88;
  --amber: #ffb340;
  --red: #ff4d4d;
  --blue: #8fe3ad;
  --cell: rgba(57, 255, 136, 0.035);
  --cell-major: rgba(57, 255, 136, 0.08);
  --mono: "SF Mono", "Consolas", "Menlo", "DejaVu Sans Mono", monospace;
}

* {
  box-sizing: border-box;
}

body {
  position: relative;
  margin: 0;
  font-family: var(--mono);
  color: var(--text);
  min-height: 100vh;
  background-color: var(--bg);
  background-image:
    repeating-linear-gradient(0deg, var(--cell-major) 0 1px, transparent 1px 120px),
    repeating-linear-gradient(90deg, var(--cell-major) 0 1px, transparent 1px 120px),
    repeating-linear-gradient(0deg, var(--cell) 0 1px, transparent 1px 24px),
    repeating-linear-gradient(90deg, var(--cell) 0 1px, transparent 1px 24px);
  background-attachment: fixed;
}


main,
footer {
  position: relative;
  z-index: 1;
}

.topbar {
  position: sticky;
  top: 0;
  z-index: 2;
  display: flex;
  align-items: center;
  gap: 14px;
  padding-block: 10px;
  padding-inline: max(16px, calc((100% - 1400px) / 2 + 16px));
  border-bottom: 2px solid var(--line);
  background: #0e120f;
  backdrop-filter: blur(6px);
}

.title h1 {
  margin: 0;
  color: var(--blue);
  font-size: 22px;
  letter-spacing: 0.42em;
  text-shadow: 0 0 14px rgba(79, 195, 255, 0.45);
}

.title small {
  color: var(--dim);
  font-size: 11px;
  letter-spacing: 0.12em;
}

.clocks {
  display: flex;
  gap: 18px;
  margin-left: auto;
  text-align: right;
}

.clock small {
  display: block;
  color: var(--dim);
  font-size: 10px;
  letter-spacing: 0.15em;
}

.clock b {
  color: var(--amber);
  font-size: 18px;
  font-variant-numeric: tabular-nums;
}

main {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  grid-template-areas:
    "map   map   telem"
    "map   map   radar"
    "modes drive comm"
    "log   log   log";
  gap: 14px;
  max-width: 1400px;
  margin: 0 auto;
  padding: 16px;
}

.panel {
  position: relative;
  padding: 28px 14px 14px;
  border: 1px solid var(--line);
  border-radius: 4px;
  background: var(--panel);
}

.panel::before {
  content: attr(data-label);
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  padding: 5px 10px;
  border-bottom: 1px solid var(--line);
  background: var(--panel2);
  color: var(--amber);
  font-size: 10px;
  letter-spacing: 0.2em;
}

.panel::after {
  content: "";
  position: absolute;
  top: 8px;
  right: 10px;
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: var(--green);
  box-shadow: 0 0 6px var(--green);
}

#telem  { grid-area: telem; }
#modes  { grid-area: modes; }
#radarp { grid-area: radar; }
#drivep { grid-area: drive; }
#comm   { grid-area: comm; }
#mapp   { grid-area: map; }
#logp   { grid-area: log; }

.stats {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 18px 14px;
}

.stat small {
  display: block;
  color: var(--dim);
  font-size: 10px;
  letter-spacing: 0.15em;
}

.stat b {
  color: var(--green);
  font-size: 24px;
  text-transform: uppercase;
  text-shadow: 0 0 8px rgba(57, 255, 136, 0.4);
}

#peak {
  margin-top: 2px;
  color: var(--amber);
}

#link {
  font-size: 14px;
}

#link.ok {
  color: var(--green);
}

#link.bad {
  color: var(--red);
  text-shadow: 0 0 8px rgba(255, 77, 77, 0.5);
  animation: blink 1s steps(2) infinite;
}

@keyframes blink {
  50% {
    opacity: 0.3;
  }
}

.bar {
  height: 10px;
  margin-top: 12px;
  border: 1px solid var(--line);
  background: #050a10;
}

.bar i {
  display: block;
  width: 0;
  height: 100%;
  background: var(--green);
  transition: width 0.25s, background 0.25s;
}

.scale {
  display: flex;
  justify-content: space-between;
  margin-top: 3px;
  color: var(--dim);
  font-size: 10px;
}

.modes {
  display: grid;
  grid-template-columns: repeat(12, 1fr);
  gap: 6px;
}

.modes button { grid-column: span 4; }

.modes button[data-mode^="default"] { grid-column: span 3; }

button {
  padding: 12px 4px;
  border: 1px solid var(--line);
  border-radius: 3px;
  background: #18201b;
  color: var(--text);
  font: inherit;
  font-size: 12px;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  cursor: pointer;
  touch-action: manipulation;
  user-select: none;
}

button:hover {
  border-color: var(--blue);
}

button:active {
  transform: translateY(1px);
}

.modes button::before {
  content: "";
  display: block;
  width: 8px;
  height: 8px;
  margin: 0 auto 6px;
  border-radius: 50%;
  background: #2c3a31;
}

.modes button.on {
  border-color: var(--green);
  background: #123321;
  color: var(--green);
}

.modes button.on::before {
  background: var(--green);
  box-shadow: 0 0 8px var(--green);
}

#stop {
  grid-column: 1 / -1;
  border-color: var(--red);
  background: #3a0d0d;
  color: #ffd6d6;
  font-weight: 700;
}

#stop::before {
  background: var(--red);
  box-shadow: 0 0 8px var(--red);
}

.pad {
  display: grid;
  grid-template-columns: repeat(3, 72px);
  grid-template-rows: repeat(3, 56px);
  gap: 6px;
  justify-content: center;
}

.pad button {
  font-size: 20px;
}

.pad button[data-drive="s"] {
  color: var(--red);
}

.hint {
  margin: 10px 0 0;
  color: var(--dim);
  font-size: 11px;
  text-align: center;
}

canvas {
  display: block;
  width: 100%;
  height: auto;
  border: 1px solid var(--line);
  background: radial-gradient(ellipse at 50% 100%, #0b2014, #050a08 70%);
}

#voice {
  width: 100%;
  padding: 16px;
  border-color: #5a4214;
  color: var(--amber);
}

#heard {
  min-height: 18px;
  margin-top: 8px;
  color: var(--dim);
  font-size: 12px;
  text-align: center;
}

.tune {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 14px;
  margin-top: 10px;
  color: var(--dim);
  font-size: 11px;
  letter-spacing: 0.15em;
}

.tune input {
  width: 72px;
  margin-left: 6px;
  padding: 4px 6px;
  border: 1px solid var(--line);
  border-radius: 3px;
  background: #18201b;
  color: var(--text);
  font: inherit;
}

.tune select {
  margin-left: 6px;
  padding: 4px 6px;
  border: 1px solid var(--line);
  border-radius: 3px;
  background: #18201b;
  color: var(--text);
  font: inherit;
}

#mapinfo {
  margin-left: auto;
  color: var(--amber);
}

#log {
  height: 120px;
  margin: 0;
  overflow: auto;
  color: var(--dim);
  font-size: 12px;
  line-height: 1.6;
}

#log span {
  color: var(--amber);
}

footer {
  padding: 6px 0 18px;
  color: #6b7d92;
  font-size: 11px;
  letter-spacing: 0.1em;
  text-align: center;
}

@media (max-width: 1200px) {
  main {
    grid-template-columns: 1fr 1fr;
    grid-template-areas:
      "map   map"
      "telem radar"
      "modes drive"
      "comm  log";
  }
}

@media (max-width: 760px) {
  main {
    grid-template-columns: 1fr;
    grid-template-areas:
      "telem"
      "map"
      "radar"
      "modes"
      "drive"
      "comm"
      "log";
  }

  .clocks .clock:first-child {
    display: none;
  }

  .stats {
    grid-template-columns: 1fr 1fr;
  }

  .stat b {
    font-size: 20px;
  }
})rawliteral";

const char DASHBOARD_JS[] = R"rawliteral(
const $ = id => document.getElementById(id);
let current = null;
const t0 = Date.now();
const pad2 = n => String(n).padStart(2, '0');
const hms = ms => { const s = Math.floor(ms / 1000); return pad2(Math.floor(s / 3600)) + ':' + pad2(Math.floor(s / 60) % 60) + ':' + pad2(s % 60); };
setInterval(() => {
  const d = new Date();
  $('utc').textContent = pad2(d.getUTCHours()) + ':' + pad2(d.getUTCMinutes()) + ':' + pad2(d.getUTCSeconds());
  $('met').textContent = hms(Date.now() - t0);
}, 1000);
function log(msg) {
  const el = $('log');
  el.innerHTML += '<span>[' + hms(Date.now() - t0) + ']</span> ' + msg + '\n';
  el.scrollTop = el.scrollHeight;
}
log('MISSION CONTROL ONLINE. ACQUIRING ROVER...');
let linkUp = null, lastMode = null;
function setLink(ok) {
  if (ok === linkUp) return;
  linkUp = ok;
  $('link').className = ok ? 'ok' : 'bad';
  $('link').textContent = ok ? 'AOS' : 'LOS';
  log(ok ? 'ACQUISITION OF SIGNAL. ROVER IS GO.' : 'LOSS OF SIGNAL. CHECK WIFI / POWER.');
}
async function call(url) {
  try {
    const r = await fetch(url, { cache: 'no-store' });
    const s = await r.json();
    render(s);
    setLink(true);
    return s;
  } catch (e) {
    setLink(false);
  }
}
function render(s) {
  current = s;
  if (s.mode !== lastMode) { log('MODE -> ' + String(s.mode).toUpperCase()); lastMode = s.mode; }
  $('mode').textContent = s.mode;
  $('dist').textContent = s.dist >= 999 ? 'clear' : s.dist + ' cm';
  if (!turnSynced && s.turn) {
    $('turn').value = String(s.turn);
    if (s.drive) $('drive').value = String(s.drive);
    if (s.stop) $('stopcm').value = String(s.stop);
    if (s.t90) $('t90').value = String(s.t90);
    turnSynced = true;
  }
  $('speed').textContent = s.speed >= 0 ? s.speed + ' cm/s' : '--';
  $('peak').textContent = 'PEAK ' + (s.peak > 0 ? s.peak + ' cm/s' : '--');
  const pct = Math.min(100, (s.dist >= 999 ? 200 : s.dist) / 2);
  $('distbar').style.width = pct + '%';
  $('distbar').style.background = s.dist < 20 ? 'var(--red)' : s.dist < 40 ? 'var(--amber)' : 'var(--green)';
  document.querySelectorAll('[data-mode]').forEach(b => b.classList.toggle('on', b.dataset.mode === s.mode && s.mode !== 'stop'));
  drawRadar(s);
}
const setMode = m => call('/mode?m=' + m);
document.querySelectorAll('[data-mode]').forEach(b => b.onclick = () => setMode(b.dataset.mode));
let driveTimer = null;
async function startDrive(d) {
  stopDrive();
  if (!current || current.mode !== 'manual') await setMode('manual');
  call('/drive?d=' + d);
  driveTimer = setInterval(() => call('/drive?d=' + d), 200);
}
function stopDrive() {
  if (driveTimer) { clearInterval(driveTimer); driveTimer = null; call('/drive?d=s'); }
}
document.querySelectorAll('[data-drive]').forEach(b => {
  b.addEventListener('pointerdown', e => { e.preventDefault(); startDrive(b.dataset.drive); });
  ['pointerup', 'pointerleave', 'pointercancel'].forEach(ev => b.addEventListener(ev, stopDrive));
});
const keys = { ArrowUp: 'f', ArrowDown: 'b', ArrowLeft: 'l', ArrowRight: 'r' };
let keyHeld = null;
addEventListener('keydown', e => { const d = keys[e.key]; if (d && keyHeld !== d) { keyHeld = d; startDrive(d); e.preventDefault(); } });
addEventListener('keyup', e => { if (keys[e.key]) { keyHeld = null; stopDrive(); } });
const RADAR_FADE_MS = 3000;
const radarSeen = new Array(19).fill(0);
let lastNeck = null;

function markSwept(neck) {
  const now = Date.now();
  const from = lastNeck === null ? neck : lastNeck;
  const lo = Math.ceil(Math.min(from, neck) / 10);
  const hi = Math.floor(Math.max(from, neck) / 10);
  for (let i = lo; i <= hi; i++) radarSeen[i] = now;
  lastNeck = neck;
}

function drawRadar(s) {
  markSwept(s.neck);
  const c = $('radar'), g = c.getContext('2d');
  const cx = c.width / 2, cy = c.height - 10, R = cy - 10, max = 250;
  g.clearRect(0, 0, c.width, c.height);
  g.strokeStyle = 'rgba(57,255,136,.25)'; g.fillStyle = 'rgba(57,255,136,.6)'; g.font = '11px monospace';
  for (let cm = 50; cm <= max; cm += 50) {
    g.beginPath(); g.arc(cx, cy, R * cm / max, Math.PI, 2 * Math.PI); g.stroke();
    g.fillText(cm + '', cx + R * cm / max - 18, cy - 2);
  }
  for (let deg = 30; deg < 180; deg += 30) {
    g.beginPath(); g.moveTo(cx, cy);
    g.lineTo(cx + Math.cos(deg * Math.PI / 180) * R, cy - Math.sin(deg * Math.PI / 180) * R); g.stroke();
  }
  const pt = (deg, cm) => [cx + Math.cos(deg * Math.PI / 180) * R * cm / max, cy - Math.sin(deg * Math.PI / 180) * R * cm / max];
  g.fillStyle = '#39ff88'; g.shadowColor = '#39ff88'; g.shadowBlur = 10;
  const now = Date.now();
  s.radar.forEach((cm, i) => {
    const age = now - radarSeen[i];
    if (cm >= max || age > RADAR_FADE_MS) return;
    const [x, y] = pt(i * 10, cm);
    g.globalAlpha = 1 - age / RADAR_FADE_MS;
    g.beginPath(); g.arc(x, y, 4, 0, 2 * Math.PI); g.fill();
  });
  g.globalAlpha = 1;
  g.shadowBlur = 0;
  const [nx, ny] = pt(s.neck, max);
  g.strokeStyle = 'rgba(57,255,136,.9)'; g.lineWidth = 2; g.beginPath(); g.moveTo(cx, cy); g.lineTo(nx, ny); g.stroke(); g.lineWidth = 1;
}
const words = [
  [/stop|halt|freeze|abort/, () => { stopDrive(); setMode('stop'); }],
  [/follow|search|track/, () => setMode('follow')],
  [/default (4|four|for)|wall/, () => setMode('default4')],
  [/default (3|three)|aim/, () => setMode('default3')],
  [/default (2|two)|map/, () => setMode('default2')],
  [/default|avoid|auto/, () => setMode('default')],
  [/radar|scan/, () => setMode('radar')],
  [/manual/, () => setMode('manual')],
  [/forward|ahead/, () => pulse('f')],
  [/back/, () => pulse('b')],
  [/left/, () => pulse('l')],
  [/right/, () => pulse('r')],
];
function pulse(d) { startDrive(d); setTimeout(stopDrive, 700); }
const WAKE_WORD = 'rover';
const WAKE_WINDOW_MS = 5000;
const SR = window.SpeechRecognition || window.webkitSpeechRecognition;

if (!SR) {
  $('voice').disabled = true;
  $('heard').textContent = 'Voice needs Chrome or Edge.';
} else {
  const rec = new SR();
  rec.lang = 'en-US';
  rec.continuous = true;
  rec.interimResults = false;

  let listening = false;
  let awakeUntil = 0;

  function runCommand(text) {
    const hit = words.find(([re]) => re.test(text));
    log('CAPCOM: "' + text + '"' + (hit ? '' : ' (IGNORED)'));
    $('heard').textContent = '"' + text + '"' + (hit ? '' : ' (not a command)');
    if (hit) hit[1]();
    return Boolean(hit);
  }

  function setListening(on) {
    listening = on;
    $('voice').textContent = on ? '\u{1F3A4} Listening for "' + WAKE_WORD + '"' : '\u{1F3A4} Start voice';
    $('heard').textContent = on ? 'Say "' + WAKE_WORD + '" then a command, e.g. "rover radar"' : 'Voice off';
    if (on) rec.start(); else rec.stop();
  }

  rec.onresult = e => {
    const said = e.results[e.results.length - 1][0].transcript.toLowerCase().trim();
    const wakeAt = said.indexOf(WAKE_WORD);

    if (wakeAt >= 0) {
      const rest = said.slice(wakeAt + WAKE_WORD.length).trim();
      if (rest && runCommand(rest)) return;
      awakeUntil = Date.now() + WAKE_WINDOW_MS;
      $('heard').textContent = 'Yes? (listening for a command)';
      return;
    }

    if (Date.now() < awakeUntil) {
      awakeUntil = 0;
      runCommand(said);
    }
  };

  rec.onend = () => { if (listening) rec.start(); };
  rec.onerror = e => {
    if (e.error === 'no-speech' || e.error === 'aborted') return;
    $('heard').textContent = 'Voice error: ' + e.error;
    if (e.error === 'not-allowed') setListening(false);
  };

  $('voice').onclick = () => setListening(!listening);
}

const ROUTE_NAMES = { 0: 'RIGHT', 90: 'STRAIGHT', 180: 'LEFT' };
let mapBusy = false;

function drawMap(m) {
  const c = $('map'), g = c.getContext('2d');
  const cx = c.width / 2, cy = c.height - 14, R = Math.min(cy - 14, cx - 14), max = 130;
  const pt = (deg, cm) => [cx + Math.cos(deg * Math.PI / 180) * R * cm / max, cy - Math.sin(deg * Math.PI / 180) * R * cm / max];
  g.clearRect(0, 0, c.width, c.height);
  g.strokeStyle = 'rgba(143,227,173,.2)'; g.fillStyle = 'rgba(143,227,173,.6)'; g.font = '11px monospace';
  for (let cm = 20; cm <= 120; cm += 20) {
    g.beginPath(); g.arc(cx, cy, R * cm / max, Math.PI, 2 * Math.PI); g.stroke();
    g.fillText(cm + '', cx + R * cm / max - 16, cy - 2);
  }
  g.strokeStyle = 'rgba(255,77,77,.5)'; g.setLineDash([4, 4]);
  g.beginPath(); g.arc(cx, cy, R * 20 / max, Math.PI, 2 * Math.PI); g.stroke(); g.setLineDash([]);
  if (m.route >= 0) {
    const [rx, ry] = pt(m.route, max);
    g.strokeStyle = 'rgba(255,179,64,.9)'; g.lineWidth = 3;
    g.beginPath(); g.moveTo(cx, cy); g.lineTo(rx, ry); g.stroke(); g.lineWidth = 1;
  }
  if (m.aim >= 0) {
    const [ax, ay] = pt(m.aim, max);
    g.strokeStyle = 'rgba(143,227,173,.9)'; g.setLineDash([6, 4]); g.lineWidth = 2;
    g.beginPath(); g.moveTo(cx, cy); g.lineTo(ax, ay); g.stroke(); g.setLineDash([]); g.lineWidth = 1;
  }
  const n = m.pts.length;
  const angleOf = i => Math.min(i * m.step, 180);
  const points = m.pts.map((cm, i) => ({ deg: angleOf(i), cm }));
  g.strokeStyle = '#8fe3ad'; g.lineWidth = 2;
  let open = false;
  g.beginPath();
  points.forEach(p => {
    if (p.cm < 0 || p.cm >= 999 || p.cm > max) { open = false; return; }
    const [x, y] = pt(p.deg, p.cm);
    if (open) g.lineTo(x, y); else g.moveTo(x, y);
    open = true;
  });
  g.stroke(); g.lineWidth = 1;
  points.forEach(p => {
    if (p.cm < 0) return;
    const far = p.cm >= 999 || p.cm > max;
    const [x, y] = pt(p.deg, far ? max : p.cm);
    g.fillStyle = far ? 'rgba(107,125,146,.6)' : p.cm <= 20 ? '#ff4d4d' : '#39ff88';
    g.beginPath(); g.arc(x, y, far ? 2 : 3.5, 0, 2 * Math.PI); g.fill();
  });
  const done = m.pts.filter(v => v >= 0).length;
  $('mapinfo').textContent = 'SCAN #' + m.id + ' · ' + done + '/' + n + ' POINTS' + (m.route >= 0 ? ' · ROUTE ' + ROUTE_NAMES[m.route] : '') + (m.aim >= 0 ? ' · AIM ' + m.aim + '°' : '');
}

async function pollMap(url) {
  if (mapBusy) return;
  mapBusy = true;
  try {
    const r = await fetch(url || '/map', { cache: 'no-store' });
    drawMap(await r.json());
  } catch (e) {}
  mapBusy = false;
}

let turnSynced = false;
const setTune = (key, el, label) => {
  const v = Math.round(Math.min(+el.max, Math.max(+el.min, +el.value || +el.min)));
  el.value = v;
  fetch('/tune?' + key + '=' + v, { cache: 'no-store' }).catch(() => {});
  log(label + ' -> ' + v);
};
$('drive').onchange = () => setTune('drive', $('drive'), 'DRIVE SPEED %');
$('turn').onchange = () => setTune('turn', $('turn'), 'TURN SPEED %');
$('stopcm').onchange = () => setTune('stop', $('stopcm'), 'STOP AT CM');
$('t90').onchange = () => setTune('t90', $('t90'), '90° TURN MS');
const tune = () => pollMap('/tune?step=' + $('step').value + '&reads=' + $('reads').value);
$('step').onchange = tune;
$('reads').onchange = tune;
setInterval(() => { if (current && /^default[23]$/.test(current.mode)) pollMap(); }, 500);
pollMap();

let busy = false;
setInterval(async () => { if (busy) return; busy = true; await call('/status'); busy = false; }, 300);
)rawliteral";
