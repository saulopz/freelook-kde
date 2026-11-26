// Test scene: an OpenWindows desktop with a few FreeLook windows.
//   QT_QPA_PLATFORM=offscreen qml -I stubs scene.qml
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick

Rectangle {
    id: scene
    width: 860
    height: 520
    color: "#40a0c0"
    property string output: "preview_decoration.png"

    TestWindow {
        x: 16; y: 14
        title: "Snapshot V3"
        active: false
        pinned: true
        clientWidth: 330; clientHeight: 96
        content: Component {
            Item {
                Text { x: 14; y: 16; text: "Snap type:"; font.bold: true; font.pixelSize: 12 }
                Row {
                    x: 110; y: 12
                    Repeater {
                        model: ["Window", "Region", "Screen"]
                        Rectangle {
                            width: 62; height: 20; border.color: "black"
                            color: index === 0 ? "#b7b7b7" : "#cccccc"
                            Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 12 }
                        }
                    }
                }
            }
        }
    }
    TestWindow {
        x: 200; y: 150
        title: "Terminal: saulo@lain"
        titleFontSize: 15
        clientWidth: 400; clientHeight: 180
        content: Component {
            Rectangle {
                color: "white"
                Column {
                    x: 8; y: 6
                    Repeater {
                        model: ["$ uname -a", "Linux lain x86_64 GNU/Linux", "$ echo 'hello, OpenWindows'", "hello, OpenWindows", "$ _"]
                        Text { text: modelData; font.family: "DejaVu Sans Mono"; font.pixelSize: 12 }
                    }
                }
            }
        }
    }
    TestWindow {
        x: 0; y: 420
        title: "Maximized (no borders, no corners)"
        maximized: true
        leftButtons: [5]; rightButtons: [10, 9, 3, 4, 6, 7]
        clientWidth: 860; clientHeight: 40
        content: Component { Rectangle { color: "white" } }
    }
    TestWindow {
        x: 630; y: 20
        title: "Large border"
        clientWidth: 200; clientHeight: 80
        leftButtons: [1, 5]; rightButtons: [8, 6, 7]
        Component.onCompleted: decorationSettings.borderSize = 4
    }

    Timer {
        interval: 700; running: true
        onTriggered: scene.grabToImage(function(r) { r.saveToFile(scene.output); console.log("saved", scene.output); Qt.quit() })
    }
}
