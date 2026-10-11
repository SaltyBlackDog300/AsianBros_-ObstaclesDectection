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
let linkUp = null, lastMode = null, lastPhase = null;
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
  $('learned').textContent = 'AUTO-CALIBRATED · STOP ' + s.stop + ' CM · 90° TURN ' + s.t90 + ' MS · CORRIDOR ' + s.corr + ' CM · CRUISE ' + s.cms + ' CM/S' + (s.phase && s.phase !== '-' ? ' · ' + s.phase.toUpperCase() : '');
  if (s.phase !== lastPhase) { if (s.phase && s.phase !== '-') log('AUTO: ' + s.phase.toUpperCase()); lastPhase = s.phase; }
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
