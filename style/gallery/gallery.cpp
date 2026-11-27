// FreeLook: widget gallery used to test the style.
//
//   QT_PLUGIN_PATH=<dir containing styles/> ./freelook-gallery output.png
//
// 1. Loads the "FreeLook" style through the plugin system (as applications do).
// 2. Tests the elevator scroll bar with simulated clicks (arrows, anchors, drag).
// 3. Saves a screenshot with the widgets and a few pressed states.
// The exit code is 0 only if every check passes.
//
// SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QImage>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include <cstdio>

// Paints states that cannot be "held" in a screenshot.
class States : public QWidget
{
public:
    States() { setFixedSize(720, 64); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        QStyle *s = style();
        int x = 4;
        auto button = [&](const QString &text, QStyle::State extra, QStyleOptionButton::ButtonFeatures f) {
            QStyleOptionButton o;
            o.initFrom(this);
            o.text = text;
            o.state |= extra | QStyle::State_Raised;
            o.features = f;
            o.rect = QRect(x, 4, 96, 24);
            s->drawControl(QStyle::CE_PushButton, &o, &p, this);
            x += 104;
        };
        button(QStringLiteral("Pressed"), QStyle::State_Sunken, QStyleOptionButton::None);
        button(QStringLiteral("Print"), QStyle::State_None, QStyleOptionButton::HasMenu);

        auto bar = [&](QStyle::SubControl active, int value, bool whitePalette = false) {
            QStyleOptionSlider o;
            o.initFrom(this);
            o.orientation = Qt::Horizontal;
            o.minimum = 0; o.maximum = 100; o.pageStep = 30; o.singleStep = 1;
            o.sliderPosition = o.sliderValue = value;
            o.rect = QRect(x, 6, 100, 16);
            o.subControls = QStyle::SC_All;
            o.activeSubControls = active;
            if (whitePalette) {   // mimics Kirigami QML lists
                o.palette.setColor(QPalette::Window, Qt::white);
                o.palette.setColor(QPalette::Button, Qt::white);
            }
            if (active != QStyle::SC_None) o.state |= QStyle::State_Sunken;
            s->drawComplexControl(QStyle::CC_ScrollBar, &o, &p, this);
            x += 106;
        };
        auto tabClose = [&](QStyle::State extra) {
            QStyleOption o;
            o.initFrom(this);
            o.state |= extra;
            o.rect = QRect(x, 6, 16, 16);
            s->drawPrimitive(QStyle::PE_IndicatorTabClose, &o, &p, this);
            x += 22;
        };
        {
            QStyleOptionSpinBox o;
            o.initFrom(this);
            o.rect = QRect(x, 2, 70, 26);
            o.frame = true;
            o.subControls = QStyle::SC_All;
            o.activeSubControls = QStyle::SC_SpinBoxUp;
            o.state |= QStyle::State_Sunken;
            o.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
            s->drawComplexControl(QStyle::CC_SpinBox, &o, &p, this);
            x += 76;
        }
        tabClose(QStyle::State_None);
        tabClose(QStyle::State_MouseOver);
        tabClose(QStyle::State_Sunken);
        bar(QStyle::SC_ScrollBarSubLine, 40);
        bar(QStyle::SC_ScrollBarSlider, 0);
        bar(QStyle::SC_None, 60, true);
        p.drawText(QRect(4, 36, 712, 24), Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("states: pressed · menu · spin ▲ pressed · tab close (normal, hover, pressed) · ◀ pressed · elevator at start · white palette"));
    }
};

static void click(QWidget *w, const QPoint &pos)
{
    const QPointF g = w->mapToGlobal(QPointF(pos));
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(pos), g, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(w, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(pos), g, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &release);
}

static int testElevator()
{
    int failures = 0;
    auto check = [&](const char *name, bool ok) {
        std::printf("  %-46s %s\n", name, ok ? "ok" : "FAILED");
        if (!ok) ++failures;
    };
    QScrollBar sb(Qt::Vertical);
    sb.setRange(0, 1000);
    sb.setPageStep(100);
    sb.setSingleStep(10);
    sb.resize(16, 300);
    sb.show();
    sb.setValue(500);

    QStyleOptionSlider opt;
    opt.initFrom(&sb);
    opt.orientation = sb.orientation();
    opt.minimum = sb.minimum(); opt.maximum = sb.maximum();
    opt.sliderPosition = opt.sliderValue = sb.value();
    opt.pageStep = sb.pageStep(); opt.singleStep = sb.singleStep();
    opt.upsideDown = false;
    QStyle *s = sb.style();
    auto rect = [&](QStyle::SubControl sc) { return s->subControlRect(QStyle::CC_ScrollBar, &opt, sc, &sb); };

    const QRect sub = rect(QStyle::SC_ScrollBarSubLine), add = rect(QStyle::SC_ScrollBarAddLine);
    const QRect elev = rect(QStyle::SC_ScrollBarSlider);
    check("arrows sit inside the elevator", elev.contains(sub) && elev.contains(add));
    check("hit test: up arrow", s->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, sub.center(), &sb) == QStyle::SC_ScrollBarSubLine);
    check("hit test: middle of the elevator", s->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, elev.center(), &sb) == QStyle::SC_ScrollBarSlider);

    click(&sb, sub.center());
    check("up arrow scrolls one line up", sb.value() == 490);
    click(&sb, add.center());
    check("down arrow scrolls one line down", sb.value() == 500);

    click(&sb, rect(QStyle::SC_ScrollBarAddPage).center());
    check("cable below scrolls one page down", sb.value() == 600);

    click(&sb, rect(QStyle::SC_ScrollBarLast).center());
    check("bottom anchor jumps to the end", sb.value() == sb.maximum());
    click(&sb, rect(QStyle::SC_ScrollBarFirst).center());
    check("top anchor jumps to the start", sb.value() == sb.minimum());

    // Drag the elevator by its middle down to the end of the cable.
    opt.sliderPosition = opt.sliderValue = sb.value();
    const QPoint from = rect(QStyle::SC_ScrollBarSlider).center();
    const QPoint to(from.x(), rect(QStyle::SC_ScrollBarGroove).bottom());
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(from), sb.mapToGlobal(QPointF(from)), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&sb, &press);
    QMouseEvent move(QEvent::MouseMove, QPointF(to), sb.mapToGlobal(QPointF(to)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&sb, &move);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(to), sb.mapToGlobal(QPointF(to)), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&sb, &release);
    check("dragging the elevator reaches the end", sb.value() == sb.maximum());
    // Spin box buttons with Kirigami's all-white palette must stay gray.
    {
        QImage img(80, 30, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        QStyleOptionSpinBox o;
        o.palette = QApplication::palette();
        o.palette.setColor(QPalette::Button, Qt::white);
        o.palette.setColor(QPalette::Base, Qt::white);
        o.state = QStyle::State_Enabled;
        o.rect = QRect(0, 2, 80, 26);
        o.frame = true;
        o.subControls = QStyle::SC_All;
        o.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
        QApplication::style()->drawComplexControl(QStyle::CC_SpinBox, &o, &p, nullptr);
        p.end();
        const QRect up = QApplication::style()->subControlRect(QStyle::CC_SpinBox, &o, QStyle::SC_SpinBoxUp, nullptr);
        const QColor fill = img.pixelColor(up.left() + 3, up.bottom() - 3);
        check("spin buttons stay gray with a white palette", fill.lightness() < 245);
    }
    // An icon-only (square) button is a box, not a circle.
    {
        QImage img(32, 32, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        QStyleOptionButton o;
        o.palette = QApplication::palette();
        o.state = QStyle::State_Enabled | QStyle::State_Raised;
        o.rect = img.rect();
        QApplication::style()->drawControl(QStyle::CE_PushButtonBevel, &o, &p, nullptr);
        p.end();
        check("square buttons are boxes, not circles", img.pixelColor(4, 4).alpha() == 255);
    }
    // List rows with a small icon have some air (Dolphin's places panel).
    {
        QStyleOptionViewItem o;
        o.features = QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration;
        o.text = QStringLiteral("Downloads");
        o.decorationSize = QSize(16, 16);
        o.fontMetrics = QApplication::fontMetrics();
        o.font = QApplication::font();
        const int h = QApplication::style()->sizeFromContents(QStyle::CT_ItemViewItem, &o, QSize(), nullptr).height();
        check("list rows have room around a small icon", h >= 24);
    }
    // Menus handed Plasma's white palette stay gray.
    {
        QImage img(40, 20, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        QStyleOption o;
        o.palette = QApplication::palette();
        o.palette.setColor(QPalette::Window, Qt::white);
        o.rect = img.rect();
        QApplication::style()->drawPrimitive(QStyle::PE_PanelMenu, &o, &p, nullptr);
        p.end();
        check("menus stay gray with a white palette", img.pixelColor(20, 10).lightness() < 245);
    }
    // Slider: grab the drag box and pull it to the far end.
    {
        QSlider sl(Qt::Horizontal);
        sl.setRange(0, 100);
        sl.setTickPosition(QSlider::TicksBelow);
        sl.resize(200, sl.sizeHint().height());
        sl.show();
        sl.setValue(50);
        QStyleOptionSlider o;
        o.initFrom(&sl);
        o.orientation = Qt::Horizontal;
        o.minimum = 0; o.maximum = 100;
        o.sliderPosition = o.sliderValue = 50;
        o.tickPosition = QSlider::TicksBelow;
        o.upsideDown = false;
        o.subControls = QStyle::SC_All;
        QStyle *st = sl.style();
        const QRect handle = st->subControlRect(QStyle::CC_Slider, &o, QStyle::SC_SliderHandle, &sl);
        check("slider: drag box sits in the middle", std::abs(handle.center().x() - 100) <= 2);
        check("slider: hit test on the drag box",
              st->hitTestComplexControl(QStyle::CC_Slider, &o, handle.center(), &sl) == QStyle::SC_SliderHandle);
        const QPoint from = handle.center(), to(sl.width() - 1, from.y());
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(from), sl.mapToGlobal(QPointF(from)), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&sl, &press);
        QMouseEvent move(QEvent::MouseMove, QPointF(to), sl.mapToGlobal(QPointF(to)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&sl, &move);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(to), sl.mapToGlobal(QPointF(to)), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&sl, &release);
        check("slider: dragging reaches the maximum", sl.value() == 100);
    }
    return failures;
}

// The FreeLook Night palette, from color-schemes/FreeLookNight.colors.
static QPalette nightPalette()
{
    const QColor base(66, 59, 51), text(238, 224, 200), view(46, 41, 36);
    QPalette p(base);
    p.setColor(QPalette::Window, base);
    p.setColor(QPalette::Button, base);
    p.setColor(QPalette::Base, view);
    p.setColor(QPalette::AlternateBase, QColor(52, 47, 41));
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::Highlight, QColor(107, 93, 76));
    p.setColor(QPalette::HighlightedText, QColor(255, 240, 214));
    p.setColor(QPalette::ToolTipBase, QColor(87, 77, 66));
    p.setColor(QPalette::ToolTipText, text);
    const QColor grayedOut(138, 126, 108);
    p.setColor(QPalette::Disabled, QPalette::WindowText, grayedOut);
    p.setColor(QPalette::Disabled, QPalette::Text, grayedOut);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, grayedOut);
    return p;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // gallery [output.png] [--night]: --night uses the FreeLook Night palette.
    QString output = QStringLiteral("gallery.png");
    bool night = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("--night")) {
            night = true;
        } else {
            output = arg;
        }
    }

    QStyle *style = QStyleFactory::create(QStringLiteral("FreeLook"));
    if (!style) {
        std::printf("ERROR: the FreeLook style was not found. Available styles: %s\n",
                    qPrintable(QStyleFactory::keys().join(QStringLiteral(", "))));
        return 1;
    }
    QApplication::setStyle(style);
    QApplication::setPalette(night ? nightPalette() : style->standardPalette());
    std::printf("style loaded: %s\n", qPrintable(QApplication::style()->name()));

    std::printf("elevator checks:\n");
    const int failures = testElevator();

    QWidget window;
    window.setWindowTitle(QStringLiteral("FreeLook"));
    auto *root = new QVBoxLayout(&window);
    auto *tabs = new QTabWidget;
    root->addWidget(tabs);
    auto *page = new QWidget;
    tabs->addTab(page, QStringLiteral("Controls"));
    tabs->addTab(new QWidget, QStringLiteral("Another tab"));
    tabs->addTab(new QWidget, QStringLiteral("Third"));
    auto *grid = new QGridLayout(page);

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(new QPushButton(QStringLiteral("Load...")));
    auto *defaultButton = new QPushButton(QStringLiteral("Save..."));
    defaultButton->setDefault(true);
    buttons->addWidget(defaultButton);
    auto *menuButton = new QPushButton(QStringLiteral("Print"));
    menuButton->setMenu(new QMenu(menuButton));
    buttons->addWidget(menuButton);
    auto *disabled = new QPushButton(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    buttons->addWidget(disabled);
    grid->addLayout(buttons, 0, 0, 1, 2);

    auto *options = new QGroupBox(QStringLiteral("Options"));
    auto *vo = new QVBoxLayout(options);
    auto *c1 = new QCheckBox(QStringLiteral("Beep during countdown"));
    c1->setChecked(true);
    vo->addWidget(c1);
    vo->addWidget(new QCheckBox(QStringLiteral("Hide window")));
    auto *c3 = new QCheckBox(QStringLiteral("Partial"));
    c3->setTristate(true);
    c3->setCheckState(Qt::PartiallyChecked);
    vo->addWidget(c3);
    auto *r1 = new QRadioButton(QStringLiteral("Window"));
    r1->setChecked(true);
    vo->addWidget(r1);
    vo->addWidget(new QRadioButton(QStringLiteral("Region")));
    grid->addWidget(options, 1, 0);

    auto *fields = new QVBoxLayout;
    auto *fileRow = new QHBoxLayout;
    fileRow->addWidget(new QLabel(QStringLiteral("Mail file:")));
    fileRow->addWidget(new QLineEdit(QStringLiteral("~/mail/inbox")));
    fields->addLayout(fileRow);
    auto *focused = new QLineEdit(QStringLiteral("has focus"));
    fields->addWidget(focused);
    auto *editable = new QComboBox;
    editable->setEditable(true);
    editable->addItem(QStringLiteral("editable combo"));
    fields->addWidget(editable);
    auto *combo = new QComboBox;
    combo->addItems({QStringLiteral("Combo"), QStringLiteral("Two")});
    fields->addWidget(combo);
    auto *spins = new QHBoxLayout;
    auto *spin = new QSpinBox;
    spin->setValue(42);
    spins->addWidget(spin);
    auto *atMax = new QSpinBox;
    atMax->setRange(0, 10);
    atMax->setValue(10);
    spins->addWidget(atMax);
    auto *plusMinus = new QDoubleSpinBox;
    plusMinus->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    plusMinus->setValue(1.5);
    spins->addWidget(plusMinus);
    fields->addLayout(spins);
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setValue(40);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);
    fields->addWidget(slider);
    auto *plainSliders = new QHBoxLayout;
    auto *plain = new QSlider(Qt::Horizontal);
    plain->setValue(70);
    plainSliders->addWidget(plain);
    auto *disabledSlider = new QSlider(Qt::Horizontal);
    disabledSlider->setValue(30);
    disabledSlider->setEnabled(false);
    plainSliders->addWidget(disabledSlider);
    fields->addLayout(plainSliders);
    auto *progress = new QProgressBar;
    progress->setValue(65);
    fields->addWidget(progress);
    grid->addLayout(fields, 1, 1);

    auto *list = new QListWidget;
    for (int i = 1; i <= 40; ++i) list->addItem(QStringLiteral("List item %1").arg(i));
    list->setCurrentRow(1);
    grid->addWidget(list, 2, 0);
    auto *text = new QTextEdit;
    text->setLineWrapMode(QTextEdit::NoWrap);
    QString lines;
    for (int i = 0; i < 60; ++i) lines += QStringLiteral("A line of text long enough to scroll sideways %1\n").arg(i);
    text->setPlainText(lines);
    grid->addWidget(text, 2, 1);

    // Document-mode tab bar with close buttons, as in Dolphin and Konsole.
    auto *documentTabs = new QTabBar;
    documentTabs->setDocumentMode(true);
    documentTabs->setTabsClosable(true);
    documentTabs->setExpanding(false);
    for (const char *name : {"~/Projects", "~/Documents", "~/mail"}) {
        documentTabs->addTab(QString::fromLatin1(name));
    }
    documentTabs->setCurrentIndex(1);
    root->insertWidget(0, documentTabs);

    // Menu bar with OPEN LOOK menu buttons, and a menu to show next to it.
    auto *menuBar = new QMenuBar;
    auto *fileMenu = menuBar->addMenu(QStringLiteral("&File"));
    menuBar->addMenu(QStringLiteral("&Edit"));
    menuBar->addMenu(QStringLiteral("&View"));
    root->setMenuBar(menuBar);
    fileMenu->addAction(QStringLiteral("Open..."));
    auto *saveAction = fileMenu->addAction(QStringLiteral("Save"));
    saveAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));
    fileMenu->addAction(QStringLiteral("Save as..."))->setEnabled(false);
    fileMenu->addSeparator();
    auto *wrap = fileMenu->addAction(QStringLiteral("Wrap lines"));
    wrap->setCheckable(true);
    wrap->setChecked(true);
    auto *recent = fileMenu->addMenu(QStringLiteral("Recent files"));
    recent->addAction(QStringLiteral("notes.txt"));
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quit"));

    root->addWidget(new States);
    window.resize(720, 640);
    window.show();
    focused->setFocus();

    QTimer::singleShot(300, [&]() {
        window.grab().save(output);
        std::printf("screenshot saved: %s\n", qPrintable(output));
        // The menu, with "Save" under the pointer, saved next to the screenshot.
        fileMenu->setActiveAction(saveAction);
        fileMenu->adjustSize();
        QString menuOutput = output;
        menuOutput.replace(QStringLiteral(".png"), QStringLiteral("-menu.png"));
        fileMenu->grab().save(menuOutput);
        std::printf("menu saved: %s\n", qPrintable(menuOutput));
        app.exit(failures == 0 ? 0 : 2);
    });
    return app.exec();
}
