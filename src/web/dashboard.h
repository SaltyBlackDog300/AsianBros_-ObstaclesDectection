#pragma once

const char DASHBOARD_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Rover Mission Control</title>
<link rel="stylesheet" href="dashboard.css">
</head>
<body>
<div class="topbar">
  <svg class="patch" viewBox="0 0 100 100" aria-hidden="true">
    <circle cx="50" cy="50" r="46" fill="#0b1a2e" stroke="#ffb340" stroke-width="4"/>
    <circle cx="50" cy="50" r="38" fill="none" stroke="#1d2b3d" stroke-width="2"/>
    <path d="M14 70 Q50 30 86 70" fill="none" stroke="#4fc3ff" stroke-width="3"/>
    <rect x="34" y="44" width="32" height="14" rx="3" fill="#39ff88"/>
    <circle cx="40" cy="62" r="5" fill="#d6e2ee"/><circle cx="60" cy="62" r="5" fill="#d6e2ee"/>
    <circle cx="50" cy="22" r="3" fill="#ffb340"/>
  </svg>
  <div class="title">
    <h1>ROVER MISSION CONTROL</h1>
    <small>CHALLENGE 1 · OBSTACLE AVOIDANCE · FLIGHT CREW OF 6</small>
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
      <div class="stat"><small>DOWNLINK</small><b id="link" class="bad">ACQ</b></div>
    </div>
    <div class="bar"><i id="distbar"></i></div>
    <div class="scale"><span>0</span><span>50</span><span>100</span><span>150</span><span>200 CM</span></div>
  </section>
  <section class="panel" id="modes" data-label="FLIGHT DIRECTOR · MODE SELECT">
    <div class="modes">
      <button data-mode="stop" id="stop">Abort</button>
      <button data-mode="default">Default</button>
      <button data-mode="manual">Manual</button>
      <button data-mode="follow">Follow</button>
      <button data-mode="radar">Radar</button>
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
    <p class="hint">HOLD TO DRIVE (SWITCHES TO MANUAL) · ARROW KEYS OK</p>
  </section>
  <section class="panel" id="comm" data-label="CAPCOM · VOICE LOOP">
    <button id="voice">&#127908; Start voice</button>
    <div id="heard">Say "rover" then: follow, default, radar, manual, stop, forward, back, left, right</div>
  </section>
  <section class="panel" id="logp" data-label="EVENT LOG">
    <pre id="log"></pre>
  </section>
</main>
<footer>BUILT BY STUDENTS IN 48 HOURS · HACKATHON 2026 · NO ROVERS WERE HARMED (YET)</footer>
<script src="dashboard.js"></script>
</body>
</html>
)rawliteral";

const char DASHBOARD_CSS[] = R"rawliteral(
:root {
  --bg: #060a10;
  --panel: #0b121c;
  --panel2: #0f1825;
  --line: #1d2b3d;
  --grid: #13202f;
  --text: #d6e2ee;
  --dim: #6b7d92;
  --green: #39ff88;
  --amber: #ffb340;
  --red: #ff4d4d;
  --blue: #4fc3ff;
  --mono: "SF Mono", "Consolas", "Menlo", "DejaVu Sans Mono", monospace;
}

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: var(--mono);
  color: var(--text);
  background-color: var(--bg);
  background-image:
    linear-gradient(var(--grid) 1px, transparent 1px),
    linear-gradient(90deg, var(--grid) 1px, transparent 1px);
  background-size: 32px 32px;
}

body::after {
  content: "";
  position: fixed;
  inset: 0;
  pointer-events: none;
  background: repeating-linear-gradient(0deg, rgba(0, 0, 0, 0.18) 0 1px, transparent 1px 3px);
}

.topbar {
  position: sticky;
  top: 0;
  z-index: 2;
  display: flex;
  align-items: center;
  gap: 14px;
  padding: 10px 16px;
  border-bottom: 2px solid var(--line);
  background: linear-gradient(#0d1726, #08101a);
}

.patch {
  flex: none;
  width: 42px;
  height: 42px;
}

.title h1 {
  margin: 0;
  font-size: 16px;
  letter-spacing: 0.18em;
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
  grid-template-columns: 1fr 1fr;
  grid-template-areas:
    "telem radar"
    "modes radar"
    "drive comm"
    "log   log";
  gap: 12px;
  max-width: 1100px;
  margin: 0 auto;
  padding: 14px;
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
  color: var(--blue);
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
#logp   { grid-area: log; }

.stats {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  gap: 10px;
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
  grid-template-columns: repeat(5, 1fr);
  gap: 6px;
}

button {
  padding: 12px 4px;
  border: 1px solid var(--line);
  border-radius: 3px;
  background: #111c2a;
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
  background: #22303f;
}

.modes button.on {
  border-color: var(--green);
  background: #0b2018;
  color: var(--green);
}

.modes button.on::before {
  background: var(--green);
  box-shadow: 0 0 8px var(--green);
}

#stop {
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
  color: var(--dim);
  font-size: 11px;
  letter-spacing: 0.1em;
  text-align: center;
}

@media (max-width: 760px) {
  main {
    grid-template-columns: 1fr;
    grid-template-areas:
      "telem"
      "modes"
      "drive"
      "radar"
      "comm"
      "log";
  }

  .clocks .clock:first-child {
    display: none;
  }

  .stat b {
    font-size: 20px;
  }
}
)rawliteral";

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

let busy = false;
setInterval(async () => { if (busy) return; busy = true; await call('/status'); busy = false; }, 300);
)rawliteral";
