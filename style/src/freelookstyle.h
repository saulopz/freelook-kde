// FreeLook: a Qt 6 widget style inspired by OPEN LOOK (Sun OpenWindows).
//
// It is a layer on top of Fusion (QProxyStyle): anything not redrawn here is
// still painted by Fusion, so every application keeps working.
//
// SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QProxyStyle>

class QPainter;
class QPainterPath;
class QStyleOptionButton;
class QStyleOptionSlider;
class QStyleOptionTab;
class QStyleOptionSpinBox;
class QStyleOptionComboBox;

class FreeLookStyle : public QProxyStyle
{
    Q_OBJECT

public:
    FreeLookStyle();

    QPalette standardPalette() const override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget = nullptr) const override;

    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                         SubControl subControl, const QWidget *widget = nullptr) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option,
                         const QWidget *widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;

    void polish(QWidget *widget) override;

private:
    // Colors derived from a palette (the base is the button color: #cccccc in the FreeLook scheme).
    struct Colors {
        QColor base, pressed, softShadow, dark, light, ink;
    };
    static Colors colors(const QPalette &palette, bool enabled = true,
                         QPalette::ColorRole baseRole = QPalette::Button);

    // Reusable drawing pieces.
    static void bevel(QPainter *p, const QRect &r, const QColor &light, const QColor &shadow);
    static void box3D(QPainter *p, const QRect &r, const Colors &c, bool sunken);
    static void triangle(QPainter *p, const QRectF &r, Qt::ArrowType direction,
                         const QColor &ink, const QColor &fill);
    static QPainterPath oblong(const QRectF &r);

    void drawButton(const QStyleOptionButton *opt, QPainter *p) const;
    void drawTab(const QStyleOptionTab *opt, QPainter *p) const;
    void drawScrollBar(const QStyleOptionSlider *opt, QPainter *p) const;
    QRect scrollBarRect(const QStyleOptionSlider *opt, SubControl sc) const;
    void drawSpinBox(const QStyleOptionSpinBox *opt, QPainter *p) const;
    QRect spinBoxRect(const QStyleOptionSpinBox *opt, SubControl sc) const;
    void drawComboBox(const QStyleOptionComboBox *opt, QPainter *p) const;
    QRect comboBoxRect(const QStyleOptionComboBox *opt, SubControl sc) const;
};
