#pragma once

#include <QWidget>

class QEvent;
class QIcon;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QToolButton;
class QVariantAnimation;

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
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void animateHover(bool hovered);
    void animateActive(bool active);
    void updateElidedTitle();

    int m_index;
    BrowserWindow *m_window;
    QString m_titleText;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QToolButton *m_closeButton;
    QVariantAnimation *m_hoverAnimation = nullptr;
    QVariantAnimation *m_activeAnimation = nullptr;
    qreal m_hoverProgress = 0;
    qreal m_activeProgress = 0;
    int m_baseWidth = 138;
    bool m_active = false;
    bool m_hasSiteIcon = false;
};

}  // namespace morphine
