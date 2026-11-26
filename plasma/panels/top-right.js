// Top bar for menus and status: [Workspace ▽] [Janelas ▽] ...... [tray] [clock]
// Right column for the work: task buttons and the pager.
removePanels();
var top = makePanel("top", 30);
addWorkspaceMenu(top, true);
addWindowMenu(top);
addRightSide(top, false);
var side = makePanel("right", 44);
addTaskButtons(side);
side.addWidget("org.kde.plasma.panelspacer");
side.addWidget("org.kde.plasma.pager");
