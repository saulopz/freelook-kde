// FreeLook: a Qt 6 widget style inspired by OPEN LOOK (Sun OpenWindows).
//
// SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "freelookstyle.h"

#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyleFactory>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

namespace
{
// OPEN LOOK metrics (logical pixels).
constexpr int kScrollBarExtent = 16;   // scroll bar thickness
constexpr int kAnchor = 6;             // length of the anchors at both ends
constexpr int kAnchorGap = 2;          // gap between an anchor and the cable
constexpr int kIndicator = 16;         // check box area (the check mark overflows inside it)
constexpr int kCheckBox = 12;          // the check box itself
}

FreeLookStyle::FreeLookStyle()
    : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion")))
{
    setObjectName(QStringLiteral("FreeLook"));
}

// ---------------------------------------------------------------------------- colors

FreeLookStyle::Colors FreeLookStyle::colors(const QPalette &pal, bool enabled, QPalette::ColorRole baseRole)
{
    Colors c;
    c.base = pal.color(baseRole);               // #cccccc
    c.pressed = c.base.darker(111);             // #b7b7b7
    c.softShadow = c.base.darker(127);          // #a0a0a0
    c.dark = c.base.darker(160);                // #7f7f7f
    c.light = QColor(255, 255, 255);
    c.ink = enabled ? pal.color(QPalette::ButtonText)
                    : pal.color(QPalette::Disabled, QPalette::ButtonText);
    return c;
}

QPalette FreeLookStyle::standardPalette() const
{
    // The OpenWindows 3 palette. Under Plasma the active color scheme wins.
    const QColor base(0xcc, 0xcc, 0xcc), black(0, 0, 0), white(255, 255, 255);
    QPalette p(base);
    p.setColor(QPalette::Window, base);
    p.setColor(QPalette::Button, base);
    p.setColor(QPalette::Base, white);
    p.setColor(QPalette::AlternateBase, QColor(0xf0, 0xf0, 0xf0));
    p.setColor(QPalette::WindowText, black);
    p.setColor(QPalette::Text, black);
    p.setColor(QPalette::ButtonText, black);
    p.setColor(QPalette::Highlight, QColor(0xa0, 0xa0, 0xa0));
    p.setColor(QPalette::HighlightedText, black);
    p.setColor(QPalette::Light, white);
    p.setColor(QPalette::Midlight, QColor(0xe0, 0xe0, 0xe0));
    p.setColor(QPalette::Mid, QColor(0xa0, 0xa0, 0xa0));
    p.setColor(QPalette::Dark, QColor(0x80, 0x80, 0x80));
    p.setColor(QPalette::Shadow, black);
    p.setColor(QPalette::ToolTipBase, QColor(0xea, 0xea, 0xea));
    p.setColor(QPalette::ToolTipText, black);
    p.setColor(QPalette::Link, QColor(0, 0, 0xcc));
    const QColor grayedOut(0x80, 0x80, 0x80);
    p.setColor(QPalette::Disabled, QPalette::WindowText, grayedOut);
    p.setColor(QPalette::Disabled, QPalette::Text, grayedOut);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, grayedOut);
    return p;
}

// ---------------------------------------------------------------------------- drawing pieces

void FreeLookStyle::bevel(QPainter *p, const QRect &r, const QColor &light, const QColor &shadow)
{
    // 1 px lines: light on top/left, shadow on bottom/right.
    p->fillRect(QRect(r.left(), r.top(), r.width(), 1), light);
    p->fillRect(QRect(r.left(), r.top(), 1, r.height()), light);
    p->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), shadow);
    p->fillRect(QRect(r.right(), r.top(), 1, r.height()), shadow);
}

void FreeLookStyle::box3D(QPainter *p, const QRect &r, const Colors &c, bool sunken)
{
    p->fillRect(r, sunken ? c.pressed : c.base);
    if (sunken) {
        bevel(p, r, c.dark, c.light);
    } else {
        bevel(p, r, c.light, c.dark);
    }
}

void FreeLookStyle::triangle(QPainter *p, const QRectF &r, Qt::ArrowType direction,
                             const QColor &ink, const QColor &fill)
{
    // OPEN LOOK hollow triangle, outlined through the middle of the pixels.
    const qreal l = std::floor(r.left()) + 0.5, t = std::floor(r.top()) + 0.5;
    const qreal rr = std::floor(r.right()) + 0.5, b = std::floor(r.bottom()) + 0.5;
    const qreal cx = (l + rr) / 2, cy = (t + b) / 2;
    QPolygonF poly;
    switch (direction) {
    case Qt::UpArrow:    poly << QPointF(l, b) << QPointF(rr, b) << QPointF(cx, t); break;
    case Qt::LeftArrow:  poly << QPointF(rr, t) << QPointF(rr, b) << QPointF(l, cy); break;
    case Qt::RightArrow: poly << QPointF(l, t) << QPointF(l, b) << QPointF(rr, cy); break;
    default:             poly << QPointF(l, t) << QPointF(rr, t) << QPointF(cx, b); break;
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QPen pen(ink, 1.0);
    pen.setJoinStyle(Qt::MiterJoin);
    p->setPen(pen);
    p->setBrush(fill);
    p->drawPolygon(poly);
    p->restore();
}

QPainterPath FreeLookStyle::oblong(const QRectF &r)
{
    // OPEN LOOK button: a rectangle with half-circle ends.
    QPainterPath path;
    const qreal radius = r.height() / 2.0;
    path.addRoundedRect(r, radius, radius);
    return path;
}

// ---------------------------------------------------------------------------- buttons

void FreeLookStyle::drawButton(const QStyleOptionButton *opt, QPainter *p) const
{
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(opt->palette, enabled);
    const bool sunken = (opt->state & (State_Sunken | State_On));
    const QRectF r = QRectF(opt->rect).adjusted(0.5, 0.5, -0.5, -0.5);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    // Fill and outline.
    p->setPen(QPen(enabled ? c.dark : c.softShadow, 1.0));
    p->setBrush(sunken ? c.pressed : c.base);
    p->drawPath(oblong(r));

    // Relief: top half light, bottom half dark (swapped when pressed).
    const QPainterPath inner = oblong(r.adjusted(1, 1, -1, -1));
    const QRectF topHalf(r.left(), r.top(), r.width(), r.height() / 2.0);
    const QRectF bottomHalf(r.left(), r.center().y(), r.width(), r.height() / 2.0 + 1);
    p->setBrush(Qt::NoBrush);
    p->setClipRect(topHalf);
    p->setPen(QPen(sunken ? c.softShadow : c.light, 1.0));
    p->drawPath(inner);
    p->setClipRect(bottomHalf);
    p->setPen(QPen(sunken ? c.light : c.softShadow, 1.0));
    p->drawPath(inner);
    p->setClipping(false);

    // Default button: an inner ring, as in OPEN LOOK.
    if (opt->features & QStyleOptionButton::DefaultButton) {
        p->setPen(QPen(c.ink, 1.0));
        p->drawPath(oblong(r.adjusted(2, 2, -2, -2)));
    }
    // Keyboard focus: a dotted ring.
    if ((opt->state & State_HasFocus) && !(opt->features & QStyleOptionButton::DefaultButton)) {
        p->setPen(QPen(c.ink, 1.0, Qt::DotLine));
        p->drawPath(oblong(r.adjusted(2, 2, -2, -2)));
    }
    p->restore();

    // Menu button: hollow triangle on the right (as in "Print ▽").
    if (opt->features & QStyleOptionButton::HasMenu) {
        const int side = std::max(7, opt->rect.height() / 3);
        const int radius = opt->rect.height() / 2;
        QRectF tri(opt->rect.right() - radius - side + 2,
                   opt->rect.center().y() - side * 0.3,
                   side, side * 0.6);
        triangle(p, tri, Qt::DownArrow, c.ink, sunken ? c.pressed : c.base);
    }
}

// ---------------------------------------------------------------------------- elevator scroll bar
//
//  [anchor] ==cable== [ ▲ | elevator | ▼ ] ==cable== [anchor]
//
// The arrows live inside the fixed-size elevator. The anchors jump to the
// start and to the end. The dark stretch of the cable shows how much of the
// content is visible.

QRect FreeLookStyle::scrollBarRect(const QStyleOptionSlider *opt, SubControl sc) const
{
    const bool horizontal = opt->orientation == Qt::Horizontal;
    const QRect r = opt->rect;
    const int length = horizontal ? r.width() : r.height();
    const int thickness = horizontal ? r.height() : r.width();

    const int anchor = std::min(kAnchor, length / 6);
    const int cableStart = anchor + kAnchorGap;
    const int cableLength = std::max(0, length - 2 * cableStart);

    int cell = thickness;
    if (3 * cell > cableLength) {
        cell = std::max(1, cableLength / 3);
    }
    const int elevator = 3 * cell;
    const int travel = std::max(0, cableLength - elevator);
    const int pos = sliderPositionFromValue(opt->minimum, opt->maximum, opt->sliderPosition,
                                            travel, opt->upsideDown);
    const int elevatorStart = cableStart + pos;

    int a = 0, n = 0;   // start and length along the axis
    switch (sc) {
    case SC_ScrollBarFirst:   a = 0;                           n = anchor; break;
    case SC_ScrollBarLast:    a = length - anchor;             n = anchor; break;
    case SC_ScrollBarGroove:  a = cableStart;                  n = cableLength; break;
    case SC_ScrollBarSlider:  a = elevatorStart;               n = elevator; break;
    case SC_ScrollBarSubLine: a = elevatorStart;               n = cell; break;
    case SC_ScrollBarAddLine: a = elevatorStart + 2 * cell;    n = cell; break;
    case SC_ScrollBarSubPage: a = cableStart;                  n = elevatorStart - cableStart; break;
    case SC_ScrollBarAddPage: a = elevatorStart + elevator;    n = cableStart + cableLength - a; break;
    default: return QRect();
    }
    const QRect result = horizontal ? QRect(r.x() + a, r.y(), n, thickness)
                                    : QRect(r.x(), r.y() + a, thickness, n);
    return visualRect(opt->direction, r, result);
}

void FreeLookStyle::drawScrollBar(const QStyleOptionSlider *opt, QPainter *p) const
{
    const bool horizontal = opt->orientation == Qt::Horizontal;
    const bool enabled = opt->state & State_Enabled;
    // The elevator takes the gray from the application palette (the color scheme).
    // Kirigami QML lists hand over an all-white local palette; that one only
    // paints the channel background.
    const Colors c = colors(QApplication::palette(), enabled, QPalette::Window);
    const bool sunken = opt->state & State_Sunken;
    auto isPressed = [&](SubControl sc) { return sunken && (opt->activeSubControls & sc); };

    p->fillRect(opt->rect, opt->palette.color(QPalette::Window));

    // Cable: a 3 px line down the middle, with the visible stretch in dark.
    const QRect cable = scrollBarRect(opt, SC_ScrollBarGroove);
    const QRect elev = scrollBarRect(opt, SC_ScrollBarSlider);
    if (cable.isValid()) {
        QRect line = horizontal ? QRect(cable.left(), cable.center().y() - 1, cable.width(), 3)
                                : QRect(cable.center().x() - 1, cable.top(), 3, cable.height());
        p->fillRect(line, c.softShadow);
        const int total = opt->maximum - opt->minimum + opt->pageStep;
        if (total > 0 && opt->maximum > opt->minimum) {
            const int cableLength = horizontal ? cable.width() : cable.height();
            const int stretch = std::max(2, int(qint64(cableLength) * opt->pageStep / total));
            if (horizontal) {
                int x = std::clamp(elev.center().x() - stretch / 2, cable.left(), cable.right() - stretch + 1);
                p->fillRect(QRect(x, line.top(), stretch, 3), c.dark);
            } else {
                int y = std::clamp(elev.center().y() - stretch / 2, cable.top(), cable.bottom() - stretch + 1);
                p->fillRect(QRect(line.left(), y, 3, stretch), c.dark);
            }
        }
    }

    // Anchors at both ends.
    for (SubControl sc : {SC_ScrollBarFirst, SC_ScrollBarLast}) {
        QRect a = scrollBarRect(opt, sc);
        a = horizontal ? a.adjusted(0, 1, 0, -1) : a.adjusted(1, 0, -1, 0);
        if (a.isValid()) {
            box3D(p, a, c, isPressed(sc));
        }
    }

    if (!elev.isValid() || opt->maximum == opt->minimum) {
        return;   // nothing to scroll: just the cable and the anchors
    }

    // Elevator: three raised cells.
    const QRect e = horizontal ? elev.adjusted(0, 1, 0, -1) : elev.adjusted(1, 0, -1, 0);
    const QRect sub = scrollBarRect(opt, SC_ScrollBarSubLine);
    const QRect add = scrollBarRect(opt, SC_ScrollBarAddLine);
    const QRect cell1 = horizontal ? QRect(sub.left(), e.top(), sub.width(), e.height())
                                   : QRect(e.left(), sub.top(), e.width(), sub.height());
    const QRect cell3 = horizontal ? QRect(add.left(), e.top(), add.width(), e.height())
                                   : QRect(e.left(), add.top(), e.width(), add.height());
    const QRect cell2 = horizontal ? QRect(cell1.right() + 1, e.top(), cell3.left() - cell1.right() - 1, e.height())
                                   : QRect(e.left(), cell1.bottom() + 1, e.width(), cell3.top() - cell1.bottom() - 1);

    box3D(p, cell1, c, isPressed(SC_ScrollBarSubLine));
    box3D(p, cell2, c, isPressed(SC_ScrollBarSlider));
    box3D(p, cell3, c, isPressed(SC_ScrollBarAddLine));
    p->save();
    p->setPen(c.dark);
    p->setBrush(Qt::NoBrush);
    p->drawRect(e.adjusted(0, 0, -1, -1));
    p->restore();

    // Arrows: grayed out when already at the start or at the end.
    const bool atStart = opt->sliderPosition <= opt->minimum;
    const bool atEnd = opt->sliderPosition >= opt->maximum;
    auto arrow = [&](const QRect &cell, Qt::ArrowType dir, bool grayed, bool pressed) {
        const int side = std::max(5, std::min(cell.width(), cell.height()) / 2 + 1);
        QRectF tri = (dir == Qt::UpArrow || dir == Qt::DownArrow)
                         ? QRectF(0, 0, side, std::ceil(side * 0.6))
                         : QRectF(0, 0, std::ceil(side * 0.6), side);
        tri.moveCenter(QRectF(cell).center() + (pressed ? QPointF(1, 1) : QPointF(0, 0)));
        triangle(p, tri, dir, (grayed || !enabled) ? c.softShadow : c.ink,
                 pressed ? c.pressed : c.base);
    };
    const bool reversed = opt->upsideDown != (horizontal && opt->direction == Qt::RightToLeft);
    arrow(cell1, horizontal ? Qt::LeftArrow : Qt::UpArrow, reversed ? atEnd : atStart,
          isPressed(SC_ScrollBarSubLine));
    arrow(cell3, horizontal ? Qt::RightArrow : Qt::DownArrow, reversed ? atStart : atEnd,
          isPressed(SC_ScrollBarAddLine));
}

// ---------------------------------------------------------------------------- QStyle

void FreeLookStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_PanelButtonCommand:
        if (const auto *b = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            drawButton(b, painter);
        } else {
            QStyleOptionButton copy;
            copy.QStyleOption::operator=(*option);
            drawButton(&copy, painter);
        }
        return;

    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight: {
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const Qt::ArrowType dir = element == PE_IndicatorArrowUp     ? Qt::UpArrow
                                  : element == PE_IndicatorArrowLeft  ? Qt::LeftArrow
                                  : element == PE_IndicatorArrowRight ? Qt::RightArrow
                                                                      : Qt::DownArrow;
        const int side = std::clamp(std::min(option->rect.width(), option->rect.height()) * 2 / 3, 5, 13);
        const bool vertical = dir == Qt::UpArrow || dir == Qt::DownArrow;
        QRectF tri = vertical ? QRectF(0, 0, side, std::ceil(side * 0.6))
                              : QRectF(0, 0, std::ceil(side * 0.6), side);
        tri.moveCenter(QRectF(option->rect).center());
        triangle(painter, tri, dir, enabled ? c.ink : c.softShadow,
                 option->palette.color(QPalette::Button));
        return;
    }

    case PE_PanelLineEdit: {
        // OPEN LOOK text field: no box, just the text on an underline.
        // Inner fields of spin boxes and combo boxes (frameless) stay with Fusion.
        const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(option);
        const QWidget *parent = widget ? widget->parentWidget() : nullptr;
        const bool inner = parent && (qobject_cast<const QAbstractSpinBox *>(parent)
                                      || qobject_cast<const QComboBox *>(parent));
        if (!f || f->lineWidth <= 0 || inner) {
            break;
        }
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect;
        const int width = (option->state & State_HasFocus) ? 2 : 1;
        painter->fillRect(QRect(r.left(), r.bottom() - width + 1, r.width(), width),
                          enabled ? c.ink : c.softShadow);
        return;
    }

    case PE_IndicatorCheckBox: {
        // Raised little box; the check mark overflows up and to the right.
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect;
        const int side = std::min(kCheckBox, std::min(r.width(), r.height()) - 3);
        const QRect box(r.left(), r.bottom() - side + 1, side, side);
        box3D(painter, box, c, option->state & State_Sunken);
        painter->save();
        painter->setPen(enabled ? c.dark : c.softShadow);
        painter->drawRect(box.adjusted(0, 0, -1, -1));
        painter->restore();
        bevel(painter, box.adjusted(1, 1, -1, -1), c.light, c.softShadow);

        if (option->state & State_On) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            QPen pen(enabled ? c.ink : c.softShadow, 2.2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
            painter->setPen(pen);
            QPainterPath check;
            check.moveTo(box.left() + 2.5, box.top() + side * 0.45);
            check.lineTo(box.left() + side * 0.45, box.bottom() - 1.5);
            check.lineTo(r.right() - 0.5, r.top() + 1.0);   // overflows the box
            painter->drawPath(check);
            painter->restore();
        } else if (option->state & State_NoChange) {
            painter->fillRect(QRect(box.left() + 3, box.center().y() - 1, side - 6, 2),
                              enabled ? c.ink : c.softShadow);
        }
        return;
    }

    case PE_IndicatorRadioButton: {
        // OPEN LOOK exclusive setting: a rectangle; the chosen one is sunken
        // with a black outline, like "Window" in Snapshot.
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect.adjusted(0, 1, 0, -1);
        const bool chosen = option->state & State_On;
        box3D(painter, r, c, chosen || (option->state & State_Sunken));
        painter->save();
        painter->setPen(chosen ? c.ink : c.dark);
        painter->drawRect(r.adjusted(0, 0, -1, -1));
        if (chosen) {
            painter->fillRect(r.adjusted(4, 4, -4, -4), enabled ? c.ink : c.softShadow);
        }
        painter->restore();
        return;
    }

    default:
        break;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void FreeLookStyle::drawControl(ControlElement element, const QStyleOption *option,
                                QPainter *painter, const QWidget *widget) const
{
    if (element == CE_PushButtonBevel) {
        if (const auto *b = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            if (!(b->features & QStyleOptionButton::Flat) || (b->state & (State_Sunken | State_On))) {
                drawButton(b, painter);
            }
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void FreeLookStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                       QPainter *painter, const QWidget *widget) const
{
    if (control == CC_ScrollBar) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            drawScrollBar(s, painter);
            return;
        }
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QRect FreeLookStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                    SubControl subControl, const QWidget *widget) const
{
    if (control == CC_ScrollBar) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            return scrollBarRect(s, subControl);
        }
    }
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QSize FreeLookStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                      const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_PushButton) {
        // Room for the half-circle ends.
        s.setHeight(std::max(s.height(), 24));
        s.setWidth(s.width() + s.height() / 2);
        if (const auto *b = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            if (b->features & QStyleOptionButton::HasMenu) {
                s.setWidth(s.width() + s.height() / 3);
            }
        }
    }
    return s;
}

int FreeLookStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent:
        return kScrollBarExtent;
    case PM_ScrollBarSliderMin:
        return 3 * kScrollBarExtent;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return kIndicator;
    case PM_ExclusiveIndicatorWidth:
        return 18;
    case PM_ExclusiveIndicatorHeight:
        return 16;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        return 1;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int FreeLookStyle::styleHint(StyleHint hint, const QStyleOption *option,
                             const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_ScrollBar_Transient:
        return false;
    case SH_ScrollBar_MiddleClickAbsolutePosition:
        return true;
    case SH_ScrollBar_LeftClickAbsolutePosition:
        return false;
    default:
        break;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void FreeLookStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    // The elevator arrows change their look while pressed.
    if (qobject_cast<QScrollBar *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, true);
    }
}
