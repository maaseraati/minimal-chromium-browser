#include "tab_button.h"

#include "assets.h"
#include "browser_window.h"

#include <QEvent>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QToolButton>
#include <QVariantAnimation>

namespace morphine {

TabButton::TabButton(int index, const QString &title, BrowserWindow *window)
    : m_index(index), m_window(window)
{
    setObjectName("browserTab");
    setFixedHeight(44);
    setFixedWidth(m_baseWidth);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 0, 9, 0);
    layout->setSpacing(7);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("tabBadge");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->hide();
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("tabTitle");
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    layout->addWidget(m_titleLabel, 1);

    m_closeButton = new QToolButton(this);
    m_closeButton->setObjectName("tabCloseBtn");
    m_closeButton->setIcon(svgIcon("close", "#ffffff", 15));
    m_closeButton->setIconSize(QSize(13, 13));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(true);
    m_closeButton->hide();
    connect(m_closeButton, &QToolButton::clicked, this, [this]() { m_window->closeTab(m_index); });
    layout->addWidget(m_closeButton);

    m_widthAnimation = new QVariantAnimation(this);
    m_widthAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_widthAnimation->setDuration(145);
    connect(m_widthAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setFixedWidth(value.toInt());
    });

    m_hoverAnimation = new QVariantAnimation(this);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_hoverAnimation->setDuration(120);
    connect(m_hoverAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverProgress = value.toReal();
        update();
    });

    m_activeAnimation = new QVariantAnimation(this);
    m_activeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_activeAnimation->setDuration(150);
    connect(m_activeAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_activeProgress = value.toReal();
        update();
    });
}

void TabButton::setActive(bool active)
{
    if (m_active == active) {
        return;
    }
    m_active = active;
    setProperty("active", active);
    style()->unpolish(this);
    style()->polish(this);
    animateActive(active);
    if (!m_hasSiteIcon && m_titleLabel->text().compare("History", Qt::CaseInsensitive) == 0) {
        const QString color = active ? QString("#ffffff") : QString(OnSurfaceVariant);
        m_iconLabel->setPixmap(svgIcon("history", color, 20).pixmap(20, 20));
    }
    if (active) {
        m_closeButton->show();
        m_closeButton->setIcon(svgIcon("close", "#ffffff", 15));
    } else if (!underMouse()) {
        m_closeButton->hide();
        m_closeButton->setIcon(svgIcon("close", OnSurfaceVariant, 15));
    }
}

void TabButton::animatePreview(bool previewed)
{
    m_widthAnimation->stop();
    m_widthAnimation->setStartValue(width());
    m_widthAnimation->setEndValue(previewed ? m_previewWidth : m_baseWidth);
    m_widthAnimation->start();
}

void TabButton::setTitleText(const QString &title)
{
    m_titleLabel->setText(title);
    if (title.compare("History", Qt::CaseInsensitive) == 0 && !m_hasSiteIcon) {
        const QString color = m_active ? QString("#ffffff") : QString(OnSurfaceVariant);
        m_iconLabel->setPixmap(svgIcon("history", color, 20).pixmap(20, 20));
        m_iconLabel->show();
    }
}

void TabButton::setIconPixmap(const QIcon &icon)
{
    if (icon.isNull()) {
        m_hasSiteIcon = false;
        m_iconLabel->clear();
        m_iconLabel->hide();
        return;
    }
    const QPixmap pixmap = icon.pixmap(16, 16);
    if (pixmap.isNull()) {
        m_hasSiteIcon = false;
        m_iconLabel->clear();
        m_iconLabel->hide();
        return;
    }
    m_hasSiteIcon = true;
    m_iconLabel->setPixmap(pixmap);
    m_iconLabel->show();
}

void TabButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_window->selectTab(m_index);
    }
    QWidget::mousePressEvent(event);
}

void TabButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF rect = QRectF(0.5, 0.5, width() - 1.0, height() - 0.5);
    const qreal radius = 12.0;

    QColor base(196, 222, 255);
    QColor hover(176, 211, 255);
    QColor activeStart(115, 171, 255);
    QColor activeEnd(78, 143, 240);
    QColor border(116, 160, 225, 95);

    QColor mixed = base;
    mixed.setRedF(base.redF() + (hover.redF() - base.redF()) * m_hoverProgress);
    mixed.setGreenF(base.greenF() + (hover.greenF() - base.greenF()) * m_hoverProgress);
    mixed.setBlueF(base.blueF() + (hover.blueF() - base.blueF()) * m_hoverProgress);

    painter.setPen(QPen(border, 1));
    painter.setBrush(mixed);
    painter.drawRoundedRect(rect, radius, radius);

    if (m_activeProgress > 0.01) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        activeStart.setAlphaF(m_activeProgress);
        activeEnd.setAlphaF(m_activeProgress);
        gradient.setColorAt(0, activeStart);
        gradient.setColorAt(1, activeEnd);
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawRoundedRect(rect, radius, radius);
    }
}

void TabButton::enterEvent(QEvent *event)
{
    animateHover(true);
    animatePreview(true);
    m_closeButton->show();
    QWidget::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    animateHover(false);
    if (!m_active) {
        m_closeButton->hide();
    }
    animatePreview(false);
    QWidget::leaveEvent(event);
}

void TabButton::animateHover(bool hovered)
{
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(hovered ? 1.0 : 0.0);
    m_hoverAnimation->start();
}

void TabButton::animateActive(bool active)
{
    m_activeAnimation->stop();
    m_activeAnimation->setStartValue(m_activeProgress);
    m_activeAnimation->setEndValue(active ? 1.0 : 0.0);
    m_activeAnimation->start();
}

}  // namespace morphine
