#pragma once

#include <QWidget>

class QEvent;
class QIcon;
class QLabel;
class QMouseEvent;
class QToolButton;

namespace morphine {

class BrowserWindow;

class TabButton : public QWidget {
public:
    explicit TabButton(int index, const QString &title, BrowserWindow *window);

    void setActive(bool active);
    void setTitleText(const QString &title);
    void setIconPixmap(const QIcon &icon);
    int index() const { return m_index; }
    void setIndex(int index) { m_index = index; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_index;
    BrowserWindow *m_window;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QToolButton *m_closeButton;
    bool m_active = false;
    bool m_hasSiteIcon = false;
};

}  // namespace morphine
