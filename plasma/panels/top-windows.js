// Top bar: [Workspace ▽] [Janelas ▽] ........ [pager] [tray] [clock]
removePanels();
var top = makePanel("top", 30);
addWorkspaceMenu(top, true);
addWindowMenu(top);
addRightSide(top, true);
