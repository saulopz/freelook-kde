// Top bar: [Workspace ▽] [task buttons] ........ [pager] [tray] [clock]
removePanels();
var top = makePanel("top", 30);
addWorkspaceMenu(top, true);
addTaskButtons(top);
addRightSide(top, true);
