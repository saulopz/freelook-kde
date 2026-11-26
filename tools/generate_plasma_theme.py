#!/usr/bin/env python3
"""
FreeLook: generator for the Plasma style (panel, menus, tooltips, widgets).

    python3 tools/generate_plasma_theme.py plasma/desktoptheme/FreeLook

Backgrounds and text use Plasma's ColorScheme-* classes, so they follow the
active color scheme. The 3D relief (white and grays) is fixed, as in OPEN LOOK.
Every element uses absolute coordinates (no transforms), and arcs are FILLED
rings, never strokes: Qt measures a stroke half a pixel larger, and then the
corner pieces would not line up with their neighbors.

SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
SPDX-License-Identifier: GPL-3.0-or-later
"""
import json
import math
import os
import sys

LIGHT = "#ffffff"
SOFT_SHADOW = "#a0a0a0"
DARK = "#808080"
PRESSED = "#b7b7b7"
BG, TEXT, NONE = "BG", "TEXT", "NONE"     # symbolic colors

STYLE = """ <style type="text/css" id="current-color-scheme">
  .ColorScheme-Background { color:#cccccc; stop-color:#cccccc; }
  .ColorScheme-Text { color:#000000; stop-color:#000000; }
 </style>
"""


def fill(color):
    if color == BG:
        return 'class="ColorScheme-Background" fill="currentColor"'
    if color == TEXT:
        return 'class="ColorScheme-Text" fill="currentColor"'
    if color == NONE:
        return 'fill="#000000" fill-opacity="0"'
    return f'fill="{color}"'


def rect(x, y, w, h, color):
    return f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" {fill(color)}/>\n'


def document(width, height, body):
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="{width}" '
            f'height="{height}" viewBox="0 0 {width} {height}">\n' + STYLE + body + '</svg>\n')


def clip(rects, cx, cy, cw, ch):
    result = []
    for x, y, w, h, color in rects:
        x0, y0 = max(x, cx), max(y, cy)
        x1, y1 = min(x + w, cx + cw), min(y + h, cy + ch)
        if x1 > x0 and y1 > y0:
            result.append((x0, y0, x1 - x0, y1 - y0, color))
    return result


def bevel(x, y, w, h, light, shadow):
    return [(x, y, w, 1, light), (x, y, 1, h, light),
            (x, y + h - 1, w, 1, shadow), (x + w - 1, y, 1, h, shadow)]


def element_id(prefix, part):
    return f"{prefix}-{part}" if prefix else part


def frame(prefix, ox, oy, borders, drawing, C=20, M=20):
    """9-piece frame from a drawing made of rectangles.
    borders = (left, top, right, bottom); drawing(W, H) returns the rectangles."""
    left, top, right, bottom = borders
    W, H = left + C + right, top + M + bottom
    rects = drawing(W, H)
    areas = {
        "topleft": (0, 0, left, top), "top": (left, 0, C, top), "topright": (left + C, 0, right, top),
        "left": (0, top, left, M), "center": (left, top, C, M), "right": (left + C, top, right, M),
        "bottomleft": (0, top + M, left, bottom), "bottom": (left, top + M, C, bottom),
        "bottomright": (left + C, top + M, right, bottom),
    }
    out = ""
    for part, (x, y, w, h) in areas.items():
        out += f' <g id="{element_id(prefix, part)}">\n'
        out += rect(ox + x, oy + y, w, h, NONE)          # defines the exact size
        for rx, ry, rw, rh, color in clip(rects, x, y, w, h):
            if color != NONE:
                out += rect(ox + rx, oy + ry, rw, rh, color)
        out += ' </g>\n'
    return out


def hints(prefix, ox, oy, margins):
    """hint-*-margin rectangles: their size is the content margin."""
    top, right, bottom, left = margins
    p = (prefix + "-") if prefix else ""
    return (f'  <rect id="{p}hint-top-margin" x="{ox}" y="{oy}" width="1" height="{top}" fill="#ff00ff" fill-opacity="0"/>\n'
            f'  <rect id="{p}hint-bottom-margin" x="{ox + 2}" y="{oy}" width="1" height="{bottom}" fill="#ff00ff" fill-opacity="0"/>\n'
            f'  <rect id="{p}hint-left-margin" x="{ox}" y="{oy + 20}" width="{left}" height="1" fill="#ff00ff" fill-opacity="0"/>\n'
            f'  <rect id="{p}hint-right-margin" x="{ox}" y="{oy + 22}" width="{right}" height="1" fill="#ff00ff" fill-opacity="0"/>\n')


# ------------------------------------------------------------------ drawings

def openlook_window(W, H):
    """OPEN LOOK pop-up window: black outline and a 1 px relief."""
    return ([(0, 0, W, H, TEXT), (1, 1, W - 2, H - 2, BG)]
            + bevel(1, 1, W - 2, H - 2, LIGHT, DARK))


def raised(W, H):
    return [(0, 0, W, H, NONE)] + bevel(0, 0, W, H, LIGHT, DARK)


def sunken(W, H):
    return [(0, 0, W, H, PRESSED)] + bevel(0, 0, W, H, DARK, LIGHT)


def attention(W, H):
    """Item asking for attention: relief with a black outline."""
    return [(0, 0, W, H, TEXT), (1, 1, W - 2, H - 2, BG)] + bevel(1, 1, W - 2, H - 2, LIGHT, DARK)


def empty(W, H):
    return [(0, 0, W, H, NONE)]


def plain_line(W, H):
    return [(0, 0, W, H, NONE)] + [(0, 0, W, 1, DARK), (0, 0, 1, H, DARK),
                                   (0, H - 1, W, 1, DARK), (W - 1, 0, 1, H, DARK)]


def underline(width):
    def drawing(W, H):
        return [(0, 0, W, H, NONE), (0, H - width, W, width, TEXT)]
    return drawing


# ------------------------------------------------------------------ oblong button

R = 12   # radius of the rounded ends (corner pieces are R x R)
QUADRANT = {"topleft": (180, 270), "topright": (270, 360),
            "bottomright": (0, 90), "bottomleft": (90, 180)}


def point(cx, cy, r, angle):
    a = math.radians(angle)
    return (round(cx + r * math.cos(a), 4), round(cy + r * math.sin(a), 4))


def ring(cx, cy, r_out, r_in, corner, color, opacity=None):
    """Quarter ring between r_in and r_out (r_in = 0 gives a quarter disk)."""
    a0, a1 = QUADRANT[corner]
    x0, y0 = point(cx, cy, r_out, a0)
    x1, y1 = point(cx, cy, r_out, a1)
    d = f"M{x0},{y0} A{r_out},{r_out} 0 0 1 {x1},{y1} "
    if r_in > 0:
        x2, y2 = point(cx, cy, r_in, a1)
        x3, y3 = point(cx, cy, r_in, a0)
        d += f"L{x2},{y2} A{r_in},{r_in} 0 0 0 {x3},{y3} Z"
    else:
        d += f"L{cx},{cy} Z"
    op = f' fill-opacity="{opacity}"' if opacity is not None else ""
    return f'  <path d="{d}" {fill(color)}{op}/>\n'


def corner_centers(ox, oy, C, M):
    """Position (x, y) of each corner piece and the center of its arc."""
    x0, x2 = ox, ox + R + C
    y0, y2 = oy, oy + R + M
    return {"topleft": ((x0, y0), (x0 + R, y0 + R)), "topright": ((x2, y0), (x2, y0 + R)),
            "bottomleft": ((x0, y2), (x0 + R, y2)), "bottomright": ((x2, y2), (x2, y2))}


def piece(prefix, part, x, y, w, h, content=""):
    return f' <g id="{element_id(prefix, part)}">\n' + rect(x, y, w, h, NONE) + content + ' </g>\n'


def button(prefix, ox, oy, pressed, C=24, M=4):
    """Button with rounded ends, like OPEN LOOK buttons."""
    light, shadow = (SOFT_SHADOW, LIGHT) if pressed else (LIGHT, SOFT_SHADOW)
    background = PRESSED if pressed else BG
    x1, x2 = ox + R, ox + R + C
    y1, y2 = oy + R, oy + R + M
    out = ""
    for corner, ((px, py), (cx, cy)) in corner_centers(ox, oy, C, M).items():
        relief = light if corner.startswith("top") else shadow
        out += piece(prefix, corner, px, py, R, R,
                     ring(cx, cy, R, 0, corner, background)
                     + ring(cx, cy, R, R - 1, corner, DARK)
                     + ring(cx, cy, R - 1, R - 2, corner, relief))
    out += piece(prefix, "top", x1, oy, C, R, rect(x1, oy, C, R, background) + rect(x1, oy, C, 1, DARK) + rect(x1, oy + 1, C, 1, light))
    out += piece(prefix, "bottom", x1, y2, C, R, rect(x1, y2, C, R, background) + rect(x1, y2 + R - 2, C, 1, shadow) + rect(x1, y2 + R - 1, C, 1, DARK))
    out += piece(prefix, "left", ox, y1, R, M, rect(ox, y1, R, M, background) + rect(ox, y1, 1, M, DARK))
    out += piece(prefix, "right", x2, y1, R, M, rect(x2, y1, R, M, background) + rect(x2 + R - 1, y1, 1, M, DARK))
    out += piece(prefix, "center", x1, y1, C, M, rect(x1, y1, C, M, background))
    return out


def focus_ring(prefix, ox, oy, C=24, M=4):
    """Keyboard focus: a thin ring inside the button (painted over it)."""
    x1, x2 = ox + R, ox + R + C
    y1, y2 = oy + R, oy + R + M
    op = 0.7
    out = ""
    for corner, ((px, py), (cx, cy)) in corner_centers(ox, oy, C, M).items():
        out += piece(prefix, corner, px, py, R, R, ring(cx, cy, R - 3, R - 4, corner, TEXT, op))
    line = lambda x, y, w, h: f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" {fill(TEXT)} fill-opacity="{op}"/>\n'
    out += piece(prefix, "top", x1, oy, C, R, line(x1, oy + 3, C, 1))
    out += piece(prefix, "bottom", x1, y2, C, R, line(x1, y2 + R - 4, C, 1))
    out += piece(prefix, "left", ox, y1, R, M, line(ox + 3, y1, 1, M))
    out += piece(prefix, "right", x2, y1, R, M, line(x2 + R - 4, y1, 1, M))
    out += piece(prefix, "center", x1, y1, C, M)
    return out


# ------------------------------------------------------------------ files

def zero_insets(ox, oy):
    """Inset hints of (practically) zero size, as Breeze has. Without them,
    PlasmoidHeading (the header of pop-ups such as the volume applet) falls
    back to a path where its margins depend on its own borders, and Plasma
    logs a binding loop for leftInset."""
    tiny = "1e-08"
    return (f'  <rect id="hint-top-inset" x="{ox}" y="{oy}" width="4" height="{tiny}" fill="#00ff00" fill-opacity="0"/>\n'
            f'  <rect id="hint-bottom-inset" x="{ox + 6}" y="{oy}" width="4" height="{tiny}" fill="#00ff00" fill-opacity="0"/>\n'
            f'  <rect id="hint-left-inset" x="{ox}" y="{oy + 6}" width="{tiny}" height="4" fill="#00ff00" fill-opacity="0"/>\n'
            f'  <rect id="hint-right-inset" x="{ox + 6}" y="{oy + 6}" width="{tiny}" height="4" fill="#00ff00" fill-opacity="0"/>\n')


def window_file(margin, insets=False):
    body = frame("", 0, 0, (3, 3, 3, 3), openlook_window)
    body += hints("", 40, 0, (margin, margin, margin, margin))
    if insets:
        body += zero_insets(48, 20)
    return document(60, 30, body)


def panel_file():
    body = frame("", 0, 0, (3, 3, 3, 3), openlook_window)
    body += hints("", 40, 0, (3, 3, 3, 3))
    return document(60, 30, body)


def button_file():
    body = ""
    width = 2 * R + 24
    for i, (prefix, pressed) in enumerate([("normal", False), ("pressed", True),
                                           ("toolbutton-hover", False), ("toolbutton-pressed", True)]):
        body += button(prefix, i * (width + 10), 0, pressed)
        body += hints(prefix, i * (width + 10), 40, (4, 10, 4, 10))
    for i, prefix in enumerate(["focus", "toolbutton-focus"]):
        body += focus_ring(prefix, i * (width + 10), 80)
    return document(4 * (width + 10), 120, body)


def menubaritem_file():
    """Menu bar entries (global menu, window list): a raised box, sunken while
    the menu is open. Small borders on purpose: the window list sizes itself
    from these margins, and with the 12 px round ends of the buttons the
    Plasma panel kept resizing it in a loop (plasmashell at 200% CPU)."""
    body = ""
    for i, (prefix, drawing) in enumerate([("normal", raised), ("hover", raised), ("pressed", sunken)]):
        body += frame(prefix, i * 30, 0, (2, 2, 2, 2), drawing)
        body += hints(prefix, i * 30, 30, (3, 8, 3, 8))
    return document(90, 60, body)


def radiobutton_file():
    """Plasma radio buttons as OPEN LOOK exclusive settings: a raised
    rectangle, wider than tall; the chosen one is sunken, with a black
    outline and a black mark. All elements share one size so they overlay."""
    W, H = 18, 14
    body = ""

    def element(name, ox, content):
        return f' <g id="{name}">\n' + rect(ox, 0, W, H, NONE) + content + ' </g>\n'

    def relief(x, y, w, h, light, shadow):
        return "".join(rect(*r) for r in bevel(x, y, w, h, light, shadow))

    def outline(ox, color):
        return (rect(ox, 0, W, 1, color) + rect(ox, H - 1, W, 1, color)
                + rect(ox, 0, 1, H, color) + rect(ox + W - 1, 0, 1, H, color))

    normal = (rect(1, 1, W - 2, H - 2, BG) + outline(0, DARK)
              + relief(1, 1, W - 2, H - 2, LIGHT, DARK))
    body += element("normal", 0, normal)
    ox = W + 4
    checked = (rect(ox + 1, 1, W - 2, H - 2, PRESSED) + outline(ox, TEXT)
               + relief(ox + 1, 1, W - 2, H - 2, DARK, LIGHT)
               + rect(ox + 5, 5, W - 10, H - 10, TEXT))
    body += element("checked", ox, checked)
    ox = 2 * (W + 4)
    body += element("shadow", ox, "")
    ox = 3 * (W + 4)
    dots = "".join(rect(ox + x, 2, 1, 1, TEXT) + rect(ox + x, H - 3, 1, 1, TEXT)
                   for x in range(2, W - 2, 2))
    dots += "".join(rect(ox + 2, y, 1, 1, TEXT) + rect(ox + W - 3, y, 1, 1, TEXT)
                    for y in range(4, H - 3, 2))
    body += element("focus", ox, dots)
    ox = 4 * (W + 4)
    body += element("hover", ox, "")
    ox = 5 * (W + 4)
    body += f'  <rect id="hint-size" x="{ox}" y="0" width="{W}" height="{H}" {fill(NONE)}/>\n'
    return document(6 * (W + 4), H, body)


def switch_file():
    """OPEN LOOK had no switches. As in the GTK theme: a sunken bar, dark from
    the left up to the handle when on, and a raised block like the drag box
    of the sliders (black outline when the switch is on)."""
    def bar(fill_color, outline_color):
        def drawing(W, H):
            return ([(0, 0, W, H, outline_color), (1, 1, W - 2, H - 2, fill_color)]
                    + bevel(1, 1, W - 2, H - 2, SOFT_SHADOW if fill_color == PRESSED else TEXT, LIGHT))
        return drawing

    body = frame("inactive", 0, 0, (2, 2, 2, 2), bar(PRESSED, DARK))
    body += frame("active", 30, 0, (2, 2, 2, 2), bar(DARK, TEXT))
    body += '  <rect id="hint-stretch-borders" x="60" y="0" width="1" height="1" fill-opacity="0"/>\n'
    body += '  <rect id="hint-bar-size" x="0" y="30" width="32" height="12" fill-opacity="0"/>\n'

    HW, HH = 12, 18

    def handle(name, ox, outline_color, pressed=False, overlay=False):
        rects = [(0, 0, HW, HH, NONE)]
        if overlay:
            rects += [(0, 0, HW, 1, TEXT), (0, HH - 1, HW, 1, TEXT), (0, 0, 1, HH, TEXT), (HW - 1, 0, 1, HH, TEXT)]
        elif outline_color is not None:
            rects += [(0, 0, HW, HH, outline_color), (1, 1, HW - 2, HH - 2, PRESSED if pressed else BG)]
            rects += bevel(1, 1, HW - 2, HH - 2, DARK if pressed else LIGHT, LIGHT if pressed else DARK)
        out = f' <g id="{name}">\n'
        for x, y, w, h, color in rects:
            out += rect(ox + x, 50 + y, w, h, color)
        return out + ' </g>\n'

    body += handle("handle", 0, DARK)
    body += handle("handle-hover", 16, DARK)
    body += handle("handle-pressed", 32, DARK, pressed=True)
    body += handle("handle-active", 48, TEXT)
    body += handle("handle-focus", 64, None, overlay=True)
    body += handle("handle-shadow", 80, None)
    return document(100, 70, body)


def viewitem_file():
    body = frame("normal", 0, 0, (2, 2, 2, 2), empty)
    body += frame("hover", 30, 0, (2, 2, 2, 2), raised)
    body += frame("selected", 60, 0, (2, 2, 2, 2), sunken)
    body += frame("selected+hover", 90, 0, (2, 2, 2, 2), sunken)
    return document(120, 30, body)


def tasks_file():
    body = ""
    states = [("normal", empty), ("hover", raised), ("focus", sunken),
              ("attention", attention), ("minimized", empty), ("progress", sunken)]
    for i, (prefix, drawing) in enumerate(states):
        body += frame(prefix, i * 30, 0, (3, 3, 3, 3), drawing)
        body += hints(prefix, i * 30, 30, (4, 4, 4, 4))
    return document(len(states) * 30, 60, body)


def lineedit_file():
    body = frame("base", 0, 0, (3, 3, 3, 3), underline(1))
    body += frame("focus", 30, 0, (3, 3, 3, 3), underline(2))
    body += hints("base", 0, 30, (3, 3, 3, 3))
    body += hints("focus", 30, 30, (3, 3, 3, 3))
    body += '  <rect id="hint-focus-over-base" x="60" y="0" width="1" height="1" fill-opacity="0"/>\n'
    return document(70, 60, body)


def frame_file():
    body = frame("plain", 0, 0, (2, 2, 2, 2), plain_line)
    body += frame("raised", 30, 0, (2, 2, 2, 2), raised)
    body += frame("sunken", 60, 0, (2, 2, 2, 2),
                  lambda W, H: [(0, 0, W, H, NONE)] + bevel(0, 0, W, H, DARK, LIGHT))
    for i, p in enumerate(["plain", "raised", "sunken"]):
        body += hints(p, i * 30, 30, (2, 2, 2, 2))
    return document(90, 60, body)


def heading_file():
    """Pop-up header and footer: an etched separator, like the one under the title."""
    body = frame("header", 0, 0, (1, 1, 1, 2),
                 lambda W, H: [(0, 0, W, H, NONE), (0, H - 2, W, 1, DARK), (0, H - 1, W, 1, LIGHT)])
    body += frame("footer", 30, 0, (1, 2, 1, 1),
                  lambda W, H: [(0, 0, W, H, NONE), (0, 0, W, 1, DARK), (0, 1, W, 1, LIGHT)])
    body += hints("", 60, 0, (4, 6, 4, 6))
    body += '  <rect id="hint-stretch-borders" x="80" y="0" width="1" height="1" fill-opacity="0"/>\n'
    return document(90, 30, body)


def scrollbar_file():
    """Plasma has no way to put arrows inside the elevator: here it is a cable and a plain elevator."""
    E = 16

    def cable(vertical):
        def drawing(W, H):
            if vertical:
                return [(0, 0, W, H, NONE), (W // 2 - 1, 0, 3, H, SOFT_SHADOW)]
            return [(0, 0, W, H, NONE), (0, H // 2 - 1, W, 3, SOFT_SHADOW)]
        return drawing

    def elevator(W, H):
        return [(0, 0, W, H, DARK), (1, 1, W - 2, H - 2, BG)] + bevel(1, 1, W - 2, H - 2, LIGHT, SOFT_SHADOW)

    body = frame("background-vertical", 0, 0, (1, 1, 1, 1), cable(True), C=E - 2, M=20)
    body += frame("background-horizontal", 30, 0, (1, 1, 1, 1), cable(False), C=20, M=E - 2)
    body += frame("slider", 60, 0, (3, 3, 3, 3), elevator, C=E - 6, M=20)
    body += frame("mouseover-slider", 90, 0, (3, 3, 3, 3), elevator, C=E - 6, M=20)
    body += f'  <rect id="hint-scrollbar-size" x="120" y="0" width="{E}" height="{E}" fill-opacity="0"/>\n'
    return document(140, 40, body)


METADATA = {
    "KPackageStructure": "Plasma/Theme",
    "KPlugin": {
        "Authors": [{"Email": "saulo.popov.zambiasi@ufsc.br", "Name": "Saulo Popov Zambiasi"}],
        "Category": "",
        "Description": "Plasma style inspired by OPEN LOOK (Sun OpenWindows)",
        "Description[pt_BR]": "Estilo do Plasma inspirado no OPEN LOOK (Sun OpenWindows)",
        "EnabledByDefault": True,
        "Id": "FreeLook",
        "License": "GPL-3.0-or-later",
        "Name": "FreeLook",
        "Version": "0.7",
        "Website": ""
    },
    "X-Plasma-API": "5.0"
}

FILES = {
    "dialogs/background.svg": lambda: window_file(6, insets=True),
    "widgets/background.svg": lambda: window_file(8),
    "widgets/tooltip.svg": lambda: window_file(6),
    "widgets/panel-background.svg": panel_file,
    "widgets/button.svg": button_file,
    "widgets/menubaritem.svg": menubaritem_file,
    "widgets/radiobutton.svg": radiobutton_file,
    "widgets/switch.svg": switch_file,
    "widgets/viewitem.svg": viewitem_file,
    "widgets/tasks.svg": tasks_file,
    "widgets/lineedit.svg": lineedit_file,
    "widgets/frame.svg": frame_file,
    "widgets/plasmoidheading.svg": heading_file,
    "widgets/scrollbar.svg": scrollbar_file,
}


def main(target):
    for path, generate in FILES.items():
        destination = os.path.join(target, path)
        os.makedirs(os.path.dirname(destination), exist_ok=True)
        with open(destination, "w") as f:
            f.write(generate())
    with open(os.path.join(target, "metadata.json"), "w") as f:
        json.dump(METADATA, f, indent=4, ensure_ascii=False)
        f.write("\n")
    print("generated in", target)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "plasma/desktoptheme/FreeLook")
