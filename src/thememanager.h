#pragma once

#include <QObject>

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool light READ isLight WRITE setLight NOTIFY lightChanged)

public:
    static ThemeManager *instance();

    bool isLight() const { return light_; }

public slots:
    void setLight(bool light);
    void toggle();

signals:
    void lightChanged(bool light);

private:
    explicit ThemeManager(QObject *parent = nullptr);

    bool light_;
};
