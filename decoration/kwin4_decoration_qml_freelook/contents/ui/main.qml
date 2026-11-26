/*
    FreeLook: a KWin window decoration inspired by OPEN LOOK
    (Sun OpenWindows, 1989-1993).

    Thin border with a black outline, L-shaped resize corners, window menu
    button with a triangle, centered title and the pushpin.

    SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick
import org.kde.kwin.decoration

Decoration {
    id: root

    // ---------------------------------------------------------------- palette
    // The base color comes from the color scheme (Colors -> title bar).
    property color base: options.titleBarColor
    property color light: "#ffffff"
    property color dark: Qt.darker(base, 1.6)          // #cccccc -> #7f7f7f
    property color softShadow: Qt.darker(base, 1.27)   // #cccccc -> #a0a0a0
    property color outline: "#000000"

    // ---------------------------------------------------------------- metrics
    property int borderSize: 5
    property int armLength: borderSize * 2 + 2
    // The title bar follows the title font, and the buttons follow the title bar.
    // For bigger or smaller buttons, change the padding (px) above and below the text.
    property int titlePadding: 4
    property int titleHeight: Math.max(22, Math.ceil(caption.implicitHeight) + 2 * titlePadding)
    property int buttonSize: titleHeight - 2
    property int separatorHeight: 2                    // etched line under the title
    readonly property bool maximized: decoration.client.maximized
    readonly property bool active: decoration.client.active

    alpha: false

    DecorationOptions {
        id: options
        deco: decoration
    }

    function readBorderSize() {
        switch (decorationSettings.borderSize) {
        case DecorationOptions.BorderNone:
        case DecorationOptions.BorderNoSides:
            root.borderSize = 1; break;
        case DecorationOptions.BorderTiny:
            root.borderSize = 3; break;
        case DecorationOptions.BorderLarge:
            root.borderSize = 7; break;
        case DecorationOptions.BorderVeryLarge:
            root.borderSize = 9; break;
        case DecorationOptions.BorderHuge:
        case DecorationOptions.BorderVeryHuge:
        case DecorationOptions.BorderOversized:
            root.borderSize = 12; break;
        default:
            root.borderSize = 5;
        }
        applyBorders();
    }

    function applyBorders() {
        borders.setBorders(root.borderSize);
        borders.setTitle(root.borderSize + root.titleHeight + root.separatorHeight);
        maximizedBorders.setBorders(0);
        maximizedBorders.setTitle(root.titleHeight + root.separatorHeight);
        // With minimal borders, an invisible extra area makes them easier to grab.
        extendedBorders.setAllBorders(root.borderSize < 3 ? 4 : 0);
        extendedBorders.setTitle(0);
    }

    onTitleHeightChanged: applyBorders()

    // ---------------------------------------------------------------- background and outline
    Rectangle {
        anchors.fill: parent
        color: root.base
        border.width: root.maximized ? 0 : 1
        border.color: root.outline
    }

    // ---------------------------------------------------------------- title bar
    Item {
        id: titleBar
        x: root.maximized ? 0 : root.borderSize
        y: root.maximized ? 0 : root.borderSize
        width: parent.width - 2 * x
        height: root.titleHeight

        ButtonGroup {
            id: leftButtons
            x: 2
            y: Math.round((root.titleHeight - root.buttonSize) / 2)
            spacing: 3
            explicitSpacer: root.buttonSize
            menuButton: menuComponent
            appMenuButton: menuComponent
            minimizeButton: minimizeComponent
            maximizeButton: maximizeComponent
            keepBelowButton: keepBelowComponent
            keepAboveButton: keepAboveComponent
            helpButton: helpComponent
            shadeButton: shadeComponent
            allDesktopsButton: allDesktopsComponent
            closeButton: closeComponent
            buttons: options.titleButtonsLeft
        }

        Text {
            id: caption
            // Centered on the whole window, not just between the buttons:
            // reserve the width of the wider button group on both sides.
            property real reserve: Math.max(leftButtons.width, rightButtons.width) + 8
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: reserve
                rightMargin: reserve
                verticalCenter: parent.verticalCenter
            }
            horizontalAlignment: Text.AlignHCenter
            text: decoration.client.caption
            textFormat: Text.PlainText
            font: options.titleFont
            color: options.fontColor
            elide: Text.ElideMiddle
            renderType: Text.NativeRendering
        }

        ButtonGroup {
            id: rightButtons
            anchors.right: parent.right
            anchors.rightMargin: 2
            y: leftButtons.y
            spacing: 3
            explicitSpacer: root.buttonSize
            menuButton: menuComponent
            appMenuButton: menuComponent
            minimizeButton: minimizeComponent
            maximizeButton: maximizeComponent
            keepBelowButton: keepBelowComponent
            keepAboveButton: keepAboveComponent
            helpButton: helpComponent
            shadeButton: shadeComponent
            allDesktopsButton: allDesktopsComponent
            closeButton: closeComponent
            buttons: options.titleButtonsRight
        }

        Component.onCompleted: decoration.installTitleItem(titleBar)
    }

    // Etched line under the title (dark + light); on inactive windows just the soft one.
    Rectangle {
        x: titleBar.x
        y: titleBar.y + titleBar.height
        width: titleBar.width
        height: 1
        color: root.active ? root.dark : root.softShadow
    }
    Rectangle {
        x: titleBar.x
        y: titleBar.y + titleBar.height + 1
        width: titleBar.width
        height: 1
        color: root.active ? root.light : root.base
    }

    // ---------------------------------------------------------------- L-shaped corners
    Repeater {
        model: [
            {mx: false, my: false},
            {mx: true,  my: false},
            {mx: false, my: true},
            {mx: true,  my: true}
        ]
        FLCorner {
            visible: !root.maximized && root.borderSize >= 3
            thickness: root.borderSize
            armLength: root.armLength
            mirrorX: modelData.mx
            mirrorY: modelData.my
            x: modelData.mx ? root.width - width : 0
            y: modelData.my ? root.height - height : 0
            base: root.base
            light: root.light
            dark: root.dark
        }
    }

    // ---------------------------------------------------------------- buttons
    Component {
        id: menuComponent
        FLWindowMenuButton {
            size: root.buttonSize
            alwaysMenu: true
            base: root.base; light: root.light; dark: root.dark
        }
    }
    Component {
        id: minimizeComponent
        FLWindowMenuButton {
            size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark
        }
    }
    Component {
        id: maximizeComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonMaximizeRestore
            glyph: "maximize"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: closeComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonClose
            glyph: "close"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: keepAboveComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonKeepAbove
            glyph: "pin"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: keepBelowComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonKeepBelow
            glyph: "below"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: shadeComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonShade
            glyph: "shade"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: allDesktopsComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonOnAllDesktops
            glyph: "alldesktops"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }
    Component {
        id: helpComponent
        FLButton {
            buttonType: DecorationOptions.DecorationButtonQuickHelp
            glyph: "help"; size: root.buttonSize
            base: root.base; light: root.light; dark: root.dark; inkDisabled: root.softShadow
        }
    }

    Component.onCompleted: readBorderSize()

    Connections {
        target: decorationSettings
        function onBorderSizeChanged() { root.readBorderSize(); }
    }
}
