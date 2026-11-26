// Test stub that mimics Aurorae's DecorationOptions (C++), enums in the original order.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
QtObject {
    enum BorderSize { BorderNone, BorderNoSides, BorderTiny, BorderNormal, BorderLarge,
                      BorderVeryLarge, BorderHuge, BorderVeryHuge, BorderOversized }
    enum DecorationButton { DecorationButtonNone, DecorationButtonMenu, DecorationButtonApplicationMenu,
        DecorationButtonOnAllDesktops, DecorationButtonQuickHelp, DecorationButtonMinimize,
        DecorationButtonMaximizeRestore, DecorationButtonClose, DecorationButtonKeepAbove,
        DecorationButtonKeepBelow, DecorationButtonShade, DecorationButtonResize,
        DecorationButtonExplicitSpacer }
    property var deco
    property color titleBarColor: "#cccccc"
    property color fontColor: deco && deco.client.active ? "#000000" : "#606060"
    property font titleFont: Qt.font({family: "DejaVu Sans", bold: true,
                                      pixelSize: deco && deco.titleFontSize ? deco.titleFontSize : 12})
    property var titleButtonsLeft: deco ? deco.leftButtons : []
    property var titleButtonsRight: deco ? deco.rightButtons : []
    property int mousePressAndHoldInterval: 500
}
