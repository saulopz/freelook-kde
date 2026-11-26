// A test window: mimics KWin's "decoration" object and loads the theme's main.qml.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Item {
    id: window
    property string title: "Window"
    property bool active: true
    property bool maximized: false
    property bool pinned: false
    property var leftButtons: [5]        // Minimize
    property var rightButtons: [8, 7]    // KeepAbove, Close
    property int clientWidth: 300
    property int clientHeight: 120
    property int titleFontSize: 12
    property Component content
    property alias theme: loader.item

    // Context that Aurorae offers to the theme
    property QtObject decorationSettings: QtObject {
        property int borderSize: 3          // BorderNormal
        property bool closeOnDoubleClickOnMenu: false
    }
    property QtObject decoration: QtObject {
        property int titleFontSize: window.titleFontSize
        property var leftButtons: window.leftButtons
        property var rightButtons: window.rightButtons
        property QtObject client: QtObject {
            property string caption: window.title
            property bool active: window.active
            property bool maximized: window.maximized
            property bool shaded: false
            property bool keepAbove: window.pinned
            property bool keepBelow: false
            property bool onAllDesktops: false
            property bool closeable: true
            property bool maximizeable: true
            property bool minimizeable: true
            property int width: window.clientWidth
            property int height: window.clientHeight
        }
        readonly property var b: loader.item ? (window.maximized ? loader.item.maximizedBorders : loader.item.borders) : null
        property int borderLeft: b ? b.left : 0
        property int borderRight: b ? b.right : 0
        property int borderTop: b ? b.top : 0
        property int borderBottom: b ? b.bottom : 0
        function installTitleItem(i) {}
        function readConfig(k, d) { return d }
        function requestMinimize() {} function requestClose() {} function requestShowWindowMenu() {}
        function requestToggleKeepAbove() {} function requestToggleMaximization(b) {}
    }

    width: loader.item ? loader.item.width : 0
    height: loader.item ? loader.item.height : 0

    Loader {
        id: loader
        source: Qt.resolvedUrl("../../decoration/kwin4_decoration_qml_freelook/contents/ui/main.qml")
        onStatusChanged: if (status === Loader.Error) console.log("ERROR loading the theme")
    }
    // Application area, positioned by the borders the theme declared
    Loader {
        x: window.decoration.borderLeft
        y: window.decoration.borderTop
        width: window.clientWidth
        height: window.clientHeight
        sourceComponent: window.content
    }
}
