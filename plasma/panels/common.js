// SPDX-License-Identifier: GPL-3.0-or-later
// FreeLook experimental panels: shared helpers (prepended to each layout by
// tools/try-panel.sh before it is sent to Plasma's scripting console).
//
// tools/try-panel.sh also prepends `var savedWidgets = {...}`: the settings of
// the clock and weather widgets of your own panels, so they come back as they were.
if (typeof savedWidgets === "undefined") {
    var savedWidgets = {};
}

// Adds a widget and, if it was on your own panels, its settings.
function addSaved(panel, plugin) {
    var w = panel.addWidget(plugin);
    var groups = savedWidgets[plugin] || {};
    for (var group in groups) {
        w.currentConfigGroup = group.split("/");
        for (var key in groups[group]) {
            w.writeConfig(key, groups[group][key]);
        }
    }
    return w;
}

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
// opens on the favorites. Kicker in Plasma 6.8 has no text label, so the
// button is a picture of an OPEN LOOK menu button (workspace-button.svg,
// copied by tools/try-panel.sh, which sets `workspaceButton` to its path).
function addWorkspaceMenu(panel, withLabel) {
    var w = panel.addWidget("org.kde.plasma.kicker");
    w.currentConfigGroup = ["General"];
    w.writeConfig("showRecentApps", false);
    w.writeConfig("showRecentDocs", false);
    if (withLabel && typeof workspaceButton !== "undefined") {
        w.writeConfig("useCustomButtonImage", true);
        w.writeConfig("customButtonImage", workspaceButton);
    }
    return w;
}

// The same menu with Kickoff, the default launcher: it opens on the favorites
// as a list (no cascading submenus), but its text label exists in Plasma 6.8
// and follows the color scheme, by day and by night.
function addWorkspaceKickoff(panel) {
    var w = panel.addWidget("org.kde.plasma.kickoff");
    w.currentConfigGroup = ["General"];
    w.writeConfig("menuLabel", "Workspace");
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
    addSaved(panel, "org.kde.plasma.digitalclock");
    if (savedWidgets["org.kde.plasma.weather"]) {
        addSaved(panel, "org.kde.plasma.weather");
    }
}
