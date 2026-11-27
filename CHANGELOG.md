# Changelog

FreeLook was built in a single long session on 2026-11-25, in small
versions, each one tested on a real Plasma 6.8 session before the next.

## 0.10 (in progress)

- Window decoration: the window with the focus has its title framed between
  two black lines, each with a light line on the inside, as in OpenWindows;
  other windows keep the title loose. The title bar is 2 px taller to give
  the buttons some air inside that band.
- Application style: icon-only (square) buttons are boxes with slightly
  rounded corners, lit on the top and left, instead of circles.
- Plasma style: the dialog background declares (zero) insets, as Breeze
  does; without them the header of pop-ups such as the volume applet made
  Plasma log a binding loop.
- Plasma style: radio buttons of Plasma widgets (the output device in the
  volume pop-up, for instance) are OPEN LOOK exclusive settings, a raised
  rectangle that sinks with a black mark when chosen, instead of Breeze's
  round ones.
- Plasma style: switches as in the GTK theme, a sunken bar that turns dark
  when on and a raised block like the drag box of the sliders.
- Cursor theme: original pointers in the OPEN LOOK spirit, black shapes with a
  white outline (a slim arrow, an I-beam, a stopwatch while busy, two-headed
  arrows for resizing, a four-way arrow for moving). SVG cursors for Plasma 6
  plus Xcursor files for X11 programs, installed in `~/.icons` (where
  Plasma's pointer settings look); the global theme sets them.
- FreeLook Night, first part: an amber color scheme ("retro terminal at
  night", base #423b33 with cream text), the application style and the
  window decoration now derive their light relief from the base color on dark
  palettes, and the wallpaper has a night variant (a deep teal gradient) that
  Plasma picks when the color scheme is dark.
- FreeLook Night, second part: a night Plasma style (the same drawings with a
  soft amber relief and black outlines, instead of white relief and cream
  outlines), and the FreeLook Night global theme. In *Global Theme >
  Configure* (or the brightness and color applet), choose FreeLook as the
  light theme and FreeLook Night as the dark one to switch between them. The
  "Workspace" button picture follows the color scheme where Plasma recolors
  it. The GTK theme comes next.
- Window decoration: the buttons redraw when the colors change, so switching
  between FreeLook and FreeLook Night keeps them right on open windows.
- Experimental panels: `top-windows-kickoff`, the same top bar with Kickoff,
  whose "Workspace" text label follows the color scheme.
- Experimental panels: the "Workspace ▽" button has its own colors, the deep
  teal of the night wallpaper with cream text, since Kicker does not recolor
  its picture. It stands out on the day and the night panels alike, without
  pulling the eye all the time.
- Konsole: the FreeLook Amber color scheme, in the spirit of the amber
  phosphor terminals. To stay retro without becoming flat, its colors are
  all amber, from deep orange to cream, and tell things apart by brightness,
  which also helps color-blind and dyslexic readers.
- GTK 3 theme: FreeLook Night. GTK 3 loads `gtk-dark.css` when Plasma
  prefers a dark theme; the generator writes it from `gtk.css` with the night
  palette and night pictures, so there is one CSS to maintain. All colors of
  `gtk.css` are now in its palette block, and the progress bar fill uses the
  selection color (the same gray by day, visible at night). GTK programs
  switch when reopened; Firefox follows at once.

Known issues (to investigate):

- Switching between FreeLook and FreeLook Night with programs open: with the
  FreeLook application style, the menu bar of Konsole (and parts of Dolphin
  and System Settings) keeps the old colors, while the rest of the program
  switches; with Breeze it switches. KCalc switches with both styles. The
  title bars of some windows also keep the old colors, with Breeze too.
  Reopening the program fixes it.

Next for FreeLook Night: stronger elevator and check box outlines on dark
palettes, and a real screenshot for the global theme preview.

## 0.9 (2026-11-26)

- GTK 3 theme with the same look as the Qt style: oblong buttons with the
  default ring, underlined text fields, numeric fields with raised boxes,
  combo boxes as menu buttons, the overflowing check mark, rectangular
  exclusive settings, tabs as exclusive settings, sliders with a drag box,
  menus where the chosen item sinks in, menu bar entries as menu buttons, and
  a scroll bar elevator marked with ▲ and ▼.
- `install.sh --apply` also switches GTK programs to FreeLook;
  `uninstall.sh` switches them back to Breeze.
- The session file turns GTK 3 overlay scroll bars off.
- Experimental panel layouts (`plasma/panels/`, tried with
  `tools/try-panel.sh`): a "Workspace ▽" menu (Kicker, opening on the
  favorites), a "Janelas ▽" window menu or task buttons, at the top, on the
  right, or both. Backs up the current panels and restores them.
  The clock and weather widgets keep the settings of your own panels.
  Kicker in Plasma 6.8 has no text label, so its button is a picture of an
  OPEN LOOK "Workspace ▽" menu button.
- Fonts: the Go family by Bigelow & Holmes (the designers of Lucida),
  installed with the theme; the global theme sets Go for the interface, Go
  Bold for window titles and Go Mono for fixed width. `uninstall.sh` puts
  the Plasma default fonts back if Go is in use.
- Plasma style: menu bar entries (global menu, window list) drawn as a
  raised box, sunken while open, instead of Breeze's highlight, which came
  out black. (A first version with round ends made the window list resize
  in a loop, with plasmashell at 200% CPU.)
- `install.sh --no-style` no longer drops the Qt plugin path from the
  session file when the style was installed before.
- Application style: rows of lists and trees have some air (Dolphin's places
  panel was cramped, icons touching each other).
- Application style: Plasma's own menus (desktop context menu, panel menus)
  are gray too; Plasma hands them a palette with a white window color.
- GIMP 3 (with its "System" theme): the icon tabs of the docks get room
  again.

## 0.8 (2026-11-26)

- Tabs drawn as an OPEN LOOK exclusive setting: raised rectangles side by
  side sharing a single line, the current one sunken with a black outline,
  and the page in a plain frame just below. (OPEN LOOK itself had no tabs;
  property windows used a "Category" menu button.)
- Tab close button: a thin black "x" without color, which gets a raised box
  under the mouse and sinks while pressed.
- Numeric fields (spin boxes): the number on an underline and two raised
  boxes side by side, [▲][▼]; the arrow at a limit is grayed out. The
  "+ −" variant is supported too.
- Combo boxes: an oblong menu button with a hollow triangle. Editable combo
  boxes: underlined text with an abbreviated menu button, like "Mail File: [▽]".
- Every text field, inside spin boxes and combo boxes too, now sits on the
  window gray instead of a white box.
- Sliders: a recessed bar filled in dark gray from the minimum to a raised
  drag box, which sinks while it is dragged; thin tick marks.
- Menus: a raised gray panel with a dark outline; the item under the pointer
  sinks in instead of turning into a colored bar; etched separators.
- Menu bar entries drawn as OPEN LOOK menu buttons, "File ▽", sunken while
  their menu is open.

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
