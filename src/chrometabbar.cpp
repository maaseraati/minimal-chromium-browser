#include "chrometabbar.h"

#include "thememanager.h"

#include <QEnterEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOptionTab>
#include <QVariantAnimation>

ChromeTabBar::ChromeTabBar(QWidget *parent)
    : QTabBar(parent),
      selectionOffset_(0),
      hoveredIndex_(-1),
      selectionAnimation_(new QPropertyAnimation(this, "selectionOffset", this))
{
    setDrawBase(false);
    setElideMode(Qt::ElideRight);
    setExpanding(false);
    setIconSize(QSize(16, 16));
    setMouseTracking(true);
    setMovable(true);
    setTabsClosable(true);
    setUsesScrollButtons(true);
    setFixedHeight(40);
    selectionAnimation_->setDuration(210);
    selectionAnimation_->setEasingCurve(QEasingCurve::OutCubic);
}

qreal ChromeTabBar::selectionOffset() const
{
    return selectionOffset_;
}

void ChromeTabBar::setSelectionOffset(qreal offset)
{
    selectionOffset_ = offset;
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

void ChromeTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), chromeBackground());

    const QColor separator(ThemeManager::instance()->isLight()
        ? QColor(QStringLiteral("#c7cbd3"))
        : QColor(QStringLiteral("#2a2f3a")));
    painter.setPen(separator);
    for (int index = 1; index < count(); ++index) {
        if (index == currentIndex() || index - 1 == currentIndex()) {
            continue;
        }
        const QRect rect = tabRect(index);
        painter.drawLine(rect.left(), rect.top() + 12, rect.left(), rect.bottom() - 10);
    }

    for (int index = 0; index < count(); ++index) {
        if (index == currentIndex()) {
            continue;
        }
        const qreal hover = hoverValue(index);
        if (hover <= 0) {
            continue;
        }
        const QRectF rect = tabRect(index).adjusted(2, 5, -2, 2);
        painter.fillPath(tabPath(rect), tabColor(index, false));
    }

    if (currentIndex() >= 0) {
        const QRect currentRect = tabRect(currentIndex());
        const QRectF selectedRect(selectionOffset_, 0, currentRect.width(), height());
        painter.fillPath(tabPath(selectedRect.adjusted(0, 4, 0, 1)), tabColor(currentIndex(), true));
    }

    for (int index = 0; index < count(); ++index) {
        QRect contentRect = tabRect(index).adjusted(14, 5, -34, -4);
        const QIcon icon = tabIcon(index);
        if (!icon.isNull()) {
            const QSize iconSize(16, 16);
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
    }
}

void ChromeTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    if (!selectionAnimation_->state()) {
        setSelectionOffset(currentIndex() >= 0 ? tabRect(currentIndex()).x() : 0);
    }
}

void ChromeTabBar::tabLayoutChange()
{
    QTabBar::tabLayoutChange();
    if (!selectionAnimation_->state()) {
        setSelectionOffset(currentIndex() >= 0 ? tabRect(currentIndex()).x() : 0);
    }
}

void ChromeTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    normalizeHoverState();
    setSelectionOffset(tabRect(currentIndex()).x());
}

void ChromeTabBar::tabRemoved(int index)
{
    Q_UNUSED(index);
    QTabBar::tabRemoved(index);
    normalizeHoverState();
    setSelectionOffset(currentIndex() >= 0 ? tabRect(currentIndex()).x() : 0);
}

QColor ChromeTabBar::chromeBackground() const
{
    return ThemeManager::instance()->isLight()
        ? QColor(QStringLiteral("#dfe3ea"))
        : QColor(QStringLiteral("#11151d"));
}

QColor ChromeTabBar::tabColor(int index, bool selected) const
{
    const bool light = ThemeManager::instance()->isLight();
    if (selected) {
        return light ? QColor(QStringLiteral("#f8fafd")) : QColor(QStringLiteral("#20242e"));
    }

    const qreal hover = hoverValue(index);
    const QColor base = light ? QColor(QStringLiteral("#dfe3ea")) : QColor(QStringLiteral("#11151d"));
    const QColor hoverColor = light ? QColor(QStringLiteral("#edf0f5")) : QColor(QStringLiteral("#1a1f29"));
    return QColor::fromRgbF(
        base.redF() + (hoverColor.redF() - base.redF()) * hover,
        base.greenF() + (hoverColor.greenF() - base.greenF()) * hover,
        base.blueF() + (hoverColor.blueF() - base.blueF()) * hover);
}

QColor ChromeTabBar::textColor(int index, bool selected) const
{
    Q_UNUSED(index);
    const bool light = ThemeManager::instance()->isLight();
    if (selected) {
        return light ? QColor(QStringLiteral("#202124")) : QColor(QStringLiteral("#f1f3f4"));
    }
    return light ? QColor(QStringLiteral("#3c4043")) : QColor(QStringLiteral("#d7dce5"));
}

QPainterPath ChromeTabBar::tabPath(const QRectF &rect) const
{
    constexpr qreal radius = 12;
    QPainterPath path;
    path.moveTo(rect.left(), rect.bottom());
    path.cubicTo(rect.left() + 7, rect.bottom(), rect.left() + 7, rect.top(), rect.left() + radius, rect.top());
    path.lineTo(rect.right() - radius, rect.top());
    path.cubicTo(rect.right() - 7, rect.top(), rect.right() - 7, rect.bottom(), rect.right(), rect.bottom());
    path.closeSubpath();
    return path;
}

void ChromeTabBar::animateSelection(int index)
{
    if (index < 0) {
        return;
    }
    selectionAnimation_->stop();
    selectionAnimation_->setStartValue(selectionOffset_);
    selectionAnimation_->setEndValue(tabRect(index).x());
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

qreal ChromeTabBar::hoverValue(int index) const
{
    return hoverValues_.value(index, 0);
}

void ChromeTabBar::normalizeHoverState()
{
    hoveredIndex_ = -1;
    hoverValues_.clear();
    qDeleteAll(hoverAnimations_);
    hoverAnimations_.clear();
}
