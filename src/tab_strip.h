#pragma once

#include <QPoint>
#include <QWidget>

class QMouseEvent;

namespace morphine {

class BrowserWindow;

class TabStrip : public QWidget {
public:
    explicit TabStrip(BrowserWindow *window);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool isDragTarget(const QPoint &pos) const;
    void beginWindowMove();

    BrowserWindow *m_window;
    QPoint m_pressPos;
    QPoint m_pressGlobal;
    bool m_pressed = false;
    bool m_dragging = false;
};

}  // namespace morphine
