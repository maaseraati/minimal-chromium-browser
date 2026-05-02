#pragma once

#include <QHash>
#include <QPointer>
#include <QRectF>
#include <QTabBar>
#include <QTabWidget>
#include <QVariantAnimation>
#include <QVector>

class QPropertyAnimation;

// Custom tab bar that paints rounded "pill" tabs in the prototype style and
// animates the active-tab pill via QPropertyAnimation on its QRectF property
// (mirrors the HTML prototype's `.active-pill` element). New tabs fade /
// slide in on insert and fade / slide out before being removed on close.
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
    // Emitted *after* the close animation finishes so the embedder can
    // actually remove the underlying widget.
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
    void onTabMoved(int from, int to);

private:
    QRectF pillRectForIndex(int index) const;
    QRect closeIconRect(int tabIndex) const;
    QRect faviconRect(int tabIndex) const;
    int closeButtonAt(const QPoint &pos) const;
    void animatePillTo(const QRectF &target);
    void updateHover(const QPoint &pos);

    qreal appearProgressFor(int index) const;
    qreal closeProgressFor(int index) const;
    bool isClosing(int index) const;
    void startAppearAnim(quint64 id);
    void startCloseAnim(int index);

    QPointer<QPropertyAnimation> pillAnim_;
    QRectF pillGeometry_;
    int hoverTabIndex_ = -1;
    int hoverCloseIndex_ = -1;
    int pressedCloseIndex_ = -1;
    bool firstLayout_ = true;

    // Per-tab animation state keyed by stable ID (positions shift as tabs are
    // moved/inserted/removed, so we can't key by index).
    QVector<quint64> tabIds_;
    QHash<quint64, qreal> appearProgress_;       // 0..1, 1 = fully visible
    QHash<quint64, qreal> closeProgress_;        // 0..1, 1 = fully gone
    QHash<quint64, QVariantAnimation *> appearAnims_;
    QHash<quint64, QVariantAnimation *> closeAnims_;
    quint64 nextTabId_ = 1;
};

// Tiny QTabWidget subclass that injects ChromeTabBar (QTabWidget::setTabBar is
// protected, so we expose it via inheritance instead of friending).
class ChromeTabWidget final : public QTabWidget {
    Q_OBJECT

public:
    explicit ChromeTabWidget(QWidget *parent = nullptr);

    ChromeTabBar *chromeBar() const;
};
