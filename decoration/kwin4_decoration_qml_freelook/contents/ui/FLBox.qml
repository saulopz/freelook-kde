/*
    FreeLook: the raised 3D box of OPEN LOOK buttons.

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick

Item {
    id: box
    property bool sunken: false
    property color base: "#cccccc"
    property color light: "#ffffff"
    property color dark: "#808080"

    width: 14
    height: 13

    Rectangle {
        anchors.fill: parent
        color: box.sunken ? Qt.darker(box.base, 1.11) : box.base
    }
    Rectangle { x: 0; y: 0; width: box.width; height: 1; color: box.sunken ? box.dark : box.light }
    Rectangle { x: 0; y: 0; width: 1; height: box.height; color: box.sunken ? box.dark : box.light }
    Rectangle { x: 0; y: box.height - 1; width: box.width; height: 1; color: box.sunken ? box.light : box.dark }
    Rectangle { x: box.width - 1; y: 0; width: 1; height: box.height; color: box.sunken ? box.light : box.dark }
}
