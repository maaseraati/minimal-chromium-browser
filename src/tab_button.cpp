#include "tab_button.h"

#include "assets.h"
#include "browser_window.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QToolButton>

namespace morphine {

TabButton::TabButton(int index, const QString &title, BrowserWindow *window)
    : m_index(index), m_window(window)
{
    setObjectName("browserTab");
    setFixedHeight(48);
    setMinimumWidth(0);
    setMaximumWidth(175);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 10, 0);
    layout->setSpacing(8);

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
}

void TabButton::setActive(bool active)
{
    m_active = active;
    setProperty("active", active);
    style()->unpolish(this);
    style()->polish(this);
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

void TabButton::enterEvent(QEvent *event)
{
    m_closeButton->show();
    QWidget::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (!m_active) {
        m_closeButton->hide();
    }
    QWidget::leaveEvent(event);
}

}  // namespace morphine
