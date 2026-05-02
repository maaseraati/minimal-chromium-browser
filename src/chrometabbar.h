#pragma once

#include <QColor>
#include <QHash>
#include <QRectF>
#include <QTabBar>

#include <functional>

class QPropertyAnimation;
class QVariantAnimation;

class ChromeTabBar final : public QTabBar {
    Q_OBJECT
    Q_PROPERTY(QRectF pillGeometry READ pillGeometry WRITE setPillGeometry)

public:
    explicit ChromeTabBar(QWidget *parent = nullptr);

    QRectF pillGeometry() const;
    void setPillGeometry(const QRectF &geometry);
    void refreshTheme();
    void animateSelectionToCurrent();
    void animateTabClose(int index, const std::function<void()> &finished);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize tabSizeHint(int index) const override;
    void tabLayoutChange() override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    QColor chromeBackground() const;
    QColor tabColor(int index, bool selected) const;
    QColor textColor(int index, bool selected) const;
    QPainterPath tabPath(const QRectF &rect) const;
    QRectF pillRectForIndex(int index) const;
    int targetTabWidth() const;
    void animateSelection(int index);
    void animateHover(int index, qreal endValue);
    void animateReveal(int index, qreal startValue, qreal endValue, int duration,
                       const std::function<void()> &finished = {});
    qreal hoverValue(int index) const;
    qreal revealValue(int index) const;
    void normalizeHoverState();

    QRectF pillGeometry_;
    int hoveredIndex_;
    QHash<int, qreal> hoverValues_;
    QHash<int, qreal> revealValues_;
    QHash<int, QVariantAnimation *> hoverAnimations_;
    QHash<int, QVariantAnimation *> revealAnimations_;
    QPropertyAnimation *selectionAnimation_;
    bool closingTab_;
};
