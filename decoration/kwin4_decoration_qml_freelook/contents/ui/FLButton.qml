/*
    FreeLook: title bar buttons.

    Inherits Aurorae's DecorationButton, which already handles clicks and
    states (hovered, pressed, toggled, enabled). Here we only paint.
    The button grows with "size"; glyphs follow the box.

    glyph: "close", "maximize", "pin" (pushpin), "below", "shade",
           "alldesktops", "help"

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick
import org.kde.kwin.decoration
import "FLGeometry.js" as Geometry

DecorationButton {
    id: button
    property string glyph: "close"
    property int size: 22
    property color base: "#cccccc"
    property color light: "#ffffff"
    property color dark: "#808080"
    property color ink: "#000000"
    property color inkDisabled: "#a0a0a0"

    readonly property bool sunken: pressed || (toggled && glyph !== "pin")
    readonly property bool maximized: decoration.client.maximized

    width: size
    height: size

    FLBox {
        id: box
        visible: button.glyph !== "pin"
        width: button.size - 4
        height: button.size - 5
        x: 2
        y: Math.round((button.size - height) / 2)
        sunken: button.sunken
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
            var m = Geometry.mapper({x: box.x, y: box.y, w: box.width, h: box.height}, button.sunken ? 1 : 0);
            var color = button.enabled ? button.ink : button.inkDisabled;
            ctx.strokeStyle = color;
            ctx.fillStyle = color;
            ctx.lineWidth = m.lineWidth;
            switch (button.glyph) {
            case "close":
                ctx.lineWidth = 1.4 * m.lineWidth;
                ctx.beginPath();
                ctx.moveTo(m.px(5.5), m.py(6)); ctx.lineTo(m.px(12.5), m.py(13));
                ctx.moveTo(m.px(12.5), m.py(6)); ctx.lineTo(m.px(5.5), m.py(13));
                ctx.stroke();
                break;
            case "maximize":
                if (button.maximized) {        // restore: two squares
                    m.strokeRect(ctx, 7.5, 5.5, 6, 5);
                    ctx.fillStyle = button.sunken ? Qt.darker(button.base, 1.11) : button.base;
                    m.fillRect(ctx, 4.5, 8.5, 6, 5);
                    m.strokeRect(ctx, 4.5, 8.5, 6, 5);
                } else {
                    m.fillRect(ctx, 4.5, 5.5, 9, 1);
                    m.strokeRect(ctx, 4.5, 5.5, 9, 8);
                }
                break;
            case "shade":
                m.fillRect(ctx, 5, 6, 8, 2);
                break;
            case "below":
                m.fillRect(ctx, 5, 11, 8, 2);
                break;
            case "alldesktops":
                m.fillRect(ctx, 6, 6, 2, 2); m.fillRect(ctx, 10, 6, 2, 2);
                m.fillRect(ctx, 6, 10, 2, 2); m.fillRect(ctx, 10, 10, 2, 2);
                break;
            case "help":
                ctx.beginPath();
                ctx.moveTo(m.px(6.5), m.py(7.5));
                ctx.bezierCurveTo(m.px(6.5), m.py(4.8), m.px(11.5), m.py(4.8), m.px(11.5), m.py(7.5));
                ctx.bezierCurveTo(m.px(11.5), m.py(9.2), m.px(9), m.py(9.2), m.px(9), m.py(11));
                ctx.stroke();
                m.fillRect(ctx, 8, 12, 2, 2);
                break;
            case "pin":
                // The pushpin was drawn for an 18 px button; here it just scales.
                var s = button.size / 18;
                ctx.scale(s, s);
                drawPushpin(ctx, button.toggled || button.pressed, color);
                break;
            }
        }

        // The OPEN LOOK pushpin: lying down when loose, seen head-on when pinned.
        function drawPushpin(ctx, pinned, color) {
            var c = 9;
            ctx.lineWidth = 1;
            if (!pinned) {
                ctx.fillStyle = color;
                ctx.fillRect(1, c, 7, 1);                       // needle
                ctx.fillRect(7, c - 2, 3, 5);                   // collar
                ctx.fillStyle = button.base;
                ctx.fillRect(8, c - 1, 1, 3);
                ctx.fillStyle = color;
                ctx.fillRect(10, c - 1, 3, 3);                  // shaft
                ctx.fillStyle = button.dark;
                ctx.fillRect(10, c, 3, 1);
                ctx.beginPath();                                 // head
                ctx.arc(14.5, c + 0.5, 2.8, 0, 2 * Math.PI);
                ctx.fillStyle = button.base; ctx.fill();
                ctx.strokeStyle = color; ctx.stroke();
                ctx.beginPath();
                ctx.arc(13.8, c - 0.3, 0.9, 0, 2 * Math.PI);
                ctx.fillStyle = button.light; ctx.fill();
            } else {
                ctx.beginPath();                                 // shadow
                ctx.ellipse(10 - 4.3, c + 2 - 3.6, 8.6, 7.2);
                ctx.fillStyle = Qt.darker(button.base, 1.27); ctx.fill();
                ctx.beginPath();                                 // head, seen from the front
                ctx.arc(9, c + 0.5, 3.8, 0, 2 * Math.PI);
                ctx.fillStyle = button.base; ctx.fill();
                ctx.strokeStyle = color; ctx.stroke();
                ctx.beginPath();
                ctx.arc(9, c + 0.5, 1.4, 0, 2 * Math.PI);
                ctx.fillStyle = color; ctx.fill();
                ctx.beginPath();
                ctx.arc(7.6, c - 0.9, 0.9, 0, 2 * Math.PI);
                ctx.fillStyle = button.light; ctx.fill();
            }
        }
    }

    onPressedChanged: canvas.requestPaint()
    onToggledChanged: canvas.requestPaint()
    onEnabledChanged: canvas.requestPaint()
    onMaximizedChanged: canvas.requestPaint()
    onSizeChanged: canvas.requestPaint()
    // Redraw when the colors change (switching between FreeLook and FreeLook
    // Night while windows are open).
    onBaseChanged: canvas.requestPaint()
    onLightChanged: canvas.requestPaint()
    onDarkChanged: canvas.requestPaint()
    onInkChanged: canvas.requestPaint()
    onInkDisabledChanged: canvas.requestPaint()
}
