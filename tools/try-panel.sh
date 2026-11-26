#!/bin/sh
# Try one of the experimental FreeLook panel layouts (plasma/panels/*.js).
#
#   tools/try-panel.sh <layout>          back up the current panels, then switch
#   tools/try-panel.sh --list            list the layouts
#   tools/try-panel.sh --restore [dir]   put back your own panels (or a given backup)
#
# Layouts replace ALL current panels (desktop widgets stay). Before every
# switch the panel configuration is copied to
# ~/.local/share/freelook/panel-backups/<date-time>/. The first copy is also
# kept as "original", and --restore puts that one back, however many layouts
# you tried in between.
#
# SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
# SPDX-License-Identifier: GPL-3.0-or-later
set -e

SRC=$(cd "$(dirname "$0")/.." && pwd)
LAYOUTS="$SRC/plasma/panels"
CONFIG=${XDG_CONFIG_HOME:-$HOME/.config}
BACKUPS="${XDG_DATA_HOME:-$HOME/.local/share}/freelook/panel-backups"
FILES="plasma-org.kde.plasma.desktop-appletsrc plasmashellrc"

usage() { sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'; }

# Runs a Plasma script through D-Bus: with qdbus (named qdbus6 or qdbus-qt6
# on some distributions) or, failing that, with dbus-send.
evaluate_script() {   # evaluate_script <script text>
    for q in qdbus6 qdbus-qt6 qdbus; do
        if command -v "$q" >/dev/null 2>&1; then
            "$q" org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript "$1" >/dev/null
            return
        fi
    done
    if command -v dbus-send >/dev/null 2>&1; then
        dbus-send --session --print-reply=literal --dest=org.kde.plasmashell /PlasmaShell \
            org.kde.PlasmaShell.evaluateScript string:"$1" >/dev/null
        return
    fi
    echo "No D-Bus tool found (qdbus6, qdbus-qt6, qdbus or dbus-send)." >&2
    exit 1
}

list_layouts() {
    for f in "$LAYOUTS"/*.js; do
        name=$(basename "$f" .js)
        [ "$name" = common ] && continue
        printf '  %-14s %s\n' "$name" "$(head -n 1 "$f" | sed 's#^// *##')"
    done
}

stop_shell() {
    # plasmashell writes its configuration when it quits, so stop it first.
    if command -v kquitapp6 >/dev/null 2>&1; then
        kquitapp6 plasmashell >/dev/null 2>&1 || true
    else
        pkill -x plasmashell 2>/dev/null || true
    fi
    sleep 2
}

start_shell() {
    if command -v kstart >/dev/null 2>&1; then
        kstart plasmashell >/dev/null 2>&1
    else
        setsid plasmashell >/dev/null 2>&1 &
    fi
}

case "$1" in
    ""|-h|--help)
        usage
        echo
        echo "Layouts:"
        list_layouts
        exit 0
        ;;
    --list)
        list_layouts
        exit 0
        ;;
    --restore)
        dir=${2:-$BACKUPS/original}
        dir=${dir%/}
        if [ -z "$dir" ] || [ ! -d "$dir" ]; then
            echo "No backup found in $BACKUPS" >&2
            exit 1
        fi
        echo "==> Restoring the panels from $dir"
        stop_shell
        for f in $FILES; do
            [ -f "$dir/$f" ] && cp "$dir/$f" "$CONFIG/$f"
        done
        start_shell
        if [ "$dir" = "$BACKUPS/original" ]; then
            # The next experiment starts from these panels again.
            mv "$dir" "$BACKUPS/original-restored-$(date +%Y%m%d-%H%M%S)"
        fi
        echo "    Done."
        exit 0
        ;;
esac

LAYOUT="$LAYOUTS/$1.js"
if [ ! -f "$LAYOUT" ] || [ "$1" = common ]; then
    echo "Unknown layout: $1" >&2
    echo "Layouts:" >&2
    list_layouts >&2
    exit 2
fi

# Back up the current panels.
stamp=$(date +%Y%m%d-%H%M%S)
mkdir -p "$BACKUPS/$stamp"
for f in $FILES; do
    [ -f "$CONFIG/$f" ] && cp "$CONFIG/$f" "$BACKUPS/$stamp/$f"
done
echo "==> Current panels saved in $BACKUPS/$stamp"
if [ ! -d "$BACKUPS/original" ]; then
    cp -r "$BACKUPS/$stamp" "$BACKUPS/original"
    echo "    (also kept as your original panels)"
fi

echo "==> Switching to the '$1' layout"
script=$(cat "$LAYOUTS/common.js" "$LAYOUT")
evaluate_script "$script"

echo "    To go back: tools/try-panel.sh --restore"
