#include "chrometabbar.h"

#include "thememanager.h"

#include <QApplication>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOptionTab>
#include <QVariantAnimation>

ChromeTabBar::ChromeTabBar(QWidget *parent)
    : QTabBar(parent),
      pillGeometry_(),
      hoveredIndex_(-1),
      selectionAnimation_(new QPropertyAnimation(this, "pillGeometry", this)),
      closingTab_(false)
{
    setDrawBase(false);
    setElideMode(Qt::ElideRight);
    setExpanding(false);
    setIconSize(QSize(16, 16));
    setMouseTracking(true);
    setMovable(true);
    setTabsClosable(true);
    setUsesScrollButtons(true);
    setFixedHeight(58);
    setCursor(Qt::OpenHandCursor);
    setStyleSheet(QStringLiteral("QTabBar::close-button { image: none; width: 0px; height: 0px; }"));
    selectionAnimation_->setDuration(240);
    selectionAnimation_->setEasingCurve(QEasingCurve::OutCubic);
}

QRectF ChromeTabBar::pillGeometry() const
{
    return pillGeometry_;
}

void ChromeTabBar::setPillGeometry(const QRectF &geometry)
{
    pillGeometry_ = geometry;
    update();
}

void ChromeTabBar::refreshTheme()
{
    update();
}

void ChromeTabBar::animateSelectionToCurrent()
{
    animateSelection(currentIndex());
}

void ChromeTabBar::animateTabClose(int index, const std::function<void()> &finished)
{
    if (index < 0 || index >= count()) {
        if (finished) {
            finished();
        }
        return;
    }
    closingTab_ = true;
    animateReveal(index, revealValue(index), 0, 220, [this, finished] {
        if (finished) {
            finished();
        }
        closingTab_ = false;
    });
}

void ChromeTabBar::enterEvent(QEnterEvent *event)
{
    QTabBar::enterEvent(event);
    const int index = tabAt(event->position().toPoint());
    if (index >= 0) {
        hoveredIndex_ = index;
        animateHover(index, 1);
    }
}

void ChromeTabBar::leaveEvent(QEvent *event)
{
    QTabBar::leaveEvent(event);
    if (hoveredIndex_ >= 0) {
        animateHover(hoveredIndex_, 0);
    }
    hoveredIndex_ = -1;
}

void ChromeTabBar::mouseMoveEvent(QMouseEvent *event)
{
    const int index = tabAt(event->pos());
    if (index != hoveredIndex_) {
        if (hoveredIndex_ >= 0) {
            animateHover(hoveredIndex_, 0);
        }
        hoveredIndex_ = index;
        if (hoveredIndex_ >= 0) {
            animateHover(hoveredIndex_, 1);
        }
    }
    QTabBar::mouseMoveEvent(event);
}

void ChromeTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    QTabBar::mouseReleaseEvent(event);
    animateSelectionToCurrent();
}

void ChromeTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), chromeBackground());

    const QColor separator(QStringLiteral("#dfe4d6"));
    painter.setPen(separator);

    for (int index = 0; index < count(); ++index) {
        if (index == currentIndex()) {
            continue;
        }
        const qreal hover = hoverValue(index);
        if (hover <= 0) {
            continue;
        }
        const QRectF rect = pillRectForIndex(index);
        QColor hoverColor(QStringLiteral("#edf1e2"));
        hoverColor.setAlphaF(0.86 * hover);
        painter.fillPath(tabPath(rect), hoverColor);
    }

    if (currentIndex() >= 0 && !pillGeometry_.isNull()) {
        QLinearGradient gradient(pillGeometry_.topLeft(), pillGeometry_.bottomLeft());
        gradient.setColorAt(0, QColor(255, 255, 255, 128));
        gradient.setColorAt(1, QColor(QStringLiteral("#d9e5c9")));
        painter.fillPath(tabPath(pillGeometry_), gradient);
        painter.setPen(QPen(QColor(QStringLiteral("#627748")), 1.5));
        painter.drawPath(tabPath(pillGeometry_));
    }

    for (int index = 0; index < count(); ++index) {
        const qreal reveal = revealValue(index);
        if (reveal <= 0.02) {
            continue;
        }
        const QRect tabBounds = tabRect(index);
        const QRectF tabPaintRect = pillRectForIndex(index);
        if (index != currentIndex()) {
            painter.setPen(QPen(QColor(QStringLiteral("#e1e5d8")), 1));
            painter.fillPath(tabPath(tabPaintRect), tabColor(index, false));
            painter.drawPath(tabPath(tabPaintRect));
        }

        QRect contentRect = tabBounds.adjusted(18, 8, -36, -6);
        const QIcon icon = tabIcon(index);
        if (!icon.isNull()) {
            const QSize iconSize(22, 22);
            const QRect iconRect(contentRect.left(),
                                 contentRect.center().y() - iconSize.height() / 2,
                                 iconSize.width(),
                                 iconSize.height());
            icon.paint(&painter, iconRect);
            contentRect.setLeft(iconRect.right() + 8);
        }

        painter.setPen(textColor(index, index == currentIndex()));
        const QString text = fontMetrics().elidedText(tabText(index), Qt::ElideRight, contentRect.width());
        painter.drawText(contentRect, Qt::AlignVCenter | Qt::AlignLeft, text);

        const QRect closeRect(tabBounds.right() - 32,
                              tabBounds.center().y() - 12,
                              24,
                              24);
        painter.setPen(QPen(QColor(QStringLiteral("#1f241d")), 1.7, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(closeRect.center() + QPoint(-4, -4), closeRect.center() + QPoint(4, 4));
        painter.drawLine(closeRect.center() + QPoint(4, -4), closeRect.center() + QPoint(-4, 4));
    }
}

void ChromeTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    if (selectionAnimation_->state() == QAbstractAnimation::Stopped) {
        setPillGeometry(pillRectForIndex(currentIndex()));
    }
}

QSize ChromeTabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);
    size.setWidth(qRound(310 * revealValue(index)));
    size.setHeight(44);
    return size;
}

void ChromeTabBar::tabLayoutChange()
{
    QTabBar::tabLayoutChange();
    if (selectionAnimation_->state() == QAbstractAnimation::Stopped) {
        setPillGeometry(pillRectForIndex(currentIndex()));
    }
}

void ChromeTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    normalizeHoverState();
    revealValues_[index] = 0;
    animateReveal(index, 0, 1, 260);
    setPillGeometry(pillRectForIndex(currentIndex()));
}

void ChromeTabBar::tabRemoved(int index)
{
    Q_UNUSED(index);
    QTabBar::tabRemoved(index);
    if (!closingTab_) {
        normalizeHoverState();
    }
    revealValues_.clear();
    qDeleteAll(revealAnimations_);
    revealAnimations_.clear();
    setPillGeometry(pillRectForIndex(currentIndex()));
}

QColor ChromeTabBar::chromeBackground() const
{
    return QColor(QStringLiteral("#fbfcf4"));
}

QColor ChromeTabBar::tabColor(int index, bool selected) const
{
    Q_UNUSED(index);
    if (selected) {
        return QColor(QStringLiteral("#d9e5c9"));
    }

    return QColor(250, 251, 244, 235);
}

QColor ChromeTabBar::textColor(int index, bool selected) const
{
    Q_UNUSED(index);
    if (selected) {
        return QColor(QStringLiteral("#1e211b"));
    }
    return QColor(QStringLiteral("#555b4f"));
}

QPainterPath ChromeTabBar::tabPath(const QRectF &rect) const
{
    QPainterPath path;
    path.addRoundedRect(rect, 15, 15);
    return path;
}

QRectF ChromeTabBar::pillRectForIndex(int index) const
{
    if (index < 0 || index >= count()) {
        return QRectF();
    }
    const QRect rect = tabRect(index);
    return QRectF(rect.left(), 8, rect.width(), 44).adjusted(0.5, 0.5, -0.5, -0.5);
}

void ChromeTabBar::animateSelection(int index)
{
    if (index < 0) {
        return;
    }
    selectionAnimation_->stop();
    selectionAnimation_->setStartValue(pillGeometry_.isNull() ? pillRectForIndex(index) : pillGeometry_);
    selectionAnimation_->setEndValue(pillRectForIndex(index));
    selectionAnimation_->start();
}

void ChromeTabBar::animateHover(int index, qreal endValue)
{
    auto *animation = hoverAnimations_.value(index);
    if (!animation) {
        animation = new QVariantAnimation(this);
        animation->setDuration(150);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QVariantAnimation::valueChanged, this, [this, index](const QVariant &value) {
            hoverValues_[index] = value.toReal();
            update(tabRect(index).adjusted(-4, 0, 4, 0));
        });
        hoverAnimations_.insert(index, animation);
    }
    animation->stop();
    animation->setStartValue(hoverValue(index));
    animation->setEndValue(endValue);
    animation->start();
}

void ChromeTabBar::animateReveal(int index, qreal startValue, qreal endValue, int duration,
                                 const std::function<void()> &finished)
{
    auto *animation = revealAnimations_.value(index);
    if (!animation) {
        animation = new QVariantAnimation(this);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QVariantAnimation::valueChanged, this, [this, index](const QVariant &value) {
            revealValues_[index] = value.toReal();
            updateGeometry();
            update();
        });
        revealAnimations_.insert(index, animation);
    }
    animation->stop();
    disconnect(animation, &QVariantAnimation::finished, this, nullptr);
    animation->setDuration(duration);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    if (finished) {
        connect(animation, &QVariantAnimation::finished, this, [this, animation, finished] {
            disconnect(animation, &QVariantAnimation::finished, this, nullptr);
            finished();
        });
    }
    animation->start();
}

qreal ChromeTabBar::hoverValue(int index) const
{
    return hoverValues_.value(index, 0);
}

qreal ChromeTabBar::revealValue(int index) const
{
    return revealValues_.value(index, 1);
}

void ChromeTabBar::normalizeHoverState()
{
    hoveredIndex_ = -1;
    hoverValues_.clear();
    qDeleteAll(hoverAnimations_);
    hoverAnimations_.clear();
    qDeleteAll(revealAnimations_);
    revealAnimations_.clear();
}
