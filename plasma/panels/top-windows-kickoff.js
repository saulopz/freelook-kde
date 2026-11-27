// Top bar with Kickoff: [Workspace] [Janelas ▽] ........ [pager] [tray] [clock]
removePanels();
var top = makePanel("top", 30);
addWorkspaceKickoff(top);
addWindowMenu(top);
addRightSide(top, true);
