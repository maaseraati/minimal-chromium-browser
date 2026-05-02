#include "chrometabbar.h"

#include "thememanager.h"

#include <QEnterEvent>
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
    selectionAnimation_->setDuration(170);
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

    for (int index = 0; index < count(); ++index) {
        if (index != currentIndex()) {
            const QRectF rect = tabRect(index).adjusted(0, 5, 0, 0);
            painter.fillPath(tabPath(rect), tabColor(index, false));
        }
    }

    if (currentIndex() >= 0) {
        const QRectF selectedRect(selectionOffset_, 0, tabRect(currentIndex()).width(), height());
        painter.fillPath(tabPath(selectedRect.adjusted(0, 4, 0, 1)), tabColor(currentIndex(), true));
    }

    const QColor separator(ThemeManager::instance()->isLight()
        ? QColor(QStringLiteral("#d7dbe4"))
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
        QStyleOptionTab option;
        initStyleOption(&option, index);
        option.shape = QTabBar::RoundedNorth;
        option.state &= ~QStyle::State_Selected;
        if (index == currentIndex()) {
            option.state |= QStyle::State_Selected;
        }
        option.rect = tabRect(index).adjusted(12, 4, -26, 0);
        option.palette.setColor(QPalette::WindowText, textColor(index, index == currentIndex()));
        option.palette.setColor(QPalette::Text, textColor(index, index == currentIndex()));
        style()->drawControl(QStyle::CE_TabBarTabLabel, &option, &painter, this);
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
        ? QColor(QStringLiteral("#dee1e6"))
        : QColor(QStringLiteral("#11151d"));
}

QColor ChromeTabBar::tabColor(int index, bool selected) const
{
    const bool light = ThemeManager::instance()->isLight();
    if (selected) {
        return light ? QColor(QStringLiteral("#f8fafd")) : QColor(QStringLiteral("#1f232d"));
    }

    const qreal hover = hoverValue(index);
    const QColor base = light ? QColor(QStringLiteral("#dee1e6")) : QColor(QStringLiteral("#11151d"));
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
        animation->setDuration(120);
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
