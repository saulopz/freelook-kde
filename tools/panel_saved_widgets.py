#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Read the settings of some panel widgets from a Plasma applets file.

Usage: panel_saved_widgets.py <plasma-org.kde.plasma.desktop-appletsrc>

Prints a line of JavaScript, `var savedWidgets = {...};`, with the settings of
the first clock and the first weather widget found on a panel, so the
experimental layouts (plasma/panels/*.js) can recreate them as they were.
"""
import json
import re
import sys

WANTED = ("org.kde.plasma.digitalclock", "org.kde.plasma.weather")
GROUP = re.compile(r"^\[(.+)\]$")


def read_groups(path):
    groups, current = {}, None
    with open(path, encoding="utf-8", errors="replace") as f:
        for raw in f:
            line = raw.rstrip("\n")
            m = GROUP.match(line)
            if m:
                current = tuple(m.group(1).split("]["))
                groups.setdefault(current, {})
            elif current is not None and "=" in line and not line.startswith("#"):
                key, value = line.split("=", 1)
                groups[current][key] = value
    return groups


def main(path):
    groups = read_groups(path)
    panels = {g[1] for g, keys in groups.items()
              if len(g) == 2 and g[0] == "Containments" and keys.get("plugin") == "org.kde.panel"}
    saved = {}
    for g, keys in groups.items():
        # [Containments][N][Applets][M] with plugin=...
        if (len(g) == 4 and g[0] == "Containments" and g[1] in panels and g[2] == "Applets"
                and keys.get("plugin") in WANTED and keys["plugin"] not in saved):
            config = {}
            for sub, values in groups.items():
                # [Containments][N][Applets][M][Configuration][Group...]
                if len(sub) > 5 and sub[:4] == g and sub[4] == "Configuration" and values:
                    config["/".join(sub[5:])] = values
            saved[keys["plugin"]] = config
    print("var savedWidgets = " + json.dumps(saved, ensure_ascii=False) + ";")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__.strip(), file=sys.stderr)
        sys.exit(2)
    main(sys.argv[1])
