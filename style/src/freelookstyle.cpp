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
#include <QSlider>
#include <QTabBar>
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
constexpr int kTabGap = 3;             // gap between the tabs and the page below them

bool isVerticalTab(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast
        || shape == QTabBar::TriangularWest || shape == QTabBar::TriangularEast;
}
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
    // Outlines: a mid gray on the light palette; on a dark one (FreeLook Night)
    // nearly black, as in the night Plasma style and GTK theme, or they fade
    // into the background.
    c.dark = c.base.lightness() < 128 ? c.base.darker(250)    // #423b33 -> #1a1714
                                      : c.base.darker(160);   // #cccccc -> #7f7f7f
    // White relief on the light palette; on a dark one (FreeLook Night) a
    // lighter shade of the base, so the relief stays visible without glaring.
    c.light = c.base.lightness() < 128 ? c.base.lighter(150) : QColor(255, 255, 255);
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
    // OPEN LOOK button: a rectangle with half-circle ends. A button about as
    // wide as it is tall (an icon-only button) would turn into a circle, so
    // it becomes a box with slightly rounded corners instead.
    QPainterPath path;
    const qreal radius = r.width() < r.height() * 1.4 ? std::min<qreal>(4.0, r.height() / 4.0)
                                                       : r.height() / 2.0;
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

    // Relief: top half light, bottom half dark (swapped when pressed). On an
    // icon-only box: light on the top and left, dark on the bottom and right.
    const QPainterPath inner = oblong(r.adjusted(1, 1, -1, -1));
    QPainterPath lit, shaded;
    if (r.width() < r.height() * 1.4) {
        const QRectF b = r.adjusted(-1, -1, 1, 1);
        lit.addPolygon(QPolygonF({b.topLeft(), b.topRight(), b.bottomLeft()}));
        shaded.addPolygon(QPolygonF({b.topRight(), b.bottomRight(), b.bottomLeft()}));
    } else {
        lit.addRect(QRectF(r.left(), r.top(), r.width(), r.height() / 2.0));
        shaded.addRect(QRectF(r.left(), r.center().y(), r.width(), r.height() / 2.0 + 1));
    }
    p->setBrush(Qt::NoBrush);
    p->setClipPath(lit);
    p->setPen(QPen(sunken ? c.softShadow : c.light, 1.0));
    p->drawPath(inner);
    p->setClipPath(shaded);
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

// ---------------------------------------------------------------------------- tabs
//
// OPEN LOOK had no tabs: property windows switched pages with a "Category"
// menu button. FreeLook draws tabs as an OPEN LOOK exclusive setting, like
// "Snap Type: [Window][Region][Screen]": raised rectangles side by side,
// sharing a single line, and the current one sunken with a black outline.
// The page sits in a plain frame a few pixels below, not attached to the tab.

void FreeLookStyle::drawTab(const QStyleOptionTab *opt, QPainter *p) const
{
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(opt->palette, enabled);
    const bool selected = opt->state & State_Selected;
    const bool vertical = isVerticalTab(opt->shape);
    QRect r = opt->rect;

    // Every tab but the first steps 1 px over the previous one, so neighbors
    // share a single line. The current tab is painted last, on top.
    const bool first = opt->position == QStyleOptionTab::Beginning
                    || opt->position == QStyleOptionTab::OnlyOneTab;
    if (!first) {
        if (vertical) {
            r.adjust(0, -1, 0, 0);
        } else if (opt->direction == Qt::RightToLeft) {
            r.adjust(0, 0, 1, 0);
        } else {
            r.adjust(-1, 0, 0, 0);
        }
    }

    box3D(p, r.adjusted(1, 1, -1, -1), c, selected);
    p->save();
    p->setPen(selected && enabled ? c.ink : (enabled ? c.dark : c.softShadow));
    p->setBrush(Qt::NoBrush);
    p->drawRect(r.adjusted(0, 0, -1, -1));
    p->restore();
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

// ---------------------------------------------------------------------------- spin box and combo box
//
// Numeric field: the number on an underline, like any text field, and two
// raised boxes side by side on the right, [▲][▼]. An arrow is grayed out at
// its limit, like the elevator arrows.
// Combo box: an oblong menu button with a hollow triangle ("Print ▽").
// Editable combo box: underlined text with an abbreviated menu button (a box
// holding just the triangle), like "Mail File: [▽]" in OPEN LOOK's Mail Tool.

namespace
{
int spinButtonSide(const QRect &r)
{
    return std::max(12, r.height() - 6);
}

// Kirigami (QML) hands spin boxes and editable combo boxes the "View" palette,
// where Button is white. Raised boxes must keep the window gray, so in that
// case take the application palette, as the scroll bar elevator does.
// Plasma hands its own menus (desktop context menu, panel menus) a palette
// whose Window is white. Menus are raised gray panels, so in that case take
// the application palette too.
QPalette menuPalette(const QPalette &palette)
{
    if (palette.color(QPalette::Window).lightness() > 245
        && QApplication::palette().color(QPalette::Window).lightness() <= 245) {
        return QApplication::palette();
    }
    return palette;
}

QPalette controlPalette(const QPalette &palette)
{
    if (palette.color(QPalette::Button).lightness() > 245
        && QApplication::palette().color(QPalette::Button).lightness() <= 245) {
        return QApplication::palette();
    }
    return palette;
}
}

QRect FreeLookStyle::spinBoxRect(const QStyleOptionSpinBox *opt, SubControl sc) const
{
    const QRect r = opt->rect;
    const bool buttons = opt->buttonSymbols != QAbstractSpinBox::NoButtons;
    const int side = buttons ? spinButtonSide(r) : 0;
    const int top = r.top() + (r.height() - side) / 2;
    const int downLeft = r.right() - side + 1;
    const int upLeft = downLeft - side - 1;
    QRect result;
    switch (sc) {
    case SC_SpinBoxFrame:     result = r; break;
    case SC_SpinBoxUp:        result = buttons ? QRect(upLeft, top, side, side) : QRect(); break;
    case SC_SpinBoxDown:      result = buttons ? QRect(downLeft, top, side, side) : QRect(); break;
    case SC_SpinBoxEditField: {
        const int right = buttons ? upLeft - 4 : r.right();
        result = QRect(r.left() + 2, r.top() + 1, right - r.left() - 1, r.height() - 3);
        break;
    }
    default: return QRect();
    }
    return visualRect(opt->direction, r, result);
}

void FreeLookStyle::drawSpinBox(const QStyleOptionSpinBox *opt, QPainter *p) const
{
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(controlPalette(opt->palette), enabled);

    // Underline below the number, thicker while the field has focus.
    if (opt->frame) {
        const QRect field = spinBoxRect(opt, SC_SpinBoxEditField);
        const int width = (opt->state & State_HasFocus) ? 2 : 1;
        p->fillRect(QRect(field.left(), opt->rect.bottom() - width + 1, field.width(), width),
                    enabled ? c.ink : c.softShadow);
    }
    if (opt->buttonSymbols == QAbstractSpinBox::NoButtons) {
        return;
    }

    auto button = [&](SubControl sc, QAbstractSpinBox::StepEnabledFlag flag, bool up) {
        const QRect r = spinBoxRect(opt, sc);
        const bool pressed = (opt->state & State_Sunken) && (opt->activeSubControls & sc);
        const bool active = enabled && (opt->stepEnabled & flag);
        box3D(p, r, c, pressed);
        p->save();
        p->setPen(c.dark);
        p->setBrush(Qt::NoBrush);
        p->drawRect(r.adjusted(0, 0, -1, -1));
        p->restore();
        const QColor ink = active ? c.ink : c.softShadow;
        const QPointF shift = pressed ? QPointF(1, 1) : QPointF(0, 0);
        if (opt->buttonSymbols == QAbstractSpinBox::PlusMinus) {
            const QRectF g = QRectF(r).adjusted(r.width() * 0.28, r.height() * 0.28,
                                                -r.width() * 0.28, -r.height() * 0.28).translated(shift);
            const int cy = int(g.center().y()), cx = int(g.center().x());
            p->fillRect(QRect(int(g.left()), cy, int(g.width()), 1), ink);
            if (up) {
                p->fillRect(QRect(cx, int(g.top()), 1, int(g.height())), ink);
            }
        } else {
            const int side = std::max(5, r.width() / 2 + 1);
            QRectF tri(0, 0, side, std::ceil(side * 0.6));
            tri.moveCenter(QRectF(r).center() + shift);
            triangle(p, tri, up ? Qt::UpArrow : Qt::DownArrow, ink, pressed ? c.pressed : c.base);
        }
    };
    button(SC_SpinBoxUp, QAbstractSpinBox::StepUpEnabled, true);
    button(SC_SpinBoxDown, QAbstractSpinBox::StepDownEnabled, false);
}

QRect FreeLookStyle::comboBoxRect(const QStyleOptionComboBox *opt, SubControl sc) const
{
    const QRect r = opt->rect;
    QRect result;
    if (opt->editable) {
        // Abbreviated menu button on the right, the text field on the left.
        const int side = spinButtonSide(r);
        const QRect arrow(r.right() - side + 1, r.top() + (r.height() - side) / 2, side, side);
        switch (sc) {
        case SC_ComboBoxFrame:     result = r; break;
        case SC_ComboBoxArrow:     result = arrow; break;
        case SC_ComboBoxEditField: result = QRect(r.left() + 2, r.top() + 1, arrow.left() - 4 - r.left() - 1, r.height() - 3); break;
        case SC_ComboBoxListBoxPopup: result = r; break;
        default: return QRect();
        }
    } else {
        // Oblong button: room for the round ends and the triangle.
        const int radius = r.height() / 2;
        const int triangleWidth = std::max(7, r.height() / 3);
        const int arrowLeft = r.right() - radius - triangleWidth + 2;
        switch (sc) {
        case SC_ComboBoxFrame:     result = r; break;
        case SC_ComboBoxArrow:     result = QRect(arrowLeft - 2, r.top(), r.right() - arrowLeft + 3, r.height()); break;
        case SC_ComboBoxEditField: result = QRect(r.left() + radius, r.top() + 2, arrowLeft - 4 - r.left() - radius, r.height() - 4); break;
        case SC_ComboBoxListBoxPopup: result = r; break;
        default: return QRect();
        }
    }
    return visualRect(opt->direction, r, result);
}

void FreeLookStyle::drawComboBox(const QStyleOptionComboBox *opt, QPainter *p) const
{
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(controlPalette(opt->palette), enabled);
    const bool pressed = (opt->state & State_Sunken) || (opt->state & State_On);

    if (!opt->editable) {
        QStyleOptionButton b;
        b.QStyleOption::operator=(*opt);
        b.features = QStyleOptionButton::HasMenu;
        b.state &= ~State_On;
        if (pressed) {
            b.state |= State_Sunken;
        }
        drawButton(&b, p);
        return;
    }

    if (opt->frame) {
        const QRect field = comboBoxRect(opt, SC_ComboBoxEditField);
        const int width = (opt->state & State_HasFocus) ? 2 : 1;
        p->fillRect(QRect(field.left(), opt->rect.bottom() - width + 1, field.width(), width),
                    enabled ? c.ink : c.softShadow);
    }
    const QRect arrow = comboBoxRect(opt, SC_ComboBoxArrow);
    const bool arrowPressed = pressed && (opt->activeSubControls & SC_ComboBoxArrow);
    box3D(p, arrow, c, arrowPressed);
    p->save();
    p->setPen(c.dark);
    p->setBrush(Qt::NoBrush);
    p->drawRect(arrow.adjusted(0, 0, -1, -1));
    p->restore();
    const int side = std::max(5, arrow.width() / 2 + 1);
    QRectF tri(0, 0, side, std::ceil(side * 0.6));
    tri.moveCenter(QRectF(arrow).center() + (arrowPressed ? QPointF(1, 1) : QPointF(0, 0)));
    triangle(p, tri, Qt::DownArrow, enabled ? c.ink : c.softShadow, arrowPressed ? c.pressed : c.base);
}

// ---------------------------------------------------------------------------- menus
//
// OPEN LOOK menus: a raised gray panel; the item under the pointer looks
// pressed in (sunken), submenus carry a hollow ▷, separators are etched lines.
// Menu bar entries are OPEN LOOK menu buttons: "File ▽" as an oblong button,
// sunken while its menu is open.

void FreeLookStyle::drawMenuItem(const QStyleOptionMenuItem *opt, QPainter *p, const QWidget *w) const
{
    const bool enabled = opt->state & State_Enabled;
    const QPalette palette = menuPalette(controlPalette(opt->palette));
    const Colors c = colors(palette, enabled, QPalette::Window);

    if (opt->menuItemType == QStyleOptionMenuItem::Separator && opt->text.isEmpty()) {
        const int y = opt->rect.center().y();
        const int left = opt->rect.left() + 4, width = opt->rect.width() - 8;
        p->fillRect(QRect(left, y, width, 1), c.softShadow);
        p->fillRect(QRect(left, y + 1, width, 1), c.light);
        return;
    }

    const bool selected = (opt->state & State_Selected) && enabled
        && opt->menuItemType != QStyleOptionMenuItem::Separator;
    QStyleOptionMenuItem copy(*opt);
    copy.palette = palette;
    if (selected) {
        box3D(p, opt->rect.adjusted(2, 0, -2, 0), c, true);
        copy.state &= ~State_Selected;
        // Hollow arrows and indicators take their fill from Button.
        copy.palette.setColor(QPalette::Button, c.pressed);
    }
    QProxyStyle::drawControl(CE_MenuItem, &copy, p, w);
}

void FreeLookStyle::drawMenuBarItem(const QStyleOptionMenuItem *opt, QPainter *p, const QWidget *w) const
{
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(controlPalette(opt->palette), enabled, QPalette::Window);
    p->fillRect(opt->rect, opt->palette.color(QPalette::Window));
    if (opt->text.isEmpty() && opt->icon.isNull()) {
        return;
    }
    const bool open = (opt->state & State_Sunken) && (opt->state & State_Selected);

    QStyleOptionButton b;
    b.QStyleOption::operator=(*opt);
    b.palette = controlPalette(opt->palette);
    b.rect = opt->rect.adjusted(0, 2, 0, -2);
    b.features = QStyleOptionButton::HasMenu;
    b.state &= ~(State_Sunken | State_On | State_HasFocus);
    if (open) {
        b.state |= State_Sunken;
    }
    drawButton(&b, p);

    const int radius = b.rect.height() / 2;
    const int side = std::max(7, b.rect.height() / 3);
    QRect text = b.rect.adjusted(radius / 2 + 2, 0, -(radius + side + 1), 0);
    if (open) {
        text.translate(1, 1);
    }
    int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
    if (!proxy()->styleHint(SH_UnderlineShortcut, opt, w)) {
        flags |= Qt::TextHideMnemonic;
    }
    p->save();
    p->setPen(c.ink);
    p->drawText(text, flags, opt->text);
    p->restore();
}

// ---------------------------------------------------------------------------- slider

namespace
{
constexpr int kDragBoxLength = 11;      // along the slider
constexpr int kDragBoxThickness = 17;   // across the slider
constexpr int kChannel = 5;             // the recessed bar
constexpr int kTick = 4;                // tick mark length (plus a 1 px gap)
}

QRect FreeLookStyle::sliderRect(const QStyleOptionSlider *opt, SubControl sc) const
{
    const QRect r = opt->rect;
    const bool horizontal = opt->orientation == Qt::Horizontal;
    const int along = horizontal ? r.width() : r.height();
    const int across = horizontal ? r.height() : r.width();
    const int length = std::min(kDragBoxLength, along);
    const int thickness = std::min(kDragBoxThickness, across);

    // Ticks above (left) push the bar down (right), and the other way round.
    int center = across / 2;
    const bool before = opt->tickPosition & QSlider::TicksAbove;
    const bool after = opt->tickPosition & QSlider::TicksBelow;
    if (before && !after) {
        center += (kTick + 1) / 2 + 1;
    } else if (after && !before) {
        center -= (kTick + 1) / 2 + 1;
    }
    center = std::clamp(center, thickness / 2, across - (thickness + 1) / 2);
    const int crossStart = center - thickness / 2;

    const int pos = sliderPositionFromValue(opt->minimum, opt->maximum, opt->sliderPosition,
                                            along - length, opt->upsideDown);
    auto make = [&](int a, int n, int c, int m) {
        return horizontal ? QRect(r.x() + a, r.y() + c, n, m) : QRect(r.x() + c, r.y() + a, m, n);
    };
    switch (sc) {
    case SC_SliderHandle:    return make(pos, length, crossStart, thickness);
    case SC_SliderGroove:    return make(0, along, crossStart, thickness);
    case SC_SliderTickmarks: return r;
    default:                 return QRect();
    }
}

void FreeLookStyle::drawSlider(const QStyleOptionSlider *opt, QPainter *p) const
{
    const bool horizontal = opt->orientation == Qt::Horizontal;
    const bool enabled = opt->state & State_Enabled;
    const Colors c = colors(controlPalette(opt->palette), enabled);
    const QRect groove = sliderRect(opt, SC_SliderGroove);
    const QRect handle = sliderRect(opt, SC_SliderHandle);

    // Tick marks: thin lines on the chosen sides.
    if ((opt->subControls & SC_SliderTickmarks) && opt->tickPosition != QSlider::NoTicks
        && opt->maximum > opt->minimum) {
        const int along = horizontal ? opt->rect.width() : opt->rect.height();
        const int span = along - handle.width() * horizontal - handle.height() * !horizontal;
        auto position = [&](int v) {
            return sliderPositionFromValue(opt->minimum, opt->maximum, v, span, opt->upsideDown);
        };
        int interval = opt->tickInterval;
        if (interval <= 0) {
            interval = opt->singleStep > 0 ? opt->singleStep : 1;
            if (std::abs(position(opt->minimum + interval) - position(opt->minimum)) < 4) {
                interval = opt->pageStep > 0 ? opt->pageStep : interval;
            }
        }
        const int half = (horizontal ? handle.width() : handle.height()) / 2;
        const QColor ink = enabled ? c.dark : c.softShadow;
        for (qint64 v = opt->minimum; v <= opt->maximum; v += interval) {
            const int a = position(int(v)) + half;
            if (horizontal) {
                const int x = opt->rect.x() + a;
                if (opt->tickPosition & QSlider::TicksAbove) {
                    p->fillRect(QRect(x, groove.top() - kTick - 1, 1, kTick), ink);
                }
                if (opt->tickPosition & QSlider::TicksBelow) {
                    p->fillRect(QRect(x, groove.bottom() + 2, 1, kTick), ink);
                }
            } else {
                const int y = opt->rect.y() + a;
                if (opt->tickPosition & QSlider::TicksLeft) {
                    p->fillRect(QRect(groove.left() - kTick - 1, y, kTick, 1), ink);
                }
                if (opt->tickPosition & QSlider::TicksRight) {
                    p->fillRect(QRect(groove.right() + 2, y, kTick, 1), ink);
                }
            }
        }
    }

    // The bar: a recessed channel, filled from the minimum end to the drag box.
    if (opt->subControls & SC_SliderGroove) {
        const QRect bar = horizontal
            ? QRect(groove.left(), groove.center().y() - kChannel / 2, groove.width(), kChannel)
            : QRect(groove.center().x() - kChannel / 2, groove.top(), kChannel, groove.height());
        p->fillRect(bar, c.base);
        bevel(p, bar, c.dark, c.light);
        const QPoint mid = handle.center();
        QRect filled;
        if (horizontal) {
            filled = opt->upsideDown ? QRect(QPoint(mid.x(), bar.top()), bar.bottomRight())
                                     : QRect(bar.topLeft(), QPoint(mid.x(), bar.bottom()));
        } else {
            filled = opt->upsideDown ? QRect(QPoint(bar.left(), mid.y()), bar.bottomRight())
                                     : QRect(bar.topLeft(), QPoint(bar.right(), mid.y()));
        }
        p->fillRect(filled.adjusted(1, 1, -1, -1), enabled ? c.dark : c.softShadow);
    }

    // The drag box: a raised block that sinks while it is dragged.
    if (opt->subControls & SC_SliderHandle) {
        const bool pressed = (opt->state & State_Sunken) && (opt->activeSubControls & SC_SliderHandle);
        const QRect box = handle.adjusted(1, 1, -1, -1);
        box3D(p, box, c, pressed);
        p->save();
        p->setPen((opt->state & State_HasFocus) && enabled ? c.ink : c.dark);
        p->setBrush(Qt::NoBrush);
        p->drawRect(handle.adjusted(0, 0, -1, -1));
        p->restore();
    }
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

    case PE_PanelMenu:
        painter->fillRect(option->rect, menuPalette(controlPalette(option->palette)).color(QPalette::Window));
        return;

    case PE_FrameMenu: {
        // A raised panel with a dark outline.
        const Colors c = colors(menuPalette(controlPalette(option->palette)), true, QPalette::Window);
        const QRect r = option->rect;
        painter->save();
        painter->setPen(c.dark);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(r.adjusted(0, 0, -1, -1));
        painter->restore();
        bevel(painter, r.adjusted(1, 1, -1, -1), c.light, c.softShadow);
        return;
    }

    case PE_PanelMenuBar:
        painter->fillRect(option->rect, option->palette.color(QPalette::Window));
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
        // Inner fields of spin boxes and combo boxes sit on the window gray.
        const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(option);
        const QWidget *parent = widget ? widget->parentWidget() : nullptr;
        const bool inner = parent && (qobject_cast<const QAbstractSpinBox *>(parent)
                                      || qobject_cast<const QComboBox *>(parent));
        if (inner) {
            return;   // the spin box or combo box draws the underline; no white fill
        }
        if (!f || f->lineWidth <= 0) {
            break;    // frameless editors (e.g. in item views) keep Fusion's fill
        }
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect;
        const int width = (option->state & State_HasFocus) ? 2 : 1;
        painter->fillRect(QRect(r.left(), r.bottom() - width + 1, r.width(), width),
                          enabled ? c.ink : c.softShadow);
        return;
    }

    case PE_FrameTabWidget: {
        // The page under the tabs: a plain raised frame.
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect;
        painter->save();
        painter->setPen(enabled ? c.dark : c.softShadow);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(r.adjusted(0, 0, -1, -1));
        painter->restore();
        bevel(painter, r.adjusted(1, 1, -1, -1), c.light, c.softShadow);
        return;
    }

    case PE_FrameTabBarBase: {
        // Document-mode tab bars (Dolphin, Konsole): an etched line between
        // the tabs and the content.
        const auto *base = qstyleoption_cast<const QStyleOptionTabBarBase *>(option);
        const Colors c = colors(option->palette);
        const QRect r = option->rect;
        if (base && !isVerticalTab(base->shape)) {
            const bool south = base->shape == QTabBar::RoundedSouth || base->shape == QTabBar::TriangularSouth;
            const int y = south ? r.top() : r.bottom() - 1;
            painter->fillRect(QRect(r.left(), y, r.width(), 1), c.dark);
            painter->fillRect(QRect(r.left(), y + 1, r.width(), 1), c.light);
        }
        return;
    }

    case PE_IndicatorTabClose: {
        // A thin black "x", without color. It gets the raised 3D box only
        // under the mouse, and sinks while pressed, like the title bar buttons.
        const bool enabled = option->state & State_Enabled;
        const Colors c = colors(option->palette, enabled);
        const QRect r = option->rect;
        const int side = std::min(r.width(), r.height());
        QRect box(0, 0, side, side);
        box.moveCenter(r.center());
        const bool pressed = option->state & State_Sunken;
        if (enabled && (pressed || (option->state & State_MouseOver))) {
            box3D(painter, box, c, pressed);
        }
        const qreal inset = std::max(3.5, side * 0.3);
        const QRectF x = QRectF(box).adjusted(inset, inset, -inset, -inset)
                             .translated(pressed ? 1 : 0, pressed ? 1 : 0);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(enabled ? c.ink : c.softShadow, 1.4));
        painter->drawLine(x.topLeft(), x.bottomRight());
        painter->drawLine(x.topRight(), x.bottomLeft());
        painter->restore();
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
    if (element == CE_TabBarTabShape) {
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            drawTab(tab, painter);
            return;
        }
    }
    if (element == CE_MenuItem) {
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            drawMenuItem(mi, painter, widget);
            return;
        }
    }
    if (element == CE_MenuBarItem) {
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            drawMenuBarItem(mi, painter, widget);
            return;
        }
    }
    if (element == CE_MenuBarEmptyArea) {
        painter->fillRect(option->rect, option->palette.color(QPalette::Window));
        return;
    }
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
    if (control == CC_Slider) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            drawSlider(s, painter);
            return;
        }
    }
    if (control == CC_SpinBox) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            drawSpinBox(s, painter);
            return;
        }
    }
    if (control == CC_ComboBox) {
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            drawComboBox(cb, painter);
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
    if (control == CC_Slider) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            return sliderRect(s, subControl);
        }
    }
    if (control == CC_SpinBox) {
        if (const auto *s = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            return spinBoxRect(s, subControl);
        }
    }
    if (control == CC_ComboBox) {
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            return comboBoxRect(cb, subControl);
        }
    }
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QRect FreeLookStyle::subElementRect(SubElement element, const QStyleOption *option,
                                    const QWidget *widget) const
{
    QRect r = QProxyStyle::subElementRect(element, option, widget);
    if (element == SE_TabWidgetTabPane) {
        // Leave a small gap between the row of tabs and the page.
        if (const auto *tw = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            switch (tw->shape) {
            case QTabBar::RoundedNorth: case QTabBar::TriangularNorth: r.adjust(0, kTabGap, 0, 0); break;
            case QTabBar::RoundedSouth: case QTabBar::TriangularSouth: r.adjust(0, 0, 0, -kTabGap); break;
            case QTabBar::RoundedWest:  case QTabBar::TriangularWest:  r.adjust(kTabGap, 0, 0, 0); break;
            case QTabBar::RoundedEast:  case QTabBar::TriangularEast:  r.adjust(0, 0, -kTabGap, 0); break;
            }
        }
    }
    return r;
}

QSize FreeLookStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                      const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_TabBarTab) {
        // A little more air around the label, like the exclusive settings.
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (isVerticalTab(tab->shape)) {
                s.setHeight(s.height() + 8);
                s.setWidth(std::max(s.width(), 24));
            } else {
                s.setWidth(s.width() + 8);
                s.setHeight(std::max(s.height(), 24));
            }
        }
    }
    if (type == CT_SpinBox) {
        // Two square buttons instead of Fusion's narrow stacked pair.
        s.setHeight(std::max(s.height(), 26));
        s.setWidth(s.width() + spinButtonSide(QRect(0, 0, s.width(), s.height())) + 4);
    }
    if (type == CT_ComboBox) {
        s.setHeight(std::max(s.height(), 26));
        s.setWidth(s.width() + s.height() / 2);
    }
    if (type == CT_ItemViewItem) {
        // Rows of lists and trees (Dolphin's places, file dialogs, settings
        // lists) get some air: Fusion's rows are tight, with icons touching.
        s.setHeight(s.height() + 6);
    }
    if (type == CT_MenuBarItem) {
        // Each entry is a small menu button: round ends and a triangle.
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            if (!mi->text.isEmpty()) {
                const int h = std::max(s.height() + 4, 28);
                s.setHeight(h);
                s.setWidth(s.width() + (h - 4) / 2 + std::max(7, (h - 4) / 3) + 8);
            }
        }
    }
    if (type == CT_MenuItem) {
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            if (mi->menuItemType != QStyleOptionMenuItem::Separator) {
                s.setHeight(std::max(s.height(), 24));
            }
        }
    }
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
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        return 0;   // all tabs on the same line, like exclusive settings
    case PM_TabBarBaseOverlap:
        return 0;
    case PM_MenuPanelWidth:
        return 2;
    case PM_MenuHMargin:
        return 1;
    case PM_MenuVMargin:
        return 3;
    case PM_MenuBarItemSpacing:
        return 4;
    case PM_MenuBarHMargin:
        return 4;
    case PM_MenuBarVMargin:
        return 1;
    case PM_MenuBarPanelWidth:
        return 0;
    case PM_SliderThickness:
    case PM_SliderControlThickness:
        return kDragBoxThickness;
    case PM_SliderLength:
        return kDragBoxLength;
    case PM_SliderTickmarkOffset:
        return kTick + 1;
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
