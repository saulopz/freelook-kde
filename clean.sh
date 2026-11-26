#!/bin/sh
# FreeLook: removes build output and files written by the test tools.
# It never touches files that are part of the project.
#
#   ./clean.sh          lists what will be removed and asks for confirmation
#   ./clean.sh --yes    no questions asked
#
# Keep this list in sync with .gitignore.
#
# SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
# SPDX-License-Identifier: GPL-3.0-or-later
set -e

cd "$(dirname "$0")"

YES=0
for arg in "$@"; do
    case "$arg" in
        -y|--yes) YES=1 ;;
        -h|--help) sed -n '2,8p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown option: $arg (see --help)" >&2; exit 2 ;;
    esac
done

# Everything that may be generated, relative to the repository root.
list_generated() {
    for path in style/build style/build.log tools/qml-test/preview_decoration.png gallery.png gtk.png gtk-menu.png; do
        [ -e "$path" ] && echo "$path"
    done
    for path in preview*.png; do
        [ -e "$path" ] && echo "$path"
    done
    find . -name __pycache__ -type d -not -path './.git/*' | sed 's|^\./||'
    find . -name '*.pyc' -type f -not -path './.git/*' -not -path '*/__pycache__/*' | sed 's|^\./||'
}

found=$(list_generated)
if [ -z "$found" ]; then
    echo "Nothing to clean."
    exit 0
fi

echo "These generated files will be removed:"
echo "$found" | sed 's/^/    /'

if [ "$YES" = 0 ]; then
    printf 'Remove them? [y/N] '
    read -r answer || answer=""
    case "$answer" in
        y|Y|yes|Yes|s|S|sim) ;;
        *) echo "Nothing was removed."; exit 0 ;;
    esac
fi

echo "$found" | while IFS= read -r path; do
    rm -rf -- "$path"
done
echo "Clean."
