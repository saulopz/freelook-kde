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
    "$HOME/.local/lib/qt6/plugins/styles/libfreelook.so" \
    "$CONFIG/plasma-workspace/env/freelook.sh"
do
    if [ -e "$path" ]; then
        rm -rf "$path"
        echo "    removed $path"
    fi
done

echo "==> Done. Log out and back in to make sure no application still uses FreeLook."
