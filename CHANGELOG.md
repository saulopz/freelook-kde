# Changelog

FreeLook was built in a single long session on 2026-11-25, in small
versions, each one tested on a real Plasma 6.8 session before the next.

## 0.8 (in progress)

- Tabs drawn as an OPEN LOOK exclusive setting: raised rectangles side by
  side sharing a single line, the current one sunken with a black outline,
  and the page in a plain frame just below. (OPEN LOOK itself had no tabs;
  property windows used a "Category" menu button.)
- Tab close button: a thin black "x" without color, which gets a raised box
  under the mouse and sinks while pressed.

## 0.7

- Global theme (`org.freelook.desktop`): applies the color scheme, the
  application style, the Plasma style, the window decoration, the title bar
  button layout and the border size in one step.
- Wallpaper package with the OpenWindows blue (`#40a0c0`).
- Code base translated to English; Portuguese kept for user-facing names
  (`pt_BR` translations) and in `LEIAME.md`.
- `install.sh` (builds the style and installs it only if its checks pass),
  `uninstall.sh` (switches back to Breeze first if FreeLook is in use) and
  `clean.sh`.
- The first SVG decoration moved to `extras/aurorae-classic`.

## 0.6

- Plasma style: panel, menus, tooltips and widgets with the OPEN LOOK
  pop-up frame, oblong buttons, underlined search field, sunken selection.
- Corner arcs drawn as filled rings, so the 9-piece frames line up without
  seams.

## 0.5

- Underlined text fields, without a box; the line gets thicker on focus.

## 0.4

- Application style in C++ (a Qt 6 `QStyle` on top of Fusion): oblong
  buttons with a default ring, hollow arrows, check marks that overflow
  their box, rectangular exclusive settings and the elevator scroll bar,
  with the arrows inside the elevator and anchors at both ends.
- Test gallery that simulates clicks on the elevator.
- 0.4.1 and 0.4.2: the elevator stays gray in Kirigami QML lists, whose
  local palette is all white.

## 0.3

- Title bar buttons follow the title font size.
- 0.3.1: glyphs back to thin 1 px strokes, snapped to the pixel grid.

## 0.2

- Window decoration rewritten in QML, which allows the real OPEN LOOK
  L-shaped corners: as thick as the border, with longer arms, raised and
  lit from the top left.
- The window menu button behaves like in olwm: left click minimizes,
  right click opens the window menu.
- QML test harness that runs the decoration outside KWin.

## 0.1

- First window decoration, as an SVG Aurorae theme, and the color scheme
  measured from an OpenWindows 3 screenshot.

## Before 0.1

- Saulo's first attempt: a Kvantum theme also called FreeLook, mixing
  OPEN LOOK and CDE. It showed that the theme needed a single, consistent
  design language, which became the starting point of this project.
