#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the FreeLook cursor theme.

Original drawings in the spirit of OPEN LOOK pointers (black shapes with a
white outline, a slim arrow pointing up and to the left, a stopwatch while
busy). Nothing is copied from Sun's bitmaps.

Writes cursors/FreeLook/ with:
  cursors_scalable/<name>/   SVG cursors for Plasma 6 (sharp at any size)
  cursors/<name>             Xcursor files for X11 and older programs
                             (needs xcursorgen and PyQt5 or PyQt6 to render)
and, for every other cursor name programs ask for, a symbolic link.

Usage: python3 tools/generate_cursors.py
"""
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
THEME = ROOT / "cursors" / "FreeLook"
CANVAS = 32            # SVG canvas; nominal size 24, as in Breeze
NOMINAL = 24
XSIZES = (24, 32, 48, 64)

INK = "#000000"
PAPER = "#ffffff"


def svg(body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{CANVAS}" height="{CANVAS}" '
            f'viewBox="0 0 {CANVAS} {CANVAS}">\n{body}</svg>\n')


def outlined(d, extra=""):
    """A filled black shape with a white outline (drawn under the fill)."""
    return (f'  <path d="{d}" fill="{PAPER}" stroke="{PAPER}" stroke-width="3" '
            f'stroke-linejoin="round"{extra}/>\n'
            f'  <path d="{d}" fill="{INK}"{extra}/>\n')


def lines(d, width=2, outline=4, cap="square", extra=""):
    """Black strokes with a white outline."""
    return (f'  <path d="{d}" fill="none" stroke="{PAPER}" stroke-width="{outline}" '
            f'stroke-linecap="{cap}" stroke-linejoin="round"{extra}/>\n'
            f'  <path d="{d}" fill="none" stroke="{INK}" stroke-width="{width}" '
            f'stroke-linecap="{cap}" stroke-linejoin="round"{extra}/>\n')


def poly(points):
    return "M" + " L".join(f"{x:.2f},{y:.2f}" for x, y in points) + " Z"


# ------------------------------------------------------------------ shapes

def arrow_path():
    """Slim arrow pointing up and to the left: a head and a straight shaft."""
    tip = (4.0, 4.0)
    head = 11.0                     # length of the head's sides
    half = 1.6                      # half width of the shaft
    end = (20.5, 20.5)
    base_mid = (tip[0] + head / 2, tip[1] + head / 2)
    n = (half / math.sqrt(2), -half / math.sqrt(2))
    return poly([tip, (tip[0] + head, tip[1]),
                 (base_mid[0] + n[0], base_mid[1] + n[1]),
                 (end[0] + n[0], end[1] + n[1]), (end[0] - n[0], end[1] - n[1]),
                 (base_mid[0] - n[0], base_mid[1] - n[1]),
                 (tip[0], tip[1] + head)])


def stopwatch(cx, cy, r, scale=1.0):
    """OPEN LOOK showed a stopwatch while busy: white face, crown, two hands."""
    s = scale
    body = ""
    # crown and stem
    body += outlined(poly([(cx - 2 * s, cy - r - 4.5 * s), (cx + 2 * s, cy - r - 4.5 * s),
                           (cx + 2 * s, cy - r - 2.5 * s), (cx + 0.8 * s, cy - r - 2.5 * s),
                           (cx + 0.8 * s, cy - r), (cx - 0.8 * s, cy - r),
                           (cx - 0.8 * s, cy - r - 2.5 * s), (cx - 2 * s, cy - r - 2.5 * s)]))
    body += (f'  <circle cx="{cx}" cy="{cy}" r="{r}" fill="{PAPER}" stroke="{PAPER}" stroke-width="{4 * s}"/>\n'
             f'  <circle cx="{cx}" cy="{cy}" r="{r}" fill="{PAPER}" stroke="{INK}" stroke-width="{1.8 * s}"/>\n')
    for k in range(4):
        a = math.radians(90 * k)
        x0, y0 = cx + (r - 2.2 * s) * math.sin(a), cy - (r - 2.2 * s) * math.cos(a)
        x1, y1 = cx + (r - 0.9 * s) * math.sin(a), cy - (r - 0.9 * s) * math.cos(a)
        body += f'  <path d="M{x0:.2f},{y0:.2f} L{x1:.2f},{y1:.2f}" stroke="{INK}" stroke-width="{1.2 * s}"/>\n'
    body += (f'  <path d="M{cx},{cy} L{cx},{cy - r + 2.5 * s} M{cx},{cy} L{cx + (r - 3.5 * s) * 0.8:.2f},{cy + (r - 3.5 * s) * 0.45:.2f}" '
             f'stroke="{INK}" stroke-width="{1.5 * s}" stroke-linecap="round" fill="none"/>\n')
    return body


def double_arrow(rotation):
    """Two-headed arrow, horizontal and then rotated around the center."""
    d = poly([(4, 16), (10, 10), (10, 14), (22, 14), (22, 10), (28, 16),
              (22, 22), (22, 18), (10, 18), (10, 22)])
    return outlined(d, f' transform="rotate({rotation} 16 16)"')


def four_way():
    h = poly([(4, 16), (9, 11), (9, 14.5), (14.5, 14.5), (14.5, 9), (11, 9), (16, 4),
              (21, 9), (17.5, 9), (17.5, 14.5), (23, 14.5), (23, 11), (28, 16),
              (23, 21), (23, 17.5), (17.5, 17.5), (17.5, 23), (21, 23), (16, 28),
              (11, 23), (14.5, 23), (14.5, 17.5), (9, 17.5), (9, 21)])
    return outlined(h)


DESIGNS = {
    # name: (svg body, hotspot x, hotspot y)
    "default": (outlined(arrow_path()), 4, 4),
    "text": (lines("M12,6 H20 M16,6 V26 M12,26 H20"), 16, 16),
    "wait": (stopwatch(16, 18, 9), 16, 18),
    "progress": (outlined(arrow_path()) + stopwatch(24, 24.5, 5.2, 0.6), 4, 4),
    "size_hor": (double_arrow(0), 16, 16),
    "size_ver": (double_arrow(90), 16, 16),
    "size_fdiag": (double_arrow(45), 16, 16),
    "size_bdiag": (double_arrow(-45), 16, 16),
    "fleur": (four_way(), 16, 16),
    "not-allowed": (f'  <circle cx="16" cy="16" r="9" fill="none" stroke="{PAPER}" stroke-width="5.5"/>\n'
                    f'  <circle cx="16" cy="16" r="9" fill="none" stroke="{INK}" stroke-width="2.5"/>\n'
                    + lines("M9.6,9.6 L22.4,22.4", width=2.5, outline=5.5, cap="butt"), 16, 16),
    "crosshair": (lines("M16,4 V28 M4,16 H28", width=1.4, outline=3.4, cap="butt"), 16, 16),
}

# Every cursor name Breeze provides, pointed at one of the designs above.
BASE_OF = {
    "alias": "default", "all-scroll": "fleur", "bottom_left_corner": "size_bdiag",
    "bottom_right_corner": "size_fdiag", "bottom_side": "size_ver", "cell": "crosshair",
    "center_ptr": "default", "col-resize": "size_hor", "color-picker": "crosshair",
    "context-menu": "default", "copy": "default", "dnd-move": "fleur", "dnd-no-drop": "not-allowed",
    "down-arrow": "default", "draft": "default", "help": "default", "left-arrow": "default",
    "left_side": "size_hor", "no-drop": "not-allowed", "openhand": "fleur", "pencil": "default",
    "pirate": "not-allowed", "pointer": "default", "right-arrow": "default", "right_ptr": "default",
    "right_side": "size_hor", "row-resize": "size_ver", "top_left_corner": "size_fdiag",
    "top_right_corner": "size_bdiag", "top_side": "size_ver", "up-arrow": "default",
    "vertical-text": "text", "wayland-cursor": "default", "x-cursor": "crosshair",
    "zoom-in": "default", "zoom-out": "default",
    # names that Breeze links to other names
    "00000000000000020006000e7e9ffc3f": "progress", "00008160000006810000408080010102": "size_ver",
    "03b6e0fcb3499374a867c041f52298f0": "not-allowed", "08e8e1c95fe2fc01f976f1e063a24ccd": "progress",
    "1081e37283d90000800003c07f3ef6bf": "default", "3085a0e285430894940527032f8b26df": "default",
    "3ecb610c1bf2410f44200f48c40d3599": "progress", "4498f0e0c1937ffe01fd06f973665830": "fleur",
    "5c6cd98b3f3ebcb1f9c7f1c204630408": "default", "6407b0e94181790501fd1e167b474872": "default",
    "640fb0e74195791501fd1ed57b41487f": "default", "9081237383d90e509aa00f00170e968f": "fleur",
    "9d800788f1b08800ae810202380a0822": "default", "a2a266d0498c3104214a47bd64ab0fc8": "default",
    "arrow": "default", "b66166c04f8c3109214a4fbd64a50fc8": "default", "circle": "not-allowed",
    "closedhand": "fleur", "cross": "crosshair", "crossed_circle": "not-allowed",
    "d9ce0ab605698f320427677b458ad60b": "default", "dnd-copy": "default", "dnd-none": "fleur",
    "e-resize": "size_hor", "e29285e634086352946a0e7090d73106": "default", "ew-resize": "size_hor",
    "fcf21c00b30f7e3f83fe0dfd12e71cff": "fleur", "forbidden": "not-allowed", "grab": "fleur",
    "grabbing": "fleur", "h_double_arrow": "size_hor", "half-busy": "progress", "hand1": "default",
    "hand2": "default", "ibeam": "text", "left_ptr": "default", "left_ptr_help": "default",
    "left_ptr_watch": "progress", "link": "default", "move": "fleur", "n-resize": "size_ver",
    "ne-resize": "size_bdiag", "nesw-resize": "size_bdiag", "ns-resize": "size_ver",
    "nw-resize": "size_fdiag", "nwse-resize": "size_fdiag", "plus": "crosshair",
    "pointing_hand": "default", "question_arrow": "default", "s-resize": "size_ver",
    "sb_h_double_arrow": "size_hor", "sb_v_double_arrow": "size_ver", "se-resize": "size_fdiag",
    "size-bdiag": "size_bdiag", "size-fdiag": "size_fdiag", "size-hor": "size_hor",
    "size-ver": "size_ver", "size_all": "fleur", "split_h": "size_hor", "split_v": "size_ver",
    "sw-resize": "size_bdiag", "tcross": "crosshair", "top_left_arrow": "default",
    "v_double_arrow": "size_ver", "w-resize": "size_hor", "watch": "wait", "whats_this": "default",
    "xterm": "text",
}

INDEX = """[Icon Theme]
Name=FreeLook
Name[pt_BR]=FreeLook
Comment=Cursors inspired by OPEN LOOK: black shapes with a white outline
Comment[pt_BR]=Cursores inspirados no OPEN LOOK: formas pretas com contorno branco
"""


def render_png(svg_text, size, path):
    """Render an SVG cursor to a size x size PNG with Qt."""
    try:
        from PyQt6.QtCore import QByteArray, QRectF
        from PyQt6.QtGui import QGuiApplication, QImage, QPainter
        from PyQt6.QtSvg import QSvgRenderer
        fmt = QImage.Format.Format_ARGB32
        transparent = 0
    except ImportError:
        from PyQt5.QtCore import QByteArray, QRectF
        from PyQt5.QtGui import QGuiApplication, QImage, QPainter
        from PyQt5.QtSvg import QSvgRenderer
        fmt = QImage.Format_ARGB32
        transparent = 0
    if QGuiApplication.instance() is None:
        render_png.app = QGuiApplication(sys.argv[:1])
    image = QImage(size, size, fmt)
    image.fill(transparent)
    painter = QPainter(image)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing if hasattr(QPainter, "RenderHint")
                          else QPainter.Antialiasing)
    QSvgRenderer(QByteArray(svg_text.encode())).render(painter, QRectF(0, 0, size, size))
    painter.end()
    image.save(str(path))


def link(target, name, folder):
    path = folder / name
    if path.is_symlink() or path.exists():
        if path.is_dir() and not path.is_symlink():
            shutil.rmtree(path)
        else:
            path.unlink()
    os.symlink(target, path)


def main():
    if THEME.exists():
        shutil.rmtree(THEME)
    scalable = THEME / "cursors_scalable"
    xcursors = THEME / "cursors"
    scalable.mkdir(parents=True)
    xcursors.mkdir(parents=True)
    (THEME / "index.theme").write_text(INDEX)

    have_xcursorgen = shutil.which("xcursorgen") is not None
    if not have_xcursorgen:
        print("xcursorgen not found: only the SVG (Plasma 6) cursors are written", file=sys.stderr)

    with tempfile.TemporaryDirectory() as tmp:
        for name, (body, hx, hy) in DESIGNS.items():
            text = svg(body)
            folder = scalable / name
            folder.mkdir()
            (folder / f"{name}.svg").write_text(text)
            (folder / "metadata.json").write_text(json.dumps(
                [{"filename": f"{name}.svg", "hotspot_x": hx, "hotspot_y": hy,
                  "nominal_size": NOMINAL}], indent=4) + "\n")
            if have_xcursorgen:
                config = []
                for size in XSIZES:
                    pixels = round(size * CANVAS / NOMINAL)
                    png = Path(tmp) / f"{name}-{size}.png"
                    render_png(text, pixels, png)
                    scale = pixels / CANVAS
                    config.append(f"{size} {round(hx * scale)} {round(hy * scale)} {png}")
                cfg = Path(tmp) / f"{name}.cfg"
                cfg.write_text("\n".join(config) + "\n")
                subprocess.run(["xcursorgen", str(cfg), str(xcursors / name)], check=True)

    for name, base in sorted(BASE_OF.items()):
        if name in DESIGNS:
            continue
        link(base, name, scalable)
        if have_xcursorgen:
            link(base, name, xcursors)
    print(f"{len(DESIGNS)} cursors and {len(BASE_OF)} names written to {THEME}")


if __name__ == "__main__":
    main()
