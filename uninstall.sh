#!/bin/sh
# FreeLook uninstaller: removes every component installed by install.sh.
#
#   ./uninstall.sh          asks for confirmation
#   ./uninstall.sh --yes    no questions asked
#
# If any FreeLook component is in use, it first switches back to the Breeze
# global theme (and the default wallpaper), so KWin and Plasma are never left
# pointing at files that no longer exist.
#
# SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
# SPDX-License-Identifier: GPL-3.0-or-later
set -e

DATA=${XDG_DATA_HOME:-$HOME/.local/share}
CONFIG=${XDG_CONFIG_HOME:-$HOME/.config}
LNF_ID=org.freelook.desktop

YES=0
for arg in "$@"; do
    case "$arg" in
        -y|--yes) YES=1 ;;
        -h|--help) sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown option: $arg (see --help)" >&2; exit 2 ;;
    esac
done

if [ "$YES" = 0 ]; then
    printf 'Remove FreeLook from this user account? [y/N] '
    read -r answer || answer=""
    case "$answer" in
        y|Y|yes|Yes|s|S|sim) ;;
        *) echo "Nothing was removed."; exit 0 ;;
    esac
fi

# Reads a setting with kreadconfig6 (or kreadconfig5); prints nothing if unavailable.
read_setting() {   # read_setting <file> <group> <key>
    for tool in kreadconfig6 kreadconfig5; do
        if command -v "$tool" >/dev/null 2>&1; then
            "$tool" --file "$1" --group "$2" --key "$3" 2>/dev/null || true
            return
        fi
    done
}

in_use=""
[ "$(read_setting kdeglobals KDE LookAndFeelPackage)" = "$LNF_ID" ] && in_use="$in_use global-theme"
[ "$(read_setting kdeglobals KDE widgetStyle)" = "FreeLook" ] && in_use="$in_use application-style"
[ "$(read_setting kdeglobals General ColorScheme)" = "FreeLook" ] && in_use="$in_use colors"
[ "$(read_setting plasmarc Theme name)" = "FreeLook" ] && in_use="$in_use plasma-style"
case "$(read_setting kwinrc org.kde.kdecoration2 theme)" in
    kwin4_decoration_qml_freelook|__aurorae__svg__FreeLook) in_use="$in_use decoration" ;;
esac

if [ -n "$in_use" ]; then
    echo "==> FreeLook is in use ($in_use ), switching back to Breeze"
    if command -v plasma-apply-lookandfeel >/dev/null 2>&1; then
        plasma-apply-lookandfeel -a org.kde.breeze.desktop || true
    else
        echo "    plasma-apply-lookandfeel not found: choose Breeze in"
        echo "    System Settings > Colors & Themes > Global Theme after this."
    fi
fi

# GTK theme: if GTK programs use FreeLook, go back to Breeze.
GTK_SETTINGS="$CONFIG/gtk-3.0/settings.ini"
if [ -f "$GTK_SETTINGS" ] && grep -q '^gtk-theme-name *= *FreeLook *$' "$GTK_SETTINGS"; then
    done_gtk=0
    for q in qdbus6 qdbus-qt6 qdbus; do
        if command -v "$q" >/dev/null 2>&1 \
            && "$q" org.kde.kded6 /modules/gtkconfig org.kde.GtkConfig.setGtkTheme Breeze >/dev/null 2>&1; then
            done_gtk=1
            break
        fi
    done
    if [ "$done_gtk" = 0 ]; then
        for tool in kwriteconfig6 kwriteconfig5; do
            if command -v "$tool" >/dev/null 2>&1; then
                "$tool" --file "$GTK_SETTINGS" --group Settings --key gtk-theme-name Breeze
                break
            fi
        done
        if command -v gsettings >/dev/null 2>&1; then
            gsettings set org.gnome.desktop.interface gtk-theme Breeze 2>/dev/null || true
        fi
        sed -i 's/^gtk-theme-name *= *FreeLook *$/gtk-theme-name=Breeze/' "$GTK_SETTINGS"
    fi
    echo "==> GTK theme set back to Breeze"
fi

# Fonts: if the session uses the Go fonts installed by FreeLook, go back to
# Plasma's default fonts (removing the keys restores the defaults).
if [ "$(read_setting kdeglobals General font | cut -d, -f1)" = "Go" ]; then
    for tool in kwriteconfig6 kwriteconfig5; do
        if command -v "$tool" >/dev/null 2>&1; then
            for key in font menuFont toolBarFont smallestReadableFont fixed; do
                "$tool" --file kdeglobals --group General --key "$key" --delete
            done
            "$tool" --file kdeglobals --group WM --key activeFont --delete
            echo "==> Fonts set back to the Plasma defaults"
            break
        fi
    done
fi

# Cursors: if the session uses the FreeLook cursors, go back to Breeze's.
if [ "$(read_setting kcminputrc Mouse cursorTheme)" = "FreeLook" ]; then
    if command -v plasma-apply-cursortheme >/dev/null 2>&1; then
        plasma-apply-cursortheme breeze_cursors >/dev/null 2>&1 || true
    else
        for tool in kwriteconfig6 kwriteconfig5; do
            if command -v "$tool" >/dev/null 2>&1; then
                "$tool" --file kcminputrc --group Mouse --key cursorTheme --delete
                break
            fi
        done
    fi
    echo "==> Cursors set back to Breeze"
fi

# Wallpaper: if the desktop shows the FreeLook wallpaper, go back to the default one.
APPLETS="$CONFIG/plasma-org.kde.plasma.desktop-appletsrc"
if [ -f "$APPLETS" ] && grep -q "wallpapers/FreeLook" "$APPLETS" 2>/dev/null; then
    for default in /usr/share/wallpapers/Next /usr/local/share/wallpapers/Next; do
        if [ -d "$default" ] && command -v plasma-apply-wallpaperimage >/dev/null 2>&1; then
            plasma-apply-wallpaperimage "$default" >/dev/null 2>&1 || true
            echo "==> Wallpaper set back to the default (Next)"
            break
        fi
    done
fi

echo "==> Removing files"
for path in \
    "$DATA/color-schemes/FreeLook.colors" \
    "$DATA/kwin/decorations/kwin4_decoration_qml_freelook" \
    "$DATA/aurorae/themes/FreeLook" \
    "$DATA/plasma/desktoptheme/FreeLook" \
    "$DATA/plasma/look-and-feel/$LNF_ID" \
    "$DATA/wallpapers/FreeLook" \
    "$DATA/themes/FreeLook" \
    "$DATA/fonts/FreeLook" \
    "$DATA/icons/FreeLook" \
    "$HOME/.icons/FreeLook" \
    "$HOME/.local/lib/qt6/plugins/styles/libfreelook.so" \
    "$CONFIG/plasma-workspace/env/freelook.sh"
do
    if [ -e "$path" ]; then
        rm -rf "$path"
        echo "    removed $path"
    fi
done

echo "==> Done. Log out and back in to make sure no application still uses FreeLook."
