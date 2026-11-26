#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the small SVG pictures used by the FreeLook GTK theme.

The shapes follow the Qt style (style/src/freelookstyle.cpp) so that GTK and
Qt applications look alike: the raised check box with a check mark that
overflows it, and the OPEN LOOK hollow triangles.

Usage: python3 tools/generate_gtk_assets.py
Writes into gtk/FreeLook/gtk-3.0/assets.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TARGETS = [ROOT / "gtk/FreeLook/gtk-3.0/assets"]

BASE = "#cccccc"
PRESSED = "#b7b7b7"
SOFT = "#a0a0a0"
DARK = "#7f7f7f"
LIGHT = "#ffffff"
INK = "#000000"


def svg(width, height, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}">\n{body}</svg>\n')


def check_box(state, enabled=True):
    """16x16: a 12x12 raised box at the bottom left; the check overflows it."""
    outline = DARK if enabled else SOFT
    ink = INK if enabled else SOFT
    side, top = 12, 4
    body = (
        f'  <rect x="0" y="{top}" width="{side}" height="{side}" fill="{BASE}"/>\n'
        # outer bevel (light top/left, dark bottom/right) then the outline
        f'  <rect x="0.5" y="{top + 0.5}" width="{side - 1}" height="{side - 1}" '
        f'fill="none" stroke="{outline}"/>\n'
        f'  <path d="M1.5 {top + side - 2} V{top + 1.5} H{side - 2}" fill="none" stroke="{LIGHT}"/>\n'
        f'  <path d="M1.5 {top + side - 1.5} H{side - 1.5} V{top + 1.5}" fill="none" stroke="{SOFT}"/>\n'
    )
    if state == "checked":
        body += (f'  <path d="M2.5 {top + side * 0.45:.2f} L{side * 0.45:.2f} {top + side - 1.5} '
                 f'L15.5 1" fill="none" stroke="{ink}" stroke-width="2.2" '
                 f'stroke-linecap="butt" stroke-linejoin="miter"/>\n')
    elif state == "mixed":
        body += f'  <rect x="3" y="{top + side // 2 - 1}" width="{side - 6}" height="2" fill="{ink}"/>\n'
    return svg(16, 16, body)


def triangle(direction, enabled=True, fill=BASE, size=(9, 6)):
    """OPEN LOOK hollow triangle, outlined through the middle of the pixels."""
    ink = INK if enabled else SOFT
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
        lines = (f'  <rect x="1" y="{ly}" width="10" height="1" fill="{SOFT}"/>\n'
                 f'  <rect x="1" y="{ly + 1}" width="10" height="1" fill="{LIGHT}"/>\n')
    else:
        w, h = 16, 12
        tx = 4 if direction == "left" else w - 4 - 5
        l, t, r, b = tx + 0.5, 2.5, tx + 4.5, 9.5
        cy = 6
        tri = [(r, t), (r, b), (l, cy)] if direction == "left" else [(l, t), (l, b), (r, cy)]
        lx = w - 2 if direction == "left" else 0
        lines = (f'  <rect x="{lx}" y="1" width="1" height="10" fill="{SOFT}"/>\n'
                 f'  <rect x="{lx + 1}" y="1" width="1" height="10" fill="{LIGHT}"/>\n')
    pts = " ".join(f"{x:g},{y:g}" for x, y in tri)
    return svg(w, h, f'  <polygon points="{pts}" fill="{BASE}" stroke="{INK}"/>\n' + lines)


def menu_button_arrow(fill=BASE):
    """28x16: the ▽ of a menu-bar entry, leaving room for the round end."""
    body = (f'  <polygon transform="translate(6 5)" points="0.5,0.5 8.5,0.5 4.5,5.5" '
            f'fill="{fill}" stroke="{INK}" stroke-linejoin="miter"/>\n')
    return svg(28, 16, body)


def main():
    files = {}
    for state in ("unchecked", "checked", "mixed"):
        files[f"check-{state}.svg"] = check_box(state)
        files[f"check-{state}-disabled.svg"] = check_box(state, enabled=False)
    for d in ("up", "down", "left", "right"):
        files[f"arrow-{d}.svg"] = triangle(d)
        files[f"arrow-{d}-pressed.svg"] = triangle(d, fill=PRESSED)
        files[f"arrow-{d}-disabled.svg"] = triangle(d, enabled=False)
        files[f"elevator-{d}.svg"] = elevator_end(d)
    files["menubar-arrow.svg"] = menu_button_arrow()
    files["menubar-arrow-pressed.svg"] = menu_button_arrow(PRESSED)
    for target in TARGETS:
        target.mkdir(parents=True, exist_ok=True)
        for name, content in files.items():
            (target / name).write_text(content)
    print(f"{len(files)} pictures written to {len(TARGETS)} folders")


if __name__ == "__main__":
    main()
