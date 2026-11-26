#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""GTK 3 widget gallery used to test the FreeLook GTK theme.

Usage (in a desktop session or under xvfb-run):
    GTK_THEME=FreeLook GTK_OVERLAY_SCROLLING=0 python3 tools/gtk-test/gallery3.py [output.png]

Prints every CSS parsing error of the theme and, with an output file, saves a
screenshot of the screen with the gallery and an open menu, then quits.
The exit code is 0 only when the theme loads without CSS errors.
"""
import sys

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, GLib, Gtk  # noqa: E402

errors = []


def on_parsing_error(provider, section, error):
    errors.append(f"line {section.get_start_line() + 1}: {error.message}")


def check_theme_css():
    theme = Gtk.Settings.get_default().props.gtk_theme_name
    for path in (f"gtk/FreeLook/gtk-3.0/gtk.css",):
        provider = Gtk.CssProvider()
        provider.connect("parsing-error", on_parsing_error)
        try:
            provider.load_from_path(path)
        except GLib.Error:
            pass
    return theme


def build():
    win = Gtk.Window(title="FreeLook GTK 3")
    win.set_default_size(700, 560)
    win.move(0, 0)
    outer = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
    win.add(outer)

    bar = Gtk.MenuBar()
    file_item = Gtk.MenuItem.new_with_mnemonic("_File")
    menu = Gtk.Menu()
    items = {}
    for label in ("Open...", "Save", "Save as...", None, "Wrap lines", "Recent files", None, "Quit"):
        if label is None:
            menu.append(Gtk.SeparatorMenuItem())
            continue
        if label == "Wrap lines":
            item = Gtk.CheckMenuItem(label=label)
            item.set_active(True)
        else:
            item = Gtk.MenuItem(label=label)
        if label == "Save as...":
            item.set_sensitive(False)
        if label == "Recent files":
            sub = Gtk.Menu()
            sub.append(Gtk.MenuItem(label="notes.txt"))
            item.set_submenu(sub)
        menu.append(item)
        items[label] = item
    file_item.set_submenu(menu)
    bar.append(file_item)
    for name in ("_Edit", "_View"):
        bar.append(Gtk.MenuItem.new_with_mnemonic(name))
    outer.pack_start(bar, False, False, 0)

    notebook = Gtk.Notebook()
    outer.pack_start(notebook, True, True, 0)
    grid = Gtk.Grid(column_spacing=12, row_spacing=8, margin=10)
    notebook.append_page(grid, Gtk.Label(label="Controls"))
    notebook.append_page(Gtk.Box(), Gtk.Label(label="Another tab"))
    notebook.append_page(Gtk.Box(), Gtk.Label(label="Third"))

    buttons = Gtk.Box(spacing=6)
    buttons.pack_start(Gtk.Button(label="Load..."), False, False, 0)
    save = Gtk.Button(label="Save...")
    save.set_can_default(True)
    save.get_style_context().add_class("default")
    buttons.pack_start(save, False, False, 0)
    menubutton = Gtk.MenuButton(label="Print")
    buttons.pack_start(menubutton, False, False, 0)
    off = Gtk.Button(label="Disabled")
    off.set_sensitive(False)
    buttons.pack_start(off, False, False, 0)
    toggle = Gtk.ToggleButton(label="Pressed")
    toggle.set_active(True)
    buttons.pack_start(toggle, False, False, 0)
    grid.attach(buttons, 0, 0, 2, 1)

    left = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
    c1 = Gtk.CheckButton(label="Beep during countdown"); c1.set_active(True)
    left.pack_start(c1, False, False, 0)
    left.pack_start(Gtk.CheckButton(label="Hide window"), False, False, 0)
    c3 = Gtk.CheckButton(label="Partial"); c3.set_inconsistent(True)
    left.pack_start(c3, False, False, 0)
    r1 = Gtk.RadioButton(label="Window")
    r2 = Gtk.RadioButton.new_with_label_from_widget(r1, "Region")
    left.pack_start(r1, False, False, 0)
    left.pack_start(r2, False, False, 0)
    sw = Gtk.Switch(); sw.set_active(True); sw.set_halign(Gtk.Align.START)
    left.pack_start(sw, False, False, 0)
    grid.attach(left, 0, 1, 1, 1)

    right = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
    row = Gtk.Box(spacing=6)
    row.pack_start(Gtk.Label(label="Mail file:"), False, False, 0)
    e = Gtk.Entry(); e.set_text("~/mail/inbox")
    row.pack_start(e, True, True, 0)
    right.pack_start(row, False, False, 0)
    focused = Gtk.Entry(); focused.set_text("has focus")
    right.pack_start(focused, False, False, 0)
    editable = Gtk.ComboBoxText.new_with_entry()
    editable.append_text("editable combo"); editable.set_active(0)
    right.pack_start(editable, False, False, 0)
    combo = Gtk.ComboBoxText()
    combo.append_text("Combo"); combo.append_text("Two"); combo.set_active(0)
    right.pack_start(combo, False, False, 0)
    spins = Gtk.Box(spacing=8)
    spins.pack_start(Gtk.SpinButton.new_with_range(0, 100, 1), False, False, 0)
    s2 = Gtk.SpinButton.new_with_range(0, 10, 1); s2.set_value(10)
    spins.pack_start(s2, False, False, 0)
    right.pack_start(spins, False, False, 0)
    scale = Gtk.Scale.new_with_range(Gtk.Orientation.HORIZONTAL, 0, 100, 1)
    scale.set_value(40); scale.set_draw_value(False)
    for v in range(0, 101, 10):
        scale.add_mark(v, Gtk.PositionType.BOTTOM, None)
    right.pack_start(scale, False, False, 0)
    progress = Gtk.ProgressBar(); progress.set_fraction(0.65)
    progress.set_text("65%"); progress.set_show_text(True)
    right.pack_start(progress, False, False, 0)
    grid.attach(right, 1, 1, 1, 1)

    store = Gtk.ListStore(str)
    for i in range(1, 41):
        store.append([f"List item {i}"])
    tree = Gtk.TreeView(model=store)
    tree.append_column(Gtk.TreeViewColumn("Name", Gtk.CellRendererText(), text=0))
    tree.get_selection().select_path(Gtk.TreePath(1))
    sc = Gtk.ScrolledWindow(); sc.set_size_request(260, 170); sc.add(tree)
    sc.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.ALWAYS)
    grid.attach(sc, 0, 2, 1, 1)
    text = Gtk.TextView()
    text.get_buffer().set_text("\n".join(f"A line of text long enough to scroll sideways {i}" for i in range(60)))
    sc2 = Gtk.ScrolledWindow(); sc2.set_size_request(320, 170); sc2.add(text)
    sc2.set_policy(Gtk.PolicyType.ALWAYS, Gtk.PolicyType.ALWAYS)
    grid.attach(sc2, 1, 2, 1, 1)

    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    focused.grab_focus()
    return win, bar, file_item, menu, items


def main():
    theme = check_theme_css()
    print(f"GTK theme in use: {theme}")
    print(f"CSS parsing errors: {len(errors)}")
    for e in errors:
        print("  " + e)
    output = sys.argv[1] if len(sys.argv) > 1 else None
    win, bar, file_item, menu, items = build()

    def shoot_window():
        root = Gdk.get_default_root_window()
        Gdk.pixbuf_get_from_window(root, 0, 0, 720, 620).savev(output, "png", [], [])
        print(f"screenshot saved: {output}")
        return False

    def open_menu():
        bar.select_item(file_item)
        file_item.activate()
        return False

    def highlight():
        menu.select_item(items["Save"])
        return False

    def shoot():
        if output:
            root = Gdk.get_default_root_window()
            pb = Gdk.pixbuf_get_from_window(root, 0, 0, 720, 620)
            menu_output = output.replace(".png", "-menu.png")
            pb.savev(menu_output, "png", [], [])
            print(f"menu screenshot saved: {menu_output}")
        Gtk.main_quit()
        return False

    if output:
        GLib.timeout_add(500, shoot_window)
        GLib.timeout_add(600, open_menu)
        GLib.timeout_add(900, highlight)
        GLib.timeout_add(1400, shoot)
    Gtk.main()
    sys.exit(0 if not errors else 1)


if __name__ == "__main__":
    main()
