#!/usr/bin/env python3
"""
FreeLook classic: the first FreeLook window decoration, as an SVG Aurorae theme.

It is kept as an extra: it is lighter than the QML decoration, but the SVG
format cannot draw the long OPEN LOOK L-shaped corners (each corner piece
must be exactly as large as the border). It is not installed by default.

    python3 generate.py FreeLook
    cp -r FreeLook ~/.local/share/aurorae/themes/

Everything is vector "pixel art": 1 px rectangles aligned to the grid.

SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
SPDX-License-Identifier: GPL-3.0-or-later
"""
import argparse
import os

parser = argparse.ArgumentParser(description="Generates the FreeLook classic Aurorae theme")
parser.add_argument("target", nargs="?", default="FreeLook")
parser.add_argument("--outer", type=int, default=5,
                    help="transparent band holding the corners (px); the corner is outer+frame")
parser.add_argument("--frame", type=int, default=5, help="visible frame thickness (px)")
ARGS = parser.parse_args()

# ---------------------------------------------------------------- palette
# Colors measured on an OpenWindows 3 screenshot (Sun, 1991).
BASE = "#cccccc"          # BG1: window and button background
PRESSED = "#b7b7b7"       # BG2: pressed / selected button
SOFT_SHADOW = "#a0a0a0"   # BG3: soft shadow
DARK = "#808080"          # separator line and button shadow
LIGHT = "#ffffff"         # 3D highlight
BLACK = "#000000"

# ---------------------------------------------------------------- metrics (px)
OUTER = ARGS.outer
FRAME = ARGS.frame
BORDER = OUTER + FRAME                # BorderLeft/Right/Bottom
TITLE_TOP = OUTER + 4                 # TitleEdgeTop: band + black + highlight + 2 gray
TITLE_HEIGHT = 18                     # TitleHeight
TITLE_BOTTOM = 5                      # TitleEdgeBottom (holds the separator)
TOP = TITLE_TOP + TITLE_HEIGHT + TITLE_BOTTOM
CORNER = BORDER                       # corner size (limited to the border by Aurorae)
BUTTON = 18                           # button width and height
MAX_TOP, MAX_BOTTOM = 3, 5            # title margins on maximized windows


def svg(width, height, body):
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" version="1.1" '
            f'width="{width}" height="{height}" viewBox="0 0 {width} {height}" '
            'shape-rendering="crispEdges">\n' + body + '</svg>\n')


def rect(x, y, w, h, color):
    return f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{color}"/>\n'


def invisible_box(x, y, w, h):
    # Defines the exact element size, even where it is transparent.
    return f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#000" fill-opacity="0"/>\n'


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


# ---------------------------------------------------------------- window frame
def frame_drawing(W, H, active):
    r = []
    # Corners (drawn first; the frame covers their inner part).
    for cx, cy in ((0, 0), (W - CORNER, 0), (0, H - CORNER), (W - CORNER, H - CORNER)):
        r.append((cx, cy, CORNER, CORNER, BLACK))
        r.append((cx + 1, cy + 1, CORNER - 2, CORNER - 2, BASE))
        r += bevel(cx + 1, cy + 1, CORNER - 2, CORNER - 2, LIGHT, SOFT_SHADOW)
    # Frame: black outline, gray background and 3D bevel.
    e = OUTER
    r.append((e, e, W - 2 * e, H - 2 * e, BLACK))
    r.append((e + 1, e + 1, W - 2 * e - 2, H - 2 * e - 2, BASE))
    r += bevel(e + 1, e + 1, W - 2 * e - 2, H - 2 * e - 2, LIGHT, SOFT_SHADOW)
    # Thin line around the application area.
    r.append((BORDER - 1, TOP - 1, W - 2 * BORDER + 2, 1, SOFT_SHADOW))
    r.append((BORDER - 1, H - BORDER, W - 2 * BORDER + 2, 1, SOFT_SHADOW))
    r.append((BORDER - 1, TOP - 1, 1, H - TOP - BORDER + 2, SOFT_SHADOW))
    r.append((W - BORDER, TOP - 1, 1, H - TOP - BORDER + 2, SOFT_SHADOW))
    # Separator under the title (etched on the active window).
    y = TITLE_TOP + TITLE_HEIGHT
    if active:
        r.append((BORDER, y, W - 2 * BORDER, 1, DARK))
        r.append((BORDER, y + 1, W - 2 * BORDER, 1, LIGHT))
    else:
        r.append((BORDER, y, W - 2 * BORDER, 1, SOFT_SHADOW))
    # Center background.
    r.append((BORDER, TOP, W - 2 * BORDER, H - TOP - BORDER, BASE))
    return r


def frame_pieces(prefix, ox, oy, active, C=40, M=30):
    """The 9 pieces with Aurorae's ids, placed at (ox, oy)."""
    W, H = BORDER + C + BORDER, TOP + M + BORDER
    drawing = frame_drawing(W, H, active)
    areas = {
        "topleft": (0, 0, BORDER, TOP), "top": (BORDER, 0, C, TOP),
        "topright": (BORDER + C, 0, BORDER, TOP),
        "left": (0, TOP, BORDER, M), "center": (BORDER, TOP, C, M),
        "right": (BORDER + C, TOP, BORDER, M),
        "bottomleft": (0, TOP + M, BORDER, BORDER), "bottom": (BORDER, TOP + M, C, BORDER),
        "bottomright": (BORDER + C, TOP + M, BORDER, BORDER),
    }
    out = ""
    for name, (x, y, w, h) in areas.items():
        out += f' <g id="{prefix}-{name}">\n' + invisible_box(ox + x, oy + y, w, h)
        for rx, ry, rw, rh, color in clip(drawing, x, y, w, h):
            out += rect(ox + rx, oy + ry, rw, rh, color)
        out += ' </g>\n'
    return out


def maximized_piece(prefix, ox, oy, active, C=40):
    h = MAX_TOP + TITLE_HEIGHT + MAX_BOTTOM
    y = MAX_TOP + TITLE_HEIGHT
    r = [(0, 0, C, h, BASE), (0, h - 1, C, 1, SOFT_SHADOW)]
    if active:
        r += [(0, y, C, 1, DARK), (0, y + 1, C, 1, LIGHT)]
    else:
        r += [(0, y, C, 1, SOFT_SHADOW)]
    out = f' <g id="{prefix}-center">\n' + invisible_box(ox, oy, C, h)
    for rx, ry, rw, rh, color in r:
        out += rect(ox + rx, oy + ry, rw, rh, color)
    return out + ' </g>\n'


def decoration():
    body = frame_pieces("decoration", 0, 0, True)
    body += frame_pieces("decoration-inactive", 100, 0, False)
    body += maximized_piece("decoration-maximized", 0, 120, True)
    body += maximized_piece("decoration-maximized-inactive", 100, 120, False)
    return svg(200, 160, body)


# ---------------------------------------------------------------- buttons
# OPEN LOOK 3D box: 14x13 px centered in the 18x18 button.
BX, BY, BW, BH = 2, 3, 14, 13


def box(pressed):
    if pressed:
        r = [(BX, BY, BW, BH, PRESSED)] + bevel(BX, BY, BW, BH, DARK, LIGHT)
    else:
        r = [(BX, BY, BW, BH, BASE)] + bevel(BX, BY, BW, BH, LIGHT, DARK)
    return "".join(rect(*a) for a in r)


def glyph(name, color, background, d):
    """Drawing of each button. d = 1 px shift while pressed."""
    s = f'stroke="{color}" stroke-width="1" fill="none" shape-rendering="geometricPrecision"'
    if name == "minimize":     # window menu button: hollow triangle
        return (f'  <polygon points="{5.5+d},{6.5+d} {12.5+d},{6.5+d} {9+d},{11.5+d}" '
                f'fill="{background}" stroke="{color}" stroke-width="1" '
                'stroke-linejoin="miter" shape-rendering="geometricPrecision"/>\n')
    if name == "close":
        return (f'  <path d="M{6+d},{6.5+d} L{12+d},{12.5+d} M{12+d},{6.5+d} L{6+d},{12.5+d}" '
                f'stroke="{color}" stroke-width="1.4" fill="none" '
                'shape-rendering="geometricPrecision"/>\n')
    if name == "maximize":
        return (rect(5 + d, 6 + d, 8, 1, color) +
                f'  <rect x="{5.5+d}" y="{6.5+d}" width="7" height="6" {s}/>\n')
    if name == "restore":
        return (f'  <rect x="{7.5+d}" y="{5.5+d}" width="5" height="4" {s}/>\n'
                f'  <rect x="{5.5+d}" y="{8.5+d}" width="5" height="4" fill="{background}" '
                f'stroke="{color}" stroke-width="1" shape-rendering="geometricPrecision"/>\n')
    if name == "shade":
        return rect(5 + d, 6 + d, 8, 2, color)
    if name == "keepbelow":
        return rect(5 + d, 11 + d, 8, 2, color)
    if name == "alldesktops":
        return "".join(rect(x + d, y + d, 2, 2, color) for x in (6, 10) for y in (6, 10))
    if name == "help":
        return (f'  <path d="M{6.5+d},{7.5+d} C{6.5+d},{4.8+d} {11.5+d},{4.8+d} {11.5+d},{7.5+d} '
                f'C{11.5+d},{9.2+d} {9+d},{9.2+d} {9+d},{11+d}" {s}/>\n' +
                rect(8 + d, 12 + d, 2, 2, color))
    raise ValueError(name)


def pushpin(pinned, color):
    """The OPEN LOOK pushpin: lying down (loose) or head-on (pinned)."""
    g = 'shape-rendering="geometricPrecision"'
    if not pinned:
        return (rect(1, 9, 7, 1, color) +                                  # needle
                rect(7, 7, 3, 5, BLACK) + rect(8, 8, 1, 3, BASE) +         # collar
                rect(10, 8, 3, 3, BLACK) + rect(10, 9, 3, 1, DARK) +       # shaft
                f'  <circle cx="14.5" cy="9.5" r="2.8" fill="{BASE}" stroke="{color}" stroke-width="1" {g}/>\n'
                f'  <circle cx="13.8" cy="8.7" r="0.9" fill="{LIGHT}" {g}/>\n')
    return (f'  <ellipse cx="10" cy="11" rx="4.3" ry="3.6" fill="{SOFT_SHADOW}" {g}/>\n'
            f'  <circle cx="9" cy="9.5" r="3.8" fill="{BASE}" stroke="{color}" stroke-width="1" {g}/>\n'
            f'  <circle cx="9" cy="9.5" r="1.4" fill="{color}" {g}/>\n'
            f'  <circle cx="7.6" cy="8.1" r="0.9" fill="{LIGHT}" {g}/>\n')


def button(name):
    states = {"active": (False, BLACK), "pressed": (True, BLACK),
              "deactivated": (False, SOFT_SHADOW)}
    body = ""
    # States overlap at (0,0): Plasma measures each element without applying
    # the group's transform, so no translate is used.
    for state, (pressed, color) in states.items():
        body += f' <g id="{state}-center">\n'
        body += invisible_box(0, 0, BUTTON, BUTTON)
        if name == "keepabove":
            body += pushpin(pressed, color)
        else:
            body += box(pressed)
            body += glyph(name, color, PRESSED if pressed else BASE, 1 if pressed else 0)
        body += ' </g>\n'
    return svg(BUTTON, BUTTON, body)


BUTTONS = ["minimize", "close", "maximize", "restore", "keepabove",
           "keepbelow", "alldesktops", "shade", "help"]


# ---------------------------------------------------------------- text files
def rc():
    return f"""[General]
ActiveTextColor=0,0,0
InactiveTextColor=96,96,96
TitleAlignment=Center
TitleVerticalAlignment=Center
Animation=0
Shadow=false
UseTextShadow=false
LeftButtons=I
RightButtons=FX

[Layout]
BorderLeft={BORDER}
BorderRight={BORDER}
BorderBottom={BORDER}
TitleEdgeTop={TITLE_TOP}
TitleEdgeBottom={TITLE_BOTTOM}
TitleEdgeLeft={BORDER + 3}
TitleEdgeRight={BORDER + 3}
TitleEdgeTopMaximized={MAX_TOP}
TitleEdgeBottomMaximized={MAX_BOTTOM}
TitleEdgeLeftMaximized=3
TitleEdgeRightMaximized=3
TitleBorderLeft=6
TitleBorderRight=6
TitleHeight={TITLE_HEIGHT}
ButtonWidth={BUTTON}
ButtonHeight={BUTTON}
ButtonSpacing=2
ButtonMarginTop=0
ExplicitButtonSpacer=10
PaddingTop=0
PaddingBottom=0
PaddingLeft=0
PaddingRight=0
"""


METADATA = """[Desktop Entry]
Name=FreeLook classic (SVG)
Name[pt_BR]=FreeLook clássico (SVG)
Comment=Window decoration inspired by OPEN LOOK (Sun OpenWindows)
Comment[pt_BR]=Decoração de janela inspirada no OPEN LOOK (Sun OpenWindows)
X-KDE-PluginInfo-Author=Saulo Popov Zambiasi
X-KDE-PluginInfo-Email=saulo.popov.zambiasi@ufsc.br
X-KDE-PluginInfo-Name=FreeLook
X-KDE-PluginInfo-Version=0.7
X-KDE-PluginInfo-License=GPL-3.0-or-later
X-KDE-PluginInfo-Category=
X-KDE-PluginInfo-Website=
"""


def main(target):
    os.makedirs(target, exist_ok=True)

    def write(name, content):
        with open(os.path.join(target, name), "w") as f:
            f.write(content)

    write("decoration.svg", decoration())
    for b in BUTTONS:
        write(f"{b}.svg", button(b))
    write("FreeLookrc", rc())
    write("metadata.desktop", METADATA)
    print("generated in", target)


if __name__ == "__main__":
    main(ARGS.target)
