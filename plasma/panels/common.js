// SPDX-License-Identifier: GPL-3.0-or-later
// FreeLook experimental panels: shared helpers (prepended to each layout by
// tools/try-panel.sh before it is sent to Plasma's scripting console).

// Removes every panel; the desktop and its widgets stay.
function removePanels() {
    panels().forEach(function (p) { p.remove(); });
}

// A panel that does not float and never hides.
function makePanel(location, thickness) {
    var p = new Panel;
    p.location = location;
    p.height = thickness;
    p.floating = false;
    p.hiding = "none";
    return p;
}

// The "Workspace ▽" menu: Kicker (the cascading application menu), which
// opens on the favorites, labelled like the OPEN LOOK Workspace menu.
function addWorkspaceMenu(panel, withLabel) {
    var w = panel.addWidget("org.kde.plasma.kicker");
    w.currentConfigGroup = ["General"];
    w.writeConfig("showRecentApps", false);
    w.writeConfig("showRecentDocs", false);
    if (withLabel) {
        w.writeConfig("menuLabel", "Workspace");
    }
    return w;
}

// "Janelas ▽": one button with a menu of the open windows.
function addWindowMenu(panel) {
    return panel.addWidget("org.kde.plasma.windowlist");
}

function addTaskButtons(panel) {
    var w = panel.addWidget("org.kde.plasma.icontasks");
    w.currentConfigGroup = ["General"];
    w.writeConfig("launchers", "");   // only open windows, no pinned launchers
    return w;
}

function addRightSide(panel, withPager) {
    panel.addWidget("org.kde.plasma.panelspacer");
    if (withPager) {
        panel.addWidget("org.kde.plasma.pager");
    }
    panel.addWidget("org.kde.plasma.systemtray");
    panel.addWidget("org.kde.plasma.digitalclock");
}
