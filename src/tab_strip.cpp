#include "tab_strip.h"

#include "browser_window.h"
#include "tab_button.h"

#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QToolButton>
#include <QWindow>

namespace morphine {

TabStrip::TabStrip(BrowserWindow *window) : m_window(window) {}

bool TabStrip::isDragTarget(const QPoint &pos) const
{
    QWidget *widget = childAt(pos);
    while (widget != nullptr && widget != this) {
        if (dynamic_cast<TabButton *>(widget) != nullptr || qobject_cast<QToolButton *>(widget) != nullptr ||
            qobject_cast<QLineEdit *>(widget) != nullptr) {
            return false;
        }
        widget = widget->parentWidget();
    }
    return true;
}

void TabStrip::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragTarget(event->pos())) {
        m_pressed = true;
        m_pressPos = event->pos();
        m_pressGlobal = event->globalPos();
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void TabStrip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed && (event->buttons() & Qt::LeftButton) && !m_dragging) {
        const QPoint delta = event->globalPos() - m_pressGlobal;
        if (delta.manhattanLength() >= QApplication::startDragDistance()) {
            m_dragging = true;
            beginWindowMove();
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TabStrip::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void TabStrip::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragTarget(event->pos())) {
        m_window->toggleWindowMaximized();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TabStrip::beginWindowMove()
{
    if (m_window->isMaximized()) {
        m_window->showNormal();
    }
    if (m_window->windowHandle() != nullptr) {
        m_window->windowHandle()->startSystemMove();
    }
}

}  // namespace morphine
