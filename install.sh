#!/bin/sh
# FreeLook installer: installs everything for the current user (no root needed).
#
#   ./install.sh             install all components
#   ./install.sh --apply     install and switch to the FreeLook global theme
#   ./install.sh --no-style  skip building the C++ application style
#
# SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
# SPDX-License-Identifier: GPL-3.0-or-later
set -e

SRC=$(cd "$(dirname "$0")" && pwd)
DATA=${XDG_DATA_HOME:-$HOME/.local/share}
CONFIG=${XDG_CONFIG_HOME:-$HOME/.config}
PLUGINS="$HOME/.local/lib/qt6/plugins"
ENV_FILE="$CONFIG/plasma-workspace/env/freelook.sh"
LNF_ID=org.freelook.desktop

APPLY=0
BUILD_STYLE=1
for arg in "$@"; do
    case "$arg" in
        --apply) APPLY=1 ;;
        --no-style) BUILD_STYLE=0 ;;
        -h|--help) sed -n '2,7p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown option: $arg (see --help)" >&2; exit 2 ;;
    esac
done

step() { printf '\n==> %s\n' "$*"; }
copy_dir() {   # copy_dir <source> <destination>
    rm -rf "$2"
    mkdir -p "$(dirname "$2")"
    cp -r "$1" "$2"
    echo "    $2"
}

step "Installing theme files into $DATA"
mkdir -p "$DATA/color-schemes"
cp "$SRC/color-schemes/FreeLook.colors" "$DATA/color-schemes/FreeLook.colors"
echo "    $DATA/color-schemes/FreeLook.colors"
copy_dir "$SRC/decoration/kwin4_decoration_qml_freelook" "$DATA/kwin/decorations/kwin4_decoration_qml_freelook"
copy_dir "$SRC/plasma/desktoptheme/FreeLook" "$DATA/plasma/desktoptheme/FreeLook"
copy_dir "$SRC/plasma/look-and-feel/$LNF_ID" "$DATA/plasma/look-and-feel/$LNF_ID"
copy_dir "$SRC/plasma/wallpapers/FreeLook" "$DATA/wallpapers/FreeLook"

STYLE_OK=0
if [ "$BUILD_STYLE" = 1 ]; then
    step "Building the application style (C++)"
    if ! command -v cmake >/dev/null 2>&1 || ! command -v c++ >/dev/null 2>&1; then
        echo "    cmake or a C++ compiler is missing, so the application style was skipped."
        echo "    Install the build tools and run ./install.sh again:"
        echo "      Fedora:         sudo dnf install cmake gcc-c++ qt6-qtbase-devel"
        echo "      Debian/Ubuntu:  sudo apt install cmake g++ qt6-base-dev"
        echo "      openSUSE:       sudo zypper install cmake gcc-c++ qt6-base-devel"
        echo "      Arch:           sudo pacman -S cmake gcc qt6-base"
    else
        BUILD="$SRC/style/build"
        if cmake -S "$SRC/style" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release >"$BUILD.log" 2>&1 \
            && cmake --build "$BUILD" -j"$(nproc 2>/dev/null || echo 2)" >>"$BUILD.log" 2>&1; then
            # Test the freshly built plugin before installing it.
            rm -rf "$BUILD/stage"
            cmake --install "$BUILD" --prefix "$BUILD/stage" >>"$BUILD.log" 2>&1
            echo "    running the style checks..."
            CHECKS_OK=0
            QT_QPA_PLATFORM=offscreen QT_PLUGIN_PATH="$BUILD/stage/lib/qt6/plugins" \
                "$BUILD/freelook-gallery" "$BUILD/gallery.png" >"$BUILD/checks.log" 2>&1 && CHECKS_OK=1
            grep -v -e QStandardPaths -e propagateSizeHints "$BUILD/checks.log" | sed 's/^/    /'
            if [ "$CHECKS_OK" = 1 ]; then
                cmake --install "$BUILD" >>"$BUILD.log" 2>&1
                echo "    installed: $PLUGINS/styles/libfreelook.so"
                STYLE_OK=1
            else
                echo "    The style checks failed, so the style was NOT installed. Details: $BUILD.log"
            fi
        else
            echo "    The build failed (is the Qt 6 development package installed?)."
            echo "    Details: $BUILD.log"
        fi
    fi
fi

if [ "$STYLE_OK" = 1 ]; then
    # Plasma sources the scripts in this folder at login, so every Qt
    # application finds the style installed in the home folder.
    mkdir -p "$(dirname "$ENV_FILE")"
    cat > "$ENV_FILE" <<'EOF'
# FreeLook: Qt plugins installed in the home folder
export QT_PLUGIN_PATH="$HOME/.local/lib/qt6/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
EOF
    echo "    $ENV_FILE"
fi

if [ "$APPLY" = 1 ]; then
    step "Applying the FreeLook global theme"
    if command -v plasma-apply-lookandfeel >/dev/null 2>&1; then
        plasma-apply-lookandfeel -a "$LNF_ID" || true
    else
        echo "    plasma-apply-lookandfeel not found: choose FreeLook in"
        echo "    System Settings > Colors & Themes > Global Theme."
    fi
    if command -v plasma-apply-wallpaperimage >/dev/null 2>&1; then
        plasma-apply-wallpaperimage "$DATA/wallpapers/FreeLook" >/dev/null 2>&1 || true
    fi
fi

step "Done"
if [ "$APPLY" = 0 ]; then
    echo "    Choose FreeLook in System Settings > Colors & Themes > Global Theme,"
    echo "    or run ./install.sh --apply"
fi
if [ "$STYLE_OK" = 1 ]; then
    echo "    The first time, log out and back in so every application finds the style."
    echo "    To try it right away: QT_PLUGIN_PATH=$PLUGINS dolphin -style FreeLook"
fi
echo "    To remove everything: ./uninstall.sh"
