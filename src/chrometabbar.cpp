#include "chrometabbar.h"

#include <QEasingCurve>
#include <QEvent>
#include <QFontMetrics>
#include <QIcon>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOptionTab>
#include <QTimer>
#include <QtMath>

namespace {

// HTML prototype palette ---------------------------------------------------
constexpr const char *kStripTopColor      = "#fbfcf4";
constexpr const char *kStripBottomColor   = "#f6f8ef";
constexpr const char *kStripBorderColor   = "#dfe4d6";
constexpr const char *kTabBackground      = "#fafbf4"; // rgba(250,251,244,.92)
constexpr const char *kTabHoverBackground = "#edf1e2"; // rgba(237,241,226,.86)
constexpr const char *kTabBorder          = "#e1e5d8";
constexpr const char *kTabHoverBorder     = "#d9dece";
constexpr const char *kTabText            = "#555b4f";
constexpr const char *kTabHoverText       = "#262b21";
constexpr const char *kPillBase           = "#d9e5c9";
constexpr const char *kPillBorder         = "#627748";
constexpr const char *kActiveText         = "#1e211b";
constexpr const char *kCloseIconColor     = "#1f241d";

// Tab sizing -----------------------------------------------------------------
constexpr int kTabHeight    = 44;
constexpr int kTabPillRadius = 15;
constexpr int kBarHeight    = 58;
constexpr int kPreferredTabWidth = 310;
constexpr int kMinTabWidth  = 130;
constexpr int kTabSpacing   = 14;
constexpr int kPillTopMargin = (kBarHeight - kTabHeight) / 2; // 7px
constexpr int kFaviconSize  = 22;
constexpr int kCloseSize    = 24;
constexpr int kPaddingLeft  = 18;
constexpr int kPaddingRight = 12;

QPainterPath roundedPath(const QRectF &rect, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

} // namespace

ChromeTabBar::ChromeTabBar(QWidget *parent)
    : QTabBar(parent)
{
    setDrawBase(false);
    setExpanding(false);
    setMovable(true);                   // built-in drag & drop reorder
    setTabsClosable(false);              // we paint and dispatch close ourselves
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setIconSize(QSize(kFaviconSize, kFaviconSize));
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(kBarHeight);

    pillAnim_ = new QPropertyAnimation(this, "pillGeometry", this);
    pillAnim_->setDuration(260);
    pillAnim_->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QTabBar::currentChanged, this, &ChromeTabBar::onCurrentChanged);
    connect(this, &QTabBar::tabMoved, this, [this](int, int) {
        snapPillToCurrent();
        update();
    });
}

void ChromeTabBar::setPillGeometry(const QRectF &geometry)
{
    if (pillGeometry_ == geometry) {
        return;
    }
    pillGeometry_ = geometry;
    update();
}

void ChromeTabBar::snapPillToCurrent()
{
    if (pillAnim_ && pillAnim_->state() == QAbstractAnimation::Running) {
        pillAnim_->stop();
    }
    if (currentIndex() < 0) {
        pillGeometry_ = QRectF();
    } else {
        pillGeometry_ = pillRectForIndex(currentIndex());
    }
    update();
}

QSize ChromeTabBar::tabSizeHint(int /*index*/) const
{
    const int barWidth = std::max(width(), 1);
    const int n = std::max(count(), 1);
    int candidate = (barWidth - kTabSpacing * (n - 1)) / n;
    candidate = std::clamp(candidate, kMinTabWidth, kPreferredTabWidth);
    return QSize(candidate, kTabHeight);
}

QSize ChromeTabBar::minimumTabSizeHint(int /*index*/) const
{
    return QSize(kMinTabWidth, kTabHeight);
}

void ChromeTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    if (firstLayout_) {
        snapPillToCurrent();
    } else {
        // Defer: tab geometry not finalised yet inside this callback.
        QTimer::singleShot(0, this, [this] {
            if (currentIndex() >= 0) {
                animatePillTo(pillRectForIndex(currentIndex()));
            }
        });
    }
}

void ChromeTabBar::tabRemoved(int index)
{
    QTabBar::tabRemoved(index);
    if (hoverTabIndex_ == index) {
        hoverTabIndex_ = -1;
    } else if (hoverTabIndex_ > index) {
        --hoverTabIndex_;
    }
    if (hoverCloseIndex_ == index) {
        hoverCloseIndex_ = -1;
    } else if (hoverCloseIndex_ > index) {
        --hoverCloseIndex_;
    }
    QTimer::singleShot(0, this, [this] {
        if (currentIndex() >= 0) {
            animatePillTo(pillRectForIndex(currentIndex()));
        } else {
            pillGeometry_ = QRectF();
            update();
        }
    });
}

void ChromeTabBar::tabLayoutChange()
{
    QTabBar::tabLayoutChange();
    if (firstLayout_) {
        firstLayout_ = false;
        snapPillToCurrent();
    } else if (pillAnim_->state() != QAbstractAnimation::Running && currentIndex() >= 0) {
        // Layout (resize / scroll) moved tabs; keep pill aligned without animation.
        pillGeometry_ = pillRectForIndex(currentIndex());
        update();
    }
}

void ChromeTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    if (currentIndex() >= 0) {
        pillGeometry_ = pillRectForIndex(currentIndex());
    }
}

void ChromeTabBar::onCurrentChanged(int index)
{
    if (index < 0) {
        return;
    }
    animatePillTo(pillRectForIndex(index));
}

void ChromeTabBar::animatePillTo(const QRectF &target)
{
    if (!pillAnim_) {
        pillGeometry_ = target;
        update();
        return;
    }
    if (pillGeometry_.isNull() || firstLayout_) {
        pillGeometry_ = target;
        update();
        return;
    }
    pillAnim_->stop();
    pillAnim_->setStartValue(pillGeometry_);
    pillAnim_->setEndValue(target);
    pillAnim_->start();
}

QRectF ChromeTabBar::pillRectForIndex(int index) const
{
    if (index < 0 || index >= count()) {
        return QRectF();
    }
    const QRect r = tabRect(index);
    if (r.isEmpty()) {
        return QRectF();
    }
    // The pill sits inside the tab rect with a small vertical inset to leave
    // room for the strip top/bottom gradient (matches the prototype's 8px top
    // offset on a 60px container).
    QRectF f(r);
    f.setHeight(kTabHeight);
    f.moveTop(r.top() + std::max(0, (r.height() - kTabHeight) / 2));
    return f;
}

QRect ChromeTabBar::faviconRect(int tabIndex) const
{
    const QRect r = tabRect(tabIndex);
    if (r.isEmpty()) {
        return {};
    }
    const QRectF pill = pillRectForIndex(tabIndex);
    const int top = static_cast<int>(pill.top() + (pill.height() - kFaviconSize) / 2);
    return QRect(r.left() + kPaddingLeft, top, kFaviconSize, kFaviconSize);
}

QRect ChromeTabBar::closeIconRect(int tabIndex) const
{
    const QRect r = tabRect(tabIndex);
    if (r.isEmpty()) {
        return {};
    }
    const QRectF pill = pillRectForIndex(tabIndex);
    const int top = static_cast<int>(pill.top() + (pill.height() - kCloseSize) / 2);
    return QRect(r.right() - kPaddingRight - kCloseSize, top, kCloseSize, kCloseSize);
}

int ChromeTabBar::closeButtonAt(const QPoint &pos) const
{
    const int idx = tabAt(pos);
    if (idx < 0) {
        return -1;
    }
    if (closeIconRect(idx).contains(pos)) {
        return idx;
    }
    return -1;
}

void ChromeTabBar::updateHover(const QPoint &pos)
{
    const int newTab = tabAt(pos);
    const int newClose = closeButtonAt(pos);
    if (newTab != hoverTabIndex_ || newClose != hoverCloseIndex_) {
        hoverTabIndex_ = newTab;
        hoverCloseIndex_ = newClose;
        update();
    }
}

void ChromeTabBar::leaveEvent(QEvent *event)
{
    QTabBar::leaveEvent(event);
    hoverTabIndex_ = -1;
    hoverCloseIndex_ = -1;
    update();
}

void ChromeTabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const int closeIdx = closeButtonAt(event->pos());
        if (closeIdx >= 0) {
            pressedCloseIndex_ = closeIdx;
            event->accept();
            update();
            return;
        }
        // Middle button or close-tab dispatch: middle-click closes.
    }
    if (event->button() == Qt::MiddleButton) {
        const int idx = tabAt(event->pos());
        if (idx >= 0) {
            emit tabCloseClicked(idx);
            event->accept();
            return;
        }
    }
    QTabBar::mousePressEvent(event);
}

void ChromeTabBar::mouseMoveEvent(QMouseEvent *event)
{
    updateHover(event->pos());
    QTabBar::mouseMoveEvent(event);
}

void ChromeTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && pressedCloseIndex_ >= 0) {
        const int releasedOver = closeButtonAt(event->pos());
        const int idx = pressedCloseIndex_;
        pressedCloseIndex_ = -1;
        if (releasedOver == idx && idx >= 0 && idx < count()) {
            emit tabCloseClicked(idx);
            event->accept();
            return;
        }
        update();
    }
    QTabBar::mouseReleaseEvent(event);
}

void ChromeTabBar::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. Strip background gradient.
    QLinearGradient stripGradient(0, 0, 0, height());
    stripGradient.setColorAt(0, QColor(kStripTopColor));
    stripGradient.setColorAt(1, QColor(kStripBottomColor));
    p.fillRect(rect(), stripGradient);

    // 2. Bottom border.
    p.setPen(QColor(kStripBorderColor));
    p.drawLine(0, height() - 1, width(), height() - 1);

    const int current = currentIndex();
    const int total = count();

    // 3. Inactive tabs (background + border + hover).
    for (int i = 0; i < total; ++i) {
        if (i == current) {
            continue;
        }
        const QRectF pill = pillRectForIndex(i);
        if (pill.isEmpty()) {
            continue;
        }
        const QPainterPath path = roundedPath(pill, kTabPillRadius);

        QColor bg(kTabBackground);
        bg.setAlphaF(0.92);
        p.fillPath(path, bg);

        if (i == hoverTabIndex_) {
            QColor hover(kTabHoverBackground);
            hover.setAlphaF(0.86);
            p.fillPath(path, hover);
            p.setPen(QPen(QColor(kTabHoverBorder), 1));
        } else {
            p.setPen(QPen(QColor(kTabBorder), 1));
        }
        p.drawPath(path);

        // Soft inner highlight (1px white inset).
        QPainterPath inner = roundedPath(pill.adjusted(0.5, 0.5, -0.5, -0.5), kTabPillRadius - 0.5);
        QPen innerPen{QColor(255, 255, 255, 209)}; // rgba(255,255,255,.82)
        innerPen.setWidthF(0.8);
        p.strokePath(inner, innerPen);
    }

    // 4. Active pill (gradient + border).
    if (current >= 0 && !pillGeometry_.isNull()) {
        const QPainterPath path = roundedPath(pillGeometry_, kTabPillRadius);
        QLinearGradient pillGradient(pillGeometry_.topLeft(), pillGeometry_.bottomLeft());
        pillGradient.setColorAt(0, QColor(255, 255, 255, 128)); // rgba(255,255,255,.50)
        pillGradient.setColorAt(1, QColor(255, 255, 255, 10));  // rgba(255,255,255,.04)
        p.fillPath(path, QColor(kPillBase));
        p.fillPath(path, pillGradient);
        QPen pen{QColor(kPillBorder)};
        pen.setWidthF(1.5);
        p.setPen(pen);
        p.drawPath(path);

        QPainterPath inner = roundedPath(pillGeometry_.adjusted(0.6, 0.6, -0.6, -0.6),
                                         kTabPillRadius - 0.6);
        QPen innerPen{QColor(255, 255, 255, 188)}; // rgba(255,255,255,.74)
        innerPen.setWidthF(0.9);
        p.strokePath(inner, innerPen);
    }

    // 5. Tab content (favicon + title + close button) for every tab.
    for (int i = 0; i < total; ++i) {
        const QRect r = tabRect(i);
        if (r.isEmpty()) {
            continue;
        }

        const bool isActive = (i == current);
        const QColor textColor(isActive ? kActiveText
                                        : (i == hoverTabIndex_ ? kTabHoverText : kTabText));

        // Favicon
        const QIcon icon = tabIcon(i);
        const QRect favRect = faviconRect(i);
        if (!icon.isNull()) {
            icon.paint(&p, favRect, Qt::AlignCenter,
                       isActive ? QIcon::Normal : QIcon::Normal,
                       QIcon::On);
        } else {
            // Placeholder dot so the layout looks right when no favicon yet.
            p.setPen(Qt::NoPen);
            QColor dot = textColor;
            dot.setAlphaF(0.18);
            p.setBrush(dot);
            const int d = 10;
            p.drawEllipse(favRect.center(), d / 2, d / 2);
        }

        // Title
        const QRect closeR = closeIconRect(i);
        QRect titleRect(favRect.right() + 11,
                        r.top(),
                        closeR.left() - (favRect.right() + 11) - 8,
                        r.height());
        if (titleRect.width() > 0) {
            QFont f = font();
            f.setPixelSize(15);
            f.setWeight(QFont::Medium);
            p.setFont(f);
            p.setPen(textColor);
            const QFontMetrics fm(f);
            const QString text = fm.elidedText(tabText(i), Qt::ElideRight, titleRect.width());
            p.drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft, text);
        }

        // Close button
        const bool isHoverClose = (i == hoverCloseIndex_);
        const bool isPressed = (i == pressedCloseIndex_);
        if (isHoverClose || isPressed) {
            QColor bg(0, 0, 0, isPressed ? 36 : 26);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawEllipse(closeR);
        }
        // Draw the X glyph
        QPen xPen{QColor(kCloseIconColor)};
        xPen.setWidthF(1.6);
        xPen.setCapStyle(Qt::RoundCap);
        p.setPen(xPen);
        const qreal pad = 7.5;
        const QPointF c = QRectF(closeR).center();
        const qreal half = (kCloseSize / 2.0) - pad;
        p.save();
        p.translate(c);
        if (isHoverClose) {
            // Spin slightly on hover (mirrors the HTML's rotate(90deg) hover).
            p.rotate(90);
        }
        p.drawLine(QPointF(-half, -half), QPointF(half, half));
        p.drawLine(QPointF(-half, half), QPointF(half, -half));
        p.restore();
    }
}

// ---------------------------------------------------------------------------
// ChromeTabWidget
// ---------------------------------------------------------------------------

ChromeTabWidget::ChromeTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    auto *bar = new ChromeTabBar(this);
    setTabBar(bar);
    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(false);
}

ChromeTabBar *ChromeTabWidget::chromeBar() const
{
    return qobject_cast<ChromeTabBar *>(tabBar());
}

