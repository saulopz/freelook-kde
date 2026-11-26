// Test stub that mimics Aurorae's Borders class (C++).
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
QtObject {
    property int left: 0; property int right: 0; property int top: 0; property int bottom: 0
    function setAllBorders(v) { setBorders(v); setTitle(v) }
    function setBorders(v) { setSideBorders(v); bottom = v }
    function setSideBorders(v) { left = v; right = v }
    function setTitle(v) { top = v }
}
