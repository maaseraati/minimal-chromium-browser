#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QPushButton;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    static QString defaultHomeUrl();
    static QString defaultSearchUrl();
    static QString defaultDownloadDirectory();

    static QString homeUrl();
    static QString searchUrl();
    static QString downloadDirectory();
    static bool restoreSessionEnabled();

signals:
    void clearHistoryRequested();
    void clearBookmarksRequested();
    void clearBrowsingDataRequested();
    void settingsChanged();

private slots:
    void browseDownloadDirectory();
    void resetDefaults();

private:
    void loadFromSettings();
    void saveToSettings();
    void accept() override;

    QLineEdit *homeUrlEdit_;
    QLineEdit *searchUrlEdit_;
    QLineEdit *downloadDirEdit_;
    QPushButton *browseButton_;
    QCheckBox *restoreSessionCheck_;
};
