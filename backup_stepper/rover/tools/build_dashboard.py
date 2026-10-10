from pathlib import Path

root = Path(__file__).resolve().parent.parent
web = root / "web"
parts = [("DASHBOARD_HTML", "dashboard.html"), ("DASHBOARD_CSS", "dashboard.css"), ("DASHBOARD_JS", "dashboard.js")]

out = ["#pragma once", ""]
for name, file in parts:
    text = (web / file).read_text(encoding="utf-8")
    out.append(f'const char {name}[] = R"rawliteral(\n{text})rawliteral";')
    out.append("")

(root / "src" / "web" / "dashboard.h").write_text("\n".join(out), encoding="utf-8")
