#pragma once

#include <QWidget>

class QEvent;
class QIcon;
class QLabel;
class QMouseEvent;
class QPaintEvent;
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
    void animatePreview(bool previewed);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void animateHover(bool hovered);
    void animateActive(bool active);

    int m_index;
    BrowserWindow *m_window;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QToolButton *m_closeButton;
    QVariantAnimation *m_widthAnimation = nullptr;
    QVariantAnimation *m_hoverAnimation = nullptr;
    QVariantAnimation *m_activeAnimation = nullptr;
    qreal m_hoverProgress = 0;
    qreal m_activeProgress = 0;
    int m_baseWidth = 118;
    int m_previewWidth = 142;
    bool m_active = false;
    bool m_hasSiteIcon = false;
};

}  // namespace morphine
