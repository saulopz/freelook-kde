/*
    FreeLook splash screen: an OPEN LOOK notice window over the wallpaper,
    with a stopwatch (the OPEN LOOK "please wait" sign) whose hand turns while
    the Workspace starts, a line that names the current stage and a progress
    bar.

    One file for the day and the night: the colors come from the color scheme
    (FreeLook or FreeLook Night), and the wallpaper follows it. install.sh
    copies this splash into the FreeLook Night global theme too.

    Test it without logging out:
        ksplashqml --test --window plasma/look-and-feel/org.freelook.desktop

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    // Set by ksplashqml: 1 to 5 while the session starts (it closes at 6).
    property int stage: 1

    // ---------------------------------------------------------------- palette
    // The same relief rules as the application style (freelookstyle.cpp).
    readonly property color base: Kirigami.Theme.backgroundColor
    readonly property bool night: base.hslLightness < 0.5
    readonly property color ink: Kirigami.Theme.textColor
    readonly property color light: night ? Qt.lighter(base, 1.5) : "#ffffff"
    readonly property color softShadow: Qt.darker(base, 1.27)
    readonly property color dark: night ? Qt.darker(base, 2.5) : Qt.darker(base, 1.6)
    readonly property color dimInk: Qt.tint(ink, Qt.rgba(base.r, base.g, base.b, 0.35))
    readonly property color face: night ? ink : "#ffffff"

    // ---------------------------------------------------------------- words
    readonly property bool portuguese: Qt.locale().name.startsWith("pt")
    readonly property var stages: portuguese
        ? ["Preparando", "Iniciando a sessão", "Iniciando o gerenciador de sessão",
           "Iniciando o gerenciador de janelas", "Carregando a área de trabalho"]
        : ["Getting ready", "Starting the session", "Starting the session manager",
           "Starting the window manager", "Loading the desktop"]

    readonly property real unit: Kirigami.Units.gridUnit / 18   // 1 at the usual font size

    color: night ? "#16222a" : "#40a0c0"   // when the wallpaper is missing

    // The FreeLook wallpaper, installed by install.sh next to this package:
    // ~/.local/share/plasma/look-and-feel/<theme>/contents/splash/ ->
    // ~/.local/share/wallpapers/FreeLook/
    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        source: "../../../../../wallpapers/FreeLook/contents/"
                + (root.night ? "images_dark" : "images") + "/1920x1080.png"
    }

    // ---------------------------------------------------------------- the notice window
    Rectangle {
        id: notice
        anchors.centerIn: parent
        width: Math.round(600 * root.unit)
        height: Math.round(210 * root.unit)
        color: root.base
        border.color: "#000000"
        border.width: 1

        // Relief: light top and left, soft shadow bottom and right.
        Rectangle { x: 1; y: 1; width: parent.width - 2; height: 1; color: root.light }
        Rectangle { x: 1; y: 1; width: 1; height: parent.height - 2; color: root.light }
        Rectangle { x: 1; y: parent.height - 2; width: parent.width - 2; height: 1; color: root.softShadow }
        Rectangle { x: parent.width - 2; y: 1; width: 1; height: parent.height - 2; color: root.softShadow }

        Text {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            y: Math.round(12 * root.unit)
            text: "FreeLook"
            color: root.ink
            font.family: "Go"
            font.pixelSize: Math.round(22 * root.unit)
        }
        // Etched line under the title.
        Rectangle { x: 6; y: Math.round(46 * root.unit); width: parent.width - 12; height: 1; color: root.softShadow }
        Rectangle { x: 6; y: Math.round(46 * root.unit) + 1; width: parent.width - 12; height: 1; color: root.light }

        // ------------------------------------------------------------ stopwatch
        Item {
            id: stopwatch
            readonly property real r: 52 * root.unit
            x: Math.round(95 * root.unit - r)
            y: Math.round(135 * root.unit - r)
            width: 2 * r
            height: 2 * r

            Canvas {
                id: dial
                anchors.fill: parent
                anchors.margins: -Math.ceil(0.4 * stopwatch.r)   // room for the crown
                property color face: root.face
                property color base: root.base
                onFaceChanged: requestPaint()
                onBaseChanged: requestPaint()
                onPaint: {
                    const ctx = getContext("2d");
                    const r = stopwatch.r;
                    const cx = width / 2, cy = height / 2;
                    const u = root.unit;
                    ctx.reset();
                    ctx.strokeStyle = "#000000";
                    // Crown on top.
                    ctx.fillStyle = base;
                    ctx.lineWidth = 3 * u;
                    ctx.fillRect(cx - 0.18 * r, cy - 1.32 * r, 0.36 * r, 0.24 * r);
                    ctx.strokeRect(cx - 0.18 * r, cy - 1.32 * r, 0.36 * r, 0.24 * r);
                    // Face.
                    ctx.beginPath();
                    ctx.arc(cx, cy, r - 2.5 * u, 0, 2 * Math.PI);
                    ctx.fillStyle = face;
                    ctx.fill();
                    ctx.lineWidth = 5 * u;
                    ctx.stroke();
                    // Twelve marks.
                    ctx.lineWidth = 3 * u;
                    for (let k = 0; k < 12; ++k) {
                        const a = k * Math.PI / 6;
                        ctx.beginPath();
                        ctx.moveTo(cx + 0.72 * r * Math.sin(a), cy - 0.72 * r * Math.cos(a));
                        ctx.lineTo(cx + 0.86 * r * Math.sin(a), cy - 0.86 * r * Math.cos(a));
                        ctx.stroke();
                    }
                    // The short hand moves with the stages.
                    ctx.lineWidth = 6 * u;
                    ctx.lineCap = "round";
                    const s = (root.stage - 1) / 5 * 2 * Math.PI;
                    ctx.beginPath();
                    ctx.moveTo(cx, cy);
                    ctx.lineTo(cx + 0.45 * r * Math.sin(s), cy - 0.45 * r * Math.cos(s));
                    ctx.stroke();
                }
                Connections {
                    target: root
                    function onStageChanged() { dial.requestPaint(); }
                }
            }

            // The long hand sweeps while waiting.
            Item {
                id: sweep
                anchors.fill: parent
                Rectangle {
                    width: Math.max(2, Math.round(4 * root.unit))
                    height: stopwatch.r * 0.72
                    radius: width / 2
                    color: "#000000"
                    x: (parent.width - width) / 2
                    y: parent.height / 2 - height
                }
                RotationAnimator on rotation {
                    from: 0
                    to: 360
                    // Not tied to the animation speed setting, like Breeze's
                    // splash, but still off when animations are disabled.
                    duration: 4000
                    loops: Animation.Infinite
                    running: Kirigami.Units.longDuration > 1
                }
            }
            Rectangle {
                anchors.centerIn: parent
                width: Math.round(12 * root.unit)
                height: width
                radius: width / 2
                color: "#000000"
            }
        }

        // ------------------------------------------------------------ words and progress
        Column {
            x: Math.round(180 * root.unit)
            y: Math.round(78 * root.unit)
            width: parent.width - x - Math.round(40 * root.unit)
            spacing: Math.round(8 * root.unit)

            Text {
                text: root.portuguese ? "Iniciando o Workspace..." : "Starting Workspace..."
                color: root.ink
                font.family: "Go"
                font.pixelSize: Math.round(21 * root.unit)
            }
            Text {
                text: root.stages[Math.max(0, Math.min(root.stages.length - 1, root.stage - 1))]
                color: root.dimInk
                font.family: "Go"
                font.pixelSize: Math.round(16 * root.unit)
            }
            Item { width: 1; height: Math.round(4 * root.unit) }
            // Progress bar: a trough in the view color (white by day), filled
            // with the selection color, as in the application style.
            Rectangle {
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
                width: parent.width
                height: Math.round(20 * root.unit)
                color: Kirigami.Theme.backgroundColor
                border.color: root.dark
                border.width: 1
                Rectangle {
                    x: 2
                    y: 2
                    height: parent.height - 4
                    width: (parent.width - 4) * Math.min(1, root.stage / 5)
                    color: Kirigami.Theme.highlightColor
                    Behavior on width {
                        enabled: Kirigami.Units.longDuration > 1
                        NumberAnimation { duration: Kirigami.Units.longDuration }
                    }
                }
            }
        }
    }
}
