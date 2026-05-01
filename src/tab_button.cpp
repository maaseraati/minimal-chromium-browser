#include "tab_button.h"

#include "assets.h"
#include "browser_window.h"

#include <QEvent>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QAbstractAnimation>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVariantAnimation>

namespace morphine {

TabButton::TabButton(int index, const QString &title, BrowserWindow *window, bool animateIn)
    : m_index(index), m_window(window)
{
    setObjectName("browserTab");
    setFixedHeight(34);
    setFixedWidth(animateIn ? 0 : m_baseWidth);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 6, 0);
    layout->setSpacing(6);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("tabBadge");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->hide();
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("tabTitle");
    m_titleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_titleLabel->setMinimumWidth(0);
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->installEventFilter(this);
    m_titleText = title;
    layout->addWidget(m_titleLabel, 1);

    m_closeButton = new QToolButton(this);
    m_closeButton->setObjectName("tabCloseBtn");
    m_closeButton->setIcon(svgIcon("close", "#ffffff", 13));
    m_closeButton->setIconSize(QSize(11, 11));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(true);
    m_closeButton->hide();
    connect(m_closeButton, &QToolButton::clicked, this, [this]() { m_window->closeTab(m_index); });
    layout->addWidget(m_closeButton);

    m_hoverAnimation = new QVariantAnimation(this);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_hoverAnimation->setDuration(120);
    connect(m_hoverAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverProgress = value.toReal();
        update();
    });

    m_activeAnimation = new QVariantAnimation(this);
    m_activeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_activeAnimation->setDuration(180);
    connect(m_activeAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_activeProgress = value.toReal();
        update();
    });

    if (animateIn) {
        auto *openAnim = new QVariantAnimation(this);
        openAnim->setStartValue(0);
        openAnim->setEndValue(m_baseWidth);
        openAnim->setDuration(220);
        openAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(openAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (!m_closing) {
                setFixedWidth(value.toInt());
            }
        });
        QTimer::singleShot(0, openAnim, [openAnim]() { openAnim->start(QAbstractAnimation::DeleteWhenStopped); });
    }
}

void TabButton::animateClose(std::function<void()> done)
{
    if (m_closing) {
        return;
    }
    m_closing = true;
    setEnabled(false);
    m_titleLabel->hide();
    m_iconLabel->hide();
    m_closeButton->hide();

    auto *anim = new QVariantAnimation(this);
    anim->setStartValue(width());
    anim->setEndValue(0);
    anim->setDuration(160);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setFixedWidth(value.toInt());
    });
    connect(anim, &QVariantAnimation::finished, this, [done]() {
        if (done) done();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
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
        m_closeButton->setIcon(svgIcon("close", "#ffffff", 13));
    } else if (!underMouse()) {
        m_closeButton->hide();
        m_closeButton->setIcon(svgIcon("close", OnSurfaceVariant, 13));
    }
}

void TabButton::setTitleText(const QString &title)
{
    m_titleText = title;
    updateElidedTitle();
    if (title.compare("History", Qt::CaseInsensitive) == 0 && !m_hasSiteIcon) {
        const QString color = m_active ? QString("#ffffff") : QString(OnSurfaceVariant);
        m_iconLabel->setPixmap(svgIcon("history", color, 20).pixmap(20, 20));
        m_iconLabel->show();
    }
}

void TabButton::updateElidedTitle()
{
    if (m_titleLabel == nullptr) {
        return;
    }
    const int available = std::max(0, m_titleLabel->width());
    const QFontMetrics metrics(m_titleLabel->font());
    const QString elided = metrics.elidedText(m_titleText, Qt::ElideRight, available);
    if (m_titleLabel->text() != elided) {
        m_titleLabel->setText(elided);
    }
}

bool TabButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleLabel && event->type() == QEvent::Resize) {
        updateElidedTitle();
    }
    return QWidget::eventFilter(watched, event);
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

void TabButton::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateElidedTitle();
}

void TabButton::paintEvent(QPaintEvent *)
{
    if (width() <= 1) {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF rect = QRectF(0, 0, width(), height());

    const QColor base(196, 218, 246);
    const QColor hover(178, 205, 240);
    const QColor activeStart(101, 165, 255);
    const QColor activeEnd(63, 134, 240);

    QColor mixed = base;
    mixed.setRedF(base.redF() + (hover.redF() - base.redF()) * m_hoverProgress);
    mixed.setGreenF(base.greenF() + (hover.greenF() - base.greenF()) * m_hoverProgress);
    mixed.setBlueF(base.blueF() + (hover.blueF() - base.blueF()) * m_hoverProgress);

    const qreal r = 12.0;
    QPainterPath path;
    path.moveTo(rect.left(), rect.bottom());
    path.lineTo(rect.left(), rect.top() + r);
    path.quadTo(rect.left(), rect.top(), rect.left() + r, rect.top());
    path.lineTo(rect.right() - r, rect.top());
    path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + r);
    path.lineTo(rect.right(), rect.bottom());
    path.closeSubpath();

    painter.setPen(Qt::NoPen);
    if (m_active) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, activeStart);
        gradient.setColorAt(1, activeEnd);
        painter.setBrush(gradient);
    } else {
        painter.setBrush(mixed);
    }
    painter.drawPath(path);
}

void TabButton::enterEvent(QEvent *event)
{
    if (!m_active) {
        animateHover(true);
    }
    m_closeButton->show();
    QWidget::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (!m_active) {
        animateHover(false);
    }
    if (!m_active) {
        m_closeButton->hide();
    }
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
