#pragma once

#include <QColor>
#include <QHash>
#include <QTabBar>

class QPropertyAnimation;
class QVariantAnimation;

class ChromeTabBar final : public QTabBar {
    Q_OBJECT
    Q_PROPERTY(qreal selectionOffset READ selectionOffset WRITE setSelectionOffset)

public:
    explicit ChromeTabBar(QWidget *parent = nullptr);

    qreal selectionOffset() const;
    void setSelectionOffset(qreal offset);
    void refreshTheme();
    void animateSelectionToCurrent();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    QColor chromeBackground() const;
    QColor tabColor(int index, bool selected) const;
    QColor textColor(int index, bool selected) const;
    QPainterPath tabPath(const QRectF &rect) const;
    void animateSelection(int index);
    void animateHover(int index, qreal endValue);
    qreal hoverValue(int index) const;
    void normalizeHoverState();

    qreal selectionOffset_;
    int hoveredIndex_;
    QHash<int, qreal> hoverValues_;
    QHash<int, QVariantAnimation *> hoverAnimations_;
    QPropertyAnimation *selectionAnimation_;
};
