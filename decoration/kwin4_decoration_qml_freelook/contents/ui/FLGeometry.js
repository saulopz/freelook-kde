/*
    FreeLook: scaled button glyphs that stay crisp.

    Glyphs were designed on a 14 x 13 px reference box (top-left corner at
    x = 2, y = 3). These helpers map those coordinates onto the real box,
    snapping strokes to pixel centers and fills to whole pixels.

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
.pragma library

// box: {x, y, w, h}; offset: 1 px shift while the button is pressed
function mapper(box, offset) {
    var sx = box.w / 14, sy = box.h / 13;
    var X = function (x) { return box.x + (x - 2) * sx + offset; };
    var Y = function (y) { return box.y + (y - 3) * sy + offset; };
    // Thin strokes, as in OPEN LOOK: they only get thicker on very large buttons.
    var lineWidth = Math.max(1, Math.floor(Math.min(sx, sy)));
    // Odd widths are crisp on pixel centers (x.5); even widths on whole pixels.
    var snap = (lineWidth % 2 === 1) ? function (v) { return Math.floor(v) + 0.5; }
                                     : function (v) { return Math.round(v); };
    return {
        sx: sx, sy: sy,
        lineWidth: lineWidth,
        // point for strokes
        px: function (x) { return snap(X(x)); },
        py: function (y) { return snap(Y(y)); },
        // outlined rectangle
        strokeRect: function (ctx, x, y, w, h) {
            var x0 = snap(X(x)), y0 = snap(Y(y)), x1 = snap(X(x + w)), y1 = snap(Y(y + h));
            ctx.strokeRect(x0, y0, x1 - x0, y1 - y0);
        },
        // filled rectangle
        fillRect: function (ctx, x, y, w, h) {
            var x0 = Math.round(X(x)), y0 = Math.round(Y(y));
            var x1 = Math.max(x0 + 1, Math.round(X(x + w))), y1 = Math.max(y0 + 1, Math.round(Y(y + h)));
            ctx.fillRect(x0, y0, x1 - x0, y1 - y0);
        }
    };
}
