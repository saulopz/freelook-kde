/*
    FreeLook: the OPEN LOOK window menu button (the box with the triangle).

    As in olwm:
      left click   -> default menu action ("Close", which iconifies = minimize)
      right click  -> opens the window menu

    If the window cannot be minimized, a left click opens the menu too.
    With alwaysMenu: true, both clicks open the menu.

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick
import "FLGeometry.js" as Geometry

Item {
    id: button
    property int size: 22
    property bool alwaysMenu: false
    property bool pressed: false
    property color base: "#cccccc"
    property color light: "#ffffff"
    property color dark: "#808080"
    property color ink: "#000000"

    width: size
    height: size

    FLBox {
        id: box
        width: button.size - 4
        height: button.size - 5
        x: 2
        y: Math.round((button.size - height) / 2)
        sunken: button.pressed
        base: button.base
        light: button.light
        dark: button.dark
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var m = Geometry.mapper({x: box.x, y: box.y, w: box.width, h: box.height}, button.pressed ? 1 : 0);
            // Hollow triangle pointing down, centered in the box.
            ctx.beginPath();
            ctx.moveTo(m.px(4.5), m.py(6));
            ctx.lineTo(m.px(13.5), m.py(6));
            ctx.lineTo((m.px(4.5) + m.px(13.5)) / 2, m.py(12.5));
            ctx.closePath();
            ctx.fillStyle = button.pressed ? Qt.darker(button.base, 1.11) : button.base;
            ctx.fill();
            ctx.lineWidth = m.lineWidth;
            ctx.lineJoin = "miter";
            ctx.strokeStyle = button.ink;
            ctx.stroke();
        }
    }
    onPressedChanged: canvas.requestPaint()
    onSizeChanged: canvas.requestPaint()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: button.pressed = true
        onReleased: button.pressed = false
        onExited: button.pressed = false
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton || button.alwaysMenu || !decoration.client.minimizeable) {
                decoration.requestShowWindowMenu();
            } else {
                decoration.requestMinimize();
            }
        }
    }
}
