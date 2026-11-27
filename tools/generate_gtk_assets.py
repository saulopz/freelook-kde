#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the small SVG pictures used by the FreeLook GTK theme.

The shapes follow the Qt style (style/src/freelookstyle.cpp) so that GTK and
Qt applications look alike: the raised check box with a check mark that
overflows it, and the OPEN LOOK hollow triangles.

It also writes gtk-dark.css, the FreeLook Night variant that GTK 3 loads when
Plasma asks for a dark theme: a copy of gtk.css with the night palette and
the night pictures (assets-dark/). Edit gtk.css, never gtk-dark.css.

Usage: python3 tools/generate_gtk_assets.py
Writes into gtk/FreeLook/gtk-3.0.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GTK3 = ROOT / "gtk/FreeLook/gtk-3.0"

# The pictures use these six colors. FreeLook Night follows the Qt style on a
# dark palette (style/src/freelookstyle.cpp): relief from the base color.
DAY = dict(BASE="#cccccc", PRESSED="#b7b7b7", SOFT="#a0a0a0", DARK="#7f7f7f",
           LIGHT="#ffffff", INK="#000000")
NIGHT = dict(BASE="#423b33", PRESSED="#3b352e", SOFT="#342e28", DARK="#1a1814",
             LIGHT="#63594d", INK="#eee0c8")

# The gtk.css palette block (@define-color fl_*) for FreeLook Night. The
# colors come from color-schemes/FreeLookNight.colors.
NIGHT_CSS = {
    "fl_bg": NIGHT["BASE"],
    "fl_pressed": NIGHT["PRESSED"],
    "fl_soft": NIGHT["SOFT"],
    "fl_dark": NIGHT["DARK"],
    "fl_light": NIGHT["LIGHT"],
    "fl_ink": NIGHT["INK"],
    "fl_grayed": "#8a7e6c",
    "fl_view": "#2e2924",
    "fl_view_alt": "#342f29",
    "fl_selected": "#6b5d4c",
    "fl_tooltip": "#574d42",
    "fl_link": "#e8b860",
    "fl_dim": "#b0a28c",
    "fl_info": "#4a4238",
    "fl_info_warning": "#5a4a28",
    "fl_info_error": "#5a3028",
    "fl_warning": "#e8c070",
    "fl_error": "#f0907a",
    "fl_success": "#a8d08c",
}


class C:
    """The palette in use while the pictures are drawn."""


def svg(width, height, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}">\n{body}</svg>\n')


def check_box(state, enabled=True):
    """16x16: a 12x12 raised box at the bottom left; the check overflows it."""
    outline = C.DARK if enabled else C.SOFT
    ink = C.INK if enabled else C.SOFT
    side, top = 12, 4
    body = (
        f'  <rect x="0" y="{top}" width="{side}" height="{side}" fill="{C.BASE}"/>\n'
        # outer bevel (light top/left, dark bottom/right) then the outline
        f'  <rect x="0.5" y="{top + 0.5}" width="{side - 1}" height="{side - 1}" '
        f'fill="none" stroke="{outline}"/>\n'
        f'  <path d="M1.5 {top + side - 2} V{top + 1.5} H{side - 2}" fill="none" stroke="{C.LIGHT}"/>\n'
        f'  <path d="M1.5 {top + side - 1.5} H{side - 1.5} V{top + 1.5}" fill="none" stroke="{C.SOFT}"/>\n'
    )
    if state == "checked":
        body += (f'  <path d="M2.5 {top + side * 0.45:.2f} L{side * 0.45:.2f} {top + side - 1.5} '
                 f'L15.5 1" fill="none" stroke="{ink}" stroke-width="2.2" '
                 f'stroke-linecap="butt" stroke-linejoin="miter"/>\n')
    elif state == "mixed":
        body += f'  <rect x="3" y="{top + side // 2 - 1}" width="{side - 6}" height="2" fill="{ink}"/>\n'
    return svg(16, 16, body)


def triangle(direction, enabled=True, fill=None, size=(9, 6)):
    """OPEN LOOK hollow triangle, outlined through the middle of the pixels."""
    fill = fill or C.BASE
    ink = C.INK if enabled else C.SOFT
    w, h = size if direction in ("up", "down") else (size[1], size[0])
    l, t, r, b = 0.5, 0.5, w - 0.5, h - 0.5
    cx, cy = w / 2, h / 2
    points = {
        "up": [(l, b), (r, b), (cx, t)],
        "down": [(l, t), (r, t), (cx, b)],
        "left": [(r, t), (r, b), (l, cy)],
        "right": [(l, t), (l, b), (r, cy)],
    }[direction]
    pts = " ".join(f"{x:g},{y:g}" for x, y in points)
    # Centered in a 16x16 icon.
    ox, oy = (16 - w) // 2, (16 - h) // 2
    return svg(16, 16, f'  <polygon transform="translate({ox} {oy})" points="{pts}" '
                       f'fill="{fill}" stroke="{ink}" stroke-linejoin="miter"/>\n')


def elevator_end(direction):
    """One end of the scroll bar elevator: a small hollow triangle and an
    etched line that separates it from the middle of the elevator.
    Vertical ends are 12x16, horizontal ones 16x12; the triangle sits 4 px
    from the outer edge (GTK 3 cannot offset a background from the bottom)."""
    tri = []
    if direction in ("up", "down"):
        w, h = 12, 16
        ty = 4 if direction == "up" else h - 4 - 5
        l, t, r, b = 2.5, ty + 0.5, 9.5, ty + 4.5
        cx = 6
        tri = [(l, b), (r, b), (cx, t)] if direction == "up" else [(l, t), (r, t), (cx, b)]
        ly = h - 2 if direction == "up" else 0
        lines = (f'  <rect x="1" y="{ly}" width="10" height="1" fill="{C.SOFT}"/>\n'
                 f'  <rect x="1" y="{ly + 1}" width="10" height="1" fill="{C.LIGHT}"/>\n')
    else:
        w, h = 16, 12
        tx = 4 if direction == "left" else w - 4 - 5
        l, t, r, b = tx + 0.5, 2.5, tx + 4.5, 9.5
        cy = 6
        tri = [(r, t), (r, b), (l, cy)] if direction == "left" else [(l, t), (l, b), (r, cy)]
        lx = w - 2 if direction == "left" else 0
        lines = (f'  <rect x="{lx}" y="1" width="1" height="10" fill="{C.SOFT}"/>\n'
                 f'  <rect x="{lx + 1}" y="1" width="1" height="10" fill="{C.LIGHT}"/>\n')
    pts = " ".join(f"{x:g},{y:g}" for x, y in tri)
    return svg(w, h, f'  <polygon points="{pts}" fill="{C.BASE}" stroke="{C.INK}"/>\n' + lines)


def menu_button_arrow(fill=None):
    """28x16: the ▽ of a menu-bar entry, leaving room for the round end."""
    fill = fill or C.BASE
    body = (f'  <polygon transform="translate(6 5)" points="0.5,0.5 8.5,0.5 4.5,5.5" '
            f'fill="{fill}" stroke="{C.INK}" stroke-linejoin="miter"/>\n')
    return svg(28, 16, body)


def pictures(palette):
    for key, value in palette.items():
        setattr(C, key, value)
    files = {}
    for state in ("unchecked", "checked", "mixed"):
        files[f"check-{state}.svg"] = check_box(state)
        files[f"check-{state}-disabled.svg"] = check_box(state, enabled=False)
    for d in ("up", "down", "left", "right"):
        files[f"arrow-{d}.svg"] = triangle(d)
        files[f"arrow-{d}-pressed.svg"] = triangle(d, fill=C.PRESSED)
        files[f"arrow-{d}-disabled.svg"] = triangle(d, enabled=False)
        files[f"elevator-{d}.svg"] = elevator_end(d)
    files["menubar-arrow.svg"] = menu_button_arrow()
    files["menubar-arrow-pressed.svg"] = menu_button_arrow(C.PRESSED)
    return files


def night_css(css):
    """gtk-dark.css: gtk.css with the night palette and pictures."""
    for name, value in NIGHT_CSS.items():
        css, n = re.subn(rf"(@define-color {name}) #[0-9a-fA-F]{{6}};", rf"\1 {value};", css)
        if n != 1:
            raise SystemExit(f"gtk.css: @define-color {name} not found exactly once")
    css = css.replace('url("assets/', 'url("assets-dark/')
    note = ("/* GENERATED by tools/generate_gtk_assets.py from gtk.css: do not edit.\n"
            " * FreeLook Night, loaded by GTK 3 when a dark theme is preferred. */\n\n")
    return note + css


def main():
    count = 0
    for folder, palette in (("assets", DAY), ("assets-dark", NIGHT)):
        target = GTK3 / folder
        target.mkdir(parents=True, exist_ok=True)
        for name, content in pictures(palette).items():
            (target / name).write_text(content)
            count += 1
    (GTK3 / "gtk-dark.css").write_text(night_css((GTK3 / "gtk.css").read_text()))
    print(f"{count} pictures and gtk-dark.css written to {GTK3}")


if __name__ == "__main__":
    main()
