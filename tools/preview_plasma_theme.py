#!/usr/bin/env python3
"""
FreeLook: checks the Plasma style elements and draws a preview (panel,
pop-up menu, tooltip and a desktop widget), assembling the 9-piece frames
the way KSvg does.

    python3 tools/preview_plasma_theme.py plasma/desktoptheme/FreeLook preview_plasma.png

Requires PyQt5 (python3-pyqt5 and python3-pyqt5.qtsvg). Exit code 1 if an element is missing.

SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
SPDX-License-Identifier: GPL-3.0-or-later
"""
import os
import sys

from PyQt5.QtCore import QRectF, Qt
from PyQt5.QtGui import QColor, QFont, QImage, QPainter
from PyQt5.QtSvg import QSvgRenderer
from PyQt5.QtWidgets import QApplication

app = QApplication(sys.argv[:1])
theme, output = sys.argv[1], sys.argv[2]
SCALE = int(os.environ.get("SCALE", "2"))
PARTS = ["topleft", "top", "topright", "left", "center", "right",
         "bottomleft", "bottom", "bottomright"]

renderers = {}


def svg(path):
    if path not in renderers:
        renderers[path] = QSvgRenderer(os.path.join(theme, path + ".svg"))
    return renderers[path]


# ------------------------------------------------------------------ checks
EXPECTED = {
    "dialogs/background": [""], "widgets/background": [""], "widgets/tooltip": [""],
    "widgets/panel-background": [""],
    "widgets/button": ["normal", "pressed", "focus", "toolbutton-hover", "toolbutton-pressed", "toolbutton-focus"],
    "widgets/viewitem": ["normal", "hover", "selected", "selected+hover"],
    "widgets/tasks": ["normal", "hover", "focus", "attention", "minimized", "progress"],
    "widgets/lineedit": ["base", "focus"],
    "widgets/frame": ["plain", "raised", "sunken"],
    "widgets/plasmoidheading": ["header", "footer"],
    "widgets/scrollbar": ["background-vertical", "background-horizontal", "slider", "mouseover-slider"],
}
errors = 0
for path, prefixes in EXPECTED.items():
    r = svg(path)
    if not r.isValid():
        print("INVALID SVG:", path); errors += 1; continue
    for p in prefixes:
        for part in PARTS:
            eid = f"{p}-{part}" if p else part
            if not r.elementExists(eid):
                print("MISSING:", path, eid); errors += 1
            elif r.boundsOnElement(eid).isEmpty():
                print("EMPTY:", path, eid); errors += 1
print("checks:", "OK" if errors == 0 else f"{errors} problem(s)")


# ------------------------------------------------------------------ drawing
def frame(p, path, prefix, x, y, w, h):
    r = svg(path)
    eid = lambda part: f"{prefix}-{part}" if prefix else part
    b = lambda part: r.boundsOnElement(eid(part))
    left, right = b("left").width(), b("right").width()
    top, bottom = b("top").height(), b("bottom").height()
    cw, ch = w - left - right, h - top - bottom
    areas = {"topleft": (x, y, left, top), "top": (x + left, y, cw, top), "topright": (x + left + cw, y, right, top),
             "left": (x, y + top, left, ch), "center": (x + left, y + top, cw, ch), "right": (x + left + cw, y + top, right, ch),
             "bottomleft": (x, y + top + ch, left, bottom), "bottom": (x + left, y + top + ch, cw, bottom),
             "bottomright": (x + left + cw, y + top + ch, right, bottom)}
    for part, a in areas.items():
        r.render(p, eid(part), QRectF(*a))


def text(p, x, y, w, h, s, size=12, bold=False, align=Qt.AlignLeft | Qt.AlignVCenter, color="#000000"):
    f = QFont("DejaVu Sans"); f.setPixelSize(size); f.setBold(bold); p.setFont(f)
    p.setPen(QColor(color)); p.drawText(QRectF(x, y, w, h), align, s)


W, H = 620, 430
img = QImage(W * SCALE, H * SCALE, QImage.Format_ARGB32)
img.fill(QColor("#40a0c0"))
p = QPainter(img)
p.setRenderHint(QPainter.Antialiasing, True)
p.setRenderHint(QPainter.TextAntialiasing, True)
p.scale(SCALE, SCALE)

# Panel at the bottom, with tasks in several states.
frame(p, "widgets/panel-background", "", -3, H - 40, W + 6, 43)
x = 50
for label, state in [("Dolphin", "normal"), ("Konsole (active)", "focus"), ("Firefox (hover)", "hover"),
                     ("Attention", "attention")]:
    frame(p, "widgets/tasks", state, x, H - 35, 130, 30)
    text(p, x + 10, H - 35, 115, 30, label)
    x += 136
frame(p, "widgets/button", "toolbutton-hover", 6, H - 36, 38, 32)
text(p, 6, H - 36, 38, 32, "▤", 14, align=Qt.AlignCenter)

# Pop-up menu (like Kickoff).
mx, my, mw, mh = 20, 20, 300, 340
frame(p, "dialogs/background", "", mx, my, mw, mh)
frame(p, "widgets/plasmoidheading", "header", mx + 3, my + 3, mw - 6, 44)
frame(p, "widgets/lineedit", "base", mx + 12, my + 12, 190, 26)
text(p, mx + 18, my + 12, 180, 24, "Search...", color="#555555")
frame(p, "widgets/button", "normal", mx + 210, my + 10, 78, 30)
text(p, mx + 210, my + 10, 78, 30, "Pin", align=Qt.AlignCenter)
items = [("System Settings", "normal"), ("Dolphin", "selected"), ("Konsole", "hover"),
         ("Firefox", "normal"), ("Kate", "normal"), ("Spotify", "normal")]
for i, (label, state) in enumerate(items):
    y = my + 56 + i * 34
    frame(p, "widgets/viewitem", state, mx + 8, y, mw - 34, 32)
    text(p, mx + 18, y, mw - 50, 32, label)
frame(p, "widgets/scrollbar", "background-vertical", mx + mw - 22, my + 56, 16, 200)
frame(p, "widgets/scrollbar", "slider", mx + mw - 22, my + 70, 16, 60)
frame(p, "widgets/plasmoidheading", "footer", mx + 3, my + mh - 50, mw - 6, 47)
frame(p, "widgets/button", "pressed", mx + 12, my + mh - 42, 110, 30)
text(p, mx + 12, my + mh - 42, 110, 30, "Shut Down", align=Qt.AlignCenter)
frame(p, "widgets/button", "normal", mx + 130, my + mh - 42, 110, 30)
frame(p, "widgets/button", "focus", mx + 130, my + mh - 42, 110, 30)
text(p, mx + 130, my + mh - 42, 110, 30, "Restart", align=Qt.AlignCenter)

# Tooltip.
frame(p, "widgets/tooltip", "", 360, 300, 220, 60)
text(p, 372, 306, 200, 22, "Konsole", 12, True)
text(p, 372, 328, 200, 22, "~ : bash", 11)

# Desktop widget with a focused field.
frame(p, "widgets/background", "", 360, 40, 230, 120)
text(p, 376, 50, 200, 22, "Notes", 12, True)
frame(p, "widgets/lineedit", "base", 376, 80, 196, 28)
frame(p, "widgets/lineedit", "focus", 376, 80, 196, 28)
text(p, 382, 80, 190, 28, "focused field")
frame(p, "widgets/frame", "sunken", 376, 116, 196, 30)
text(p, 382, 116, 190, 30, "sunken frame", 11)
p.end()
img.save(output)
print("preview:", output)
sys.exit(1 if errors else 0)
