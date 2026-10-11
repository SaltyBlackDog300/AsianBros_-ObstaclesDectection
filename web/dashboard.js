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
