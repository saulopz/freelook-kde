/*
    FreeLook: the OPEN LOOK L-shaped resize corner.

    The L is as thick as the border and its arms are longer, running along
    the edges. It is raised, lit from the top left: edges facing up or left
    are light, the others are dark.

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick

Item {
    id: corner
    property int thickness: 5
    property int armLength: 12
    property bool mirrorX: false   // false = left corner
    property bool mirrorY: false   // false = top corner
    property color base: "#cccccc"
    property color light: "#ffffff"
    property color dark: "#808080"

    width: armLength
    height: armLength

    function pieces() {
        var t = thickness, a = armLength;
        // Edges of the top-left corner: [x, y, width, height, normalX, normalY]
        var edges = [
            [0, 0, a, 1, 0, -1],          // top
            [0, 0, 1, a, -1, 0],          // left side
            [a - 1, 0, 1, t, 1, 0],       // end of the horizontal arm
            [t, t - 1, a - t, 1, 0, 1],   // inner step, bottom
            [t - 1, t, 1, a - t, 1, 0],   // inner step, right
            [0, a - 1, t, 1, 0, 1]        // end of the vertical arm
        ];
        var result = [
            {x: 0, y: 0, w: a, h: t, c: base},
            {x: 0, y: 0, w: t, h: a, c: base}
        ];
        var lit = [], shaded = [];
        for (var i = 0; i < edges.length; i++) {
            var e = edges[i];
            var x = e[0], y = e[1], w = e[2], h = e[3], nx = e[4], ny = e[5];
            if (mirrorX) { x = a - x - w; nx = -nx; }
            if (mirrorY) { y = a - y - h; ny = -ny; }
            var facesLight = (ny === -1 || nx === -1);
            (facesLight ? lit : shaded).push({x: x, y: y, w: w, h: h, c: facesLight ? light : dark});
        }
        // Shaded edges last: where both meet, the shadow wins.
        return result.concat(lit, shaded);
    }

    Repeater {
        model: corner.pieces()
        Rectangle {
            x: modelData.x
            y: modelData.y
            width: modelData.w
            height: modelData.h
            color: modelData.c
            antialiasing: false
        }
    }
}
