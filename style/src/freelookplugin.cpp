// FreeLook: plugin that registers the style under the name "FreeLook".
//
// SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "freelookstyle.h"

#include <QStylePlugin>

class FreeLookStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "freelook.json")

public:
    QStyle *create(const QString &key) override
    {
        if (key.compare(QLatin1String("freelook"), Qt::CaseInsensitive) == 0) {
            return new FreeLookStyle;
        }
        return nullptr;
    }
};

#include "freelookplugin.moc"
