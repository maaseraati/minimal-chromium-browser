#pragma once

#include <QHash>
#include <QPointer>
#include <QRectF>
#include <QTabBar>
#include <QTabWidget>

class QPropertyAnimation;
class QVariantAnimation;

// Custom tab bar that paints rounded "pill" tabs in the green prototype style
// and animates the active-tab pill via QPropertyAnimation on its QRectF
// geometry (mirrors the HTML prototype's `.active-pill` CSS element).
class ChromeTabBar final : public QTabBar {
    Q_OBJECT
    Q_PROPERTY(QRectF pillGeometry READ pillGeometry WRITE setPillGeometry)

public:
    explicit ChromeTabBar(QWidget *parent = nullptr);

    QRectF pillGeometry() const { return pillGeometry_; }
    void setPillGeometry(const QRectF &geometry);

    // Force the pill to jump to the current tab without animation. Useful on
    // first show / after layout changes so we don't animate from (0,0).
    void snapPillToCurrent();

signals:
    void tabCloseClicked(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;
    void tabLayoutChange() override;
    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;

private slots:
    void onCurrentChanged(int index);

private:
    QRectF pillRectForIndex(int index) const;
    QRect closeIconRect(int tabIndex) const;
    QRect faviconRect(int tabIndex) const;
    int closeButtonAt(const QPoint &pos) const;
    void animatePillTo(const QRectF &target);
    void updateHover(const QPoint &pos);

    QPointer<QPropertyAnimation> pillAnim_;
    QRectF pillGeometry_;
    int hoverTabIndex_ = -1;
    int hoverCloseIndex_ = -1;
    int pressedCloseIndex_ = -1;
    bool firstLayout_ = true;
};

// Tiny QTabWidget subclass that injects ChromeTabBar (QTabWidget::setTabBar is
// protected, so we expose it via inheritance instead of friending).
class ChromeTabWidget final : public QTabWidget {
    Q_OBJECT

public:
    explicit ChromeTabWidget(QWidget *parent = nullptr);

    ChromeTabBar *chromeBar() const;
};

