// One column on the right: menu, task buttons, then pager, tray and clock.
removePanels();
var side = makePanel("right", 44);
addWorkspaceMenu(side, false);
addTaskButtons(side);
addRightSide(side, true);
