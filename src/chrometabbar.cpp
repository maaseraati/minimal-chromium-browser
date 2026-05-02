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
#include <QtMath>

namespace {

// Material 3 light palette mirrored from morphine://home (the in-app start
// page) so the chrome reads as part of the same surface as the home page.
//   --primary:               #4a76b3
//   --on-primary:            #ffffff
//   --primary-container:     #d8e2ff   (active-pill fill)
//   --on-primary-container:  #001a41
//   --surface:               #fafbff
//   --surface-container:     #eef0f7
//   --surface-container-high:#e2e6ee
//   --on-surface:            #1a1c1f
//   --on-surface-variant:    #44474e
//   --outline-variant:       #c4c6cf
constexpr const char *kStripBorderColor   = "#e2e6ee";
constexpr const char *kTabHoverBackground = "#eef0f7";
constexpr const char *kTabBorder          = "#dfe2ea";
constexpr const char *kTabHoverBorder     = "#c4c6cf";
constexpr const char *kTabText            = "#44474e";
constexpr const char *kTabHoverText       = "#1a1c1f";
constexpr const char *kPillBase           = "#d8e2ff";
constexpr const char *kPillBorder         = "#4a76b3";
constexpr const char *kActiveText         = "#001a41";
constexpr const char *kCloseIconColor     = "#1a1c1f";

// Tab sizing -----------------------------------------------------------------
constexpr int kTabHeight    = 44;
constexpr int kTabPillRadius = 15;
constexpr int kBarHeight    = 58;
constexpr int kPreferredTabWidth = 310;
constexpr int kMinTabWidth  = 130;
constexpr int kFaviconSize  = 22;
constexpr int kCloseSize    = 24;
constexpr int kPaddingLeft  = 18;
constexpr int kPaddingRight = 12;

// Animation durations (matches HTML prototype's transition timings).
constexpr int kPillAnimDuration   = 240;
constexpr int kAppearDuration     = 220;
constexpr int kCloseDuration      = 180;

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
    pillAnim_->setDuration(kPillAnimDuration);
    pillAnim_->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QTabBar::currentChanged, this, &ChromeTabBar::onCurrentChanged);
    connect(this, &QTabBar::tabMoved, this, &ChromeTabBar::onTabMoved);
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
    int candidate = barWidth / n;
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
    const quint64 id = nextTabId_++;
    if (index >= 0 && index <= tabIds_.size()) {
        tabIds_.insert(index, id);
    } else {
        tabIds_.append(id);
    }
    appearProgress_[id] = 0.0;
    if (firstLayout_) {
        // Don't animate the very first tab — feels janky on app start.
        appearProgress_[id] = 1.0;
    } else {
        startAppearAnim(id);
    }
}

void ChromeTabBar::tabRemoved(int index)
{
    QTabBar::tabRemoved(index);
    if (index >= 0 && index < tabIds_.size()) {
        const quint64 id = tabIds_.takeAt(index);
        if (auto *a = appearAnims_.take(id)) {
            a->stop();
            a->deleteLater();
        }
        if (auto *a = closeAnims_.take(id)) {
            a->stop();
            a->deleteLater();
        }
        appearProgress_.remove(id);
        closeProgress_.remove(id);
    }
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
}

void ChromeTabBar::tabLayoutChange()
{
    QTabBar::tabLayoutChange();
    if (firstLayout_) {
        firstLayout_ = false;
        snapPillToCurrent();
        return;
    }
    if (currentIndex() >= 0) {
        const QRectF target = pillRectForIndex(currentIndex());
        if (target != pillGeometry_) {
            animatePillTo(target);
        }
    }
}

void ChromeTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    // On window resize tab geometry shifts in lockstep with the pill, so just
    // re-anchor without animating to avoid a visible drag.
    if (currentIndex() >= 0) {
        if (pillAnim_ && pillAnim_->state() == QAbstractAnimation::Running) {
            pillAnim_->stop();
        }
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

void ChromeTabBar::onTabMoved(int from, int to)
{
    if (from >= 0 && from < tabIds_.size() && to >= 0 && to < tabIds_.size()) {
        const quint64 id = tabIds_.takeAt(from);
        tabIds_.insert(to, id);
    }
    if (currentIndex() >= 0) {
        animatePillTo(pillRectForIndex(currentIndex()));
    }
    update();
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

qreal ChromeTabBar::appearProgressFor(int index) const
{
    if (index < 0 || index >= tabIds_.size()) {
        return 1.0;
    }
    return appearProgress_.value(tabIds_.at(index), 1.0);
}

qreal ChromeTabBar::closeProgressFor(int index) const
{
    if (index < 0 || index >= tabIds_.size()) {
        return 0.0;
    }
    return closeProgress_.value(tabIds_.at(index), 0.0);
}

bool ChromeTabBar::isClosing(int index) const
{
    if (index < 0 || index >= tabIds_.size()) {
        return false;
    }
    return closeAnims_.contains(tabIds_.at(index));
}

void ChromeTabBar::startAppearAnim(quint64 id)
{
    auto *anim = new QVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(kAppearDuration);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    appearAnims_.insert(id, anim);
    connect(anim, &QVariantAnimation::valueChanged, this, [this, id](const QVariant &v) {
        appearProgress_[id] = v.toReal();
        update();
    });
    connect(anim, &QVariantAnimation::finished, this, [this, id]() {
        appearProgress_[id] = 1.0;
        appearAnims_.remove(id);
        update();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChromeTabBar::startCloseAnim(int index)
{
    if (index < 0 || index >= tabIds_.size()) {
        return;
    }
    const quint64 id = tabIds_.at(index);
    if (closeAnims_.contains(id)) {
        return;
    }
    auto *anim = new QVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(kCloseDuration);
    anim->setEasingCurve(QEasingCurve::InCubic);
    closeProgress_[id] = 0.0;
    closeAnims_.insert(id, anim);
    connect(anim, &QVariantAnimation::valueChanged, this, [this, id](const QVariant &v) {
        closeProgress_[id] = v.toReal();
        update();
    });
    connect(anim, &QVariantAnimation::finished, this, [this, id]() {
        closeAnims_.remove(id);
        const int currentIdx = tabIds_.indexOf(id);
        if (currentIdx >= 0) {
            emit tabCloseClicked(currentIdx);
        }
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
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
    }
    if (event->button() == Qt::MiddleButton) {
        const int idx = tabAt(event->pos());
        if (idx >= 0) {
            startCloseAnim(idx);
            event->accept();
            return;
        }
    }
    // Don't let clicks fall on a tab that's already animating closed.
    if (event->button() == Qt::LeftButton) {
        const int idx = tabAt(event->pos());
        if (idx >= 0 && isClosing(idx)) {
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
            startCloseAnim(idx);
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

    // Strip surface — flat M3 surface-container so it visually unifies with
    // the toolbar and pane below (no banding between widgets).
    p.fillRect(rect(), QColor("#eef0f7"));

    // Bottom hairline, like Chrome's top-strip separator.
    p.setPen(QColor(kStripBorderColor));
    p.drawLine(0, height() - 1, width(), height() - 1);

    const int current = currentIndex();
    const int total = count();

    // 1. Inactive tab pills (so the active pill paints over them).
    for (int i = 0; i < total; ++i) {
        if (i == current) {
            continue;
        }
        QRectF pill = pillRectForIndex(i);
        if (pill.isEmpty()) {
            continue;
        }
        const qreal appear = appearProgressFor(i);
        const qreal close = closeProgressFor(i);
        const qreal alpha = std::max(0.0, std::min(1.0, appear * (1.0 - close)));
        if (alpha <= 0.001) {
            continue;
        }
        const qreal yShift = (1.0 - appear) * 6.0 + close * 6.0;
        pill.translate(0, yShift);

        p.save();
        p.setOpacity(alpha);
        const QPainterPath path = roundedPath(pill, kTabPillRadius);

        if (i == hoverTabIndex_) {
            p.fillPath(path, QColor(kTabHoverBackground));
            p.setPen(QPen(QColor(kTabHoverBorder), 1));
        } else {
            // Inactive tabs are essentially flat against the strip with a
            // subtle outline — matches the prototype's `--tab-bg`.
            p.setPen(QPen(QColor(kTabBorder), 1));
        }
        p.drawPath(path);
        p.restore();
    }

    // 2. Active pill (M3 primary-container with primary outline).
    if (current >= 0 && !pillGeometry_.isNull()) {
        const qreal appear = appearProgressFor(current);
        const qreal close = closeProgressFor(current);
        const qreal alpha = std::max(0.0, std::min(1.0, appear * (1.0 - close)));
        if (alpha > 0.001) {
            QRectF pill = pillGeometry_;
            const qreal yShift = (1.0 - appear) * 6.0 + close * 6.0;
            pill.translate(0, yShift);
            const QPainterPath path = roundedPath(pill, kTabPillRadius);
            p.save();
            p.setOpacity(alpha);
            p.fillPath(path, QColor(kPillBase));
            QPen pen{QColor(kPillBorder)};
            pen.setWidthF(1.4);
            p.setPen(pen);
            p.drawPath(path);
            p.restore();
        }
    }

    // 3. Per-tab content (favicon + title + close button).
    for (int i = 0; i < total; ++i) {
        QRect r = tabRect(i);
        if (r.isEmpty()) {
            continue;
        }
        const qreal appear = appearProgressFor(i);
        const qreal close = closeProgressFor(i);
        const qreal alpha = std::max(0.0, std::min(1.0, appear * (1.0 - close)));
        if (alpha <= 0.001) {
            continue;
        }
        const qreal yShift = (1.0 - appear) * 6.0 + close * 6.0;
        r.translate(0, qRound(yShift));

        p.save();
        p.setOpacity(alpha);

        const bool isActive = (i == current);
        const QColor textColor(isActive ? kActiveText
                                        : (i == hoverTabIndex_ ? kTabHoverText : kTabText));

        // Favicon
        const QIcon icon = tabIcon(i);
        QRect favRect(r.left() + kPaddingLeft,
                      r.top() + (r.height() - kFaviconSize) / 2,
                      kFaviconSize, kFaviconSize);
        if (!icon.isNull()) {
            icon.paint(&p, favRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
        } else {
            p.setPen(Qt::NoPen);
            QColor dot = textColor;
            dot.setAlphaF(0.18);
            p.setBrush(dot);
            const int d = 10;
            p.drawEllipse(favRect.center(), d / 2, d / 2);
        }

        // Title
        QRect closeR(r.right() - kPaddingRight - kCloseSize,
                     r.top() + (r.height() - kCloseSize) / 2,
                     kCloseSize, kCloseSize);
        QRect titleRect(favRect.right() + 11,
                        r.top(),
                        closeR.left() - (favRect.right() + 11) - 8,
                        r.height());
        if (titleRect.width() > 0) {
            QFont f = font();
            f.setPixelSize(14);
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
            QColor bg(0, 26, 65, isPressed ? 60 : 38); // M3 on-primary-container tint
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawEllipse(closeR);
        }
        QPen xPen{QColor(kCloseIconColor)};
        xPen.setWidthF(1.5);
        xPen.setCapStyle(Qt::RoundCap);
        p.setPen(xPen);
        const qreal pad = 7.5;
        const QPointF c = QRectF(closeR).center();
        const qreal half = (kCloseSize / 2.0) - pad;
        p.save();
        p.translate(c);
        if (isHoverClose) {
            p.rotate(90);
        }
        p.drawLine(QPointF(-half, -half), QPointF(half, half));
        p.drawLine(QPointF(-half, half), QPointF(half, -half));
        p.restore();

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
