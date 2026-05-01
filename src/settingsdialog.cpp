#include "settingsdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace {
constexpr auto kHomeUrlKey = "general/homeUrl";
constexpr auto kSearchUrlKey = "general/searchUrl";
constexpr auto kRestoreSessionKey = "general/restoreSession";
constexpr auto kDownloadDirKey = "downloads/directory";

QSettings appSettings()
{
    return QSettings(QStringLiteral("Morphine"), QStringLiteral("Morphine"));
}
} // namespace

QString SettingsDialog::defaultHomeUrl()
{
    return QStringLiteral("morphine://home");
}

QString SettingsDialog::defaultSearchUrl()
{
    return QStringLiteral("https://www.google.com/search?q=%1");
}

QString SettingsDialog::defaultDownloadDirectory()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    return path.isEmpty() ? QDir::homePath() : path;
}

QString SettingsDialog::homeUrl()
{
    auto settings = appSettings();
    const QString stored = settings.value(kHomeUrlKey, defaultHomeUrl()).toString().trimmed();
    return stored.isEmpty() ? defaultHomeUrl() : stored;
}

QString SettingsDialog::searchUrl()
{
    auto settings = appSettings();
    const QString stored = settings.value(kSearchUrlKey, defaultSearchUrl()).toString().trimmed();
    return stored.isEmpty() ? defaultSearchUrl() : stored;
}

QString SettingsDialog::downloadDirectory()
{
    auto settings = appSettings();
    const QString stored = settings.value(kDownloadDirKey, defaultDownloadDirectory()).toString().trimmed();
    return stored.isEmpty() ? defaultDownloadDirectory() : stored;
}

bool SettingsDialog::restoreSessionEnabled()
{
    auto settings = appSettings();
    return settings.value(kRestoreSessionKey, true).toBool();
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
      homeUrlEdit_(new QLineEdit(this)),
      searchUrlEdit_(new QLineEdit(this)),
      downloadDirEdit_(new QLineEdit(this)),
      browseButton_(new QPushButton(QStringLiteral("Browse…"), this)),
      restoreSessionCheck_(new QCheckBox(QStringLiteral("Restore tabs from last session on launch"), this))
{
    setWindowTitle(QStringLiteral("Morphine — Settings"));
    setModal(true);
    resize(560, 0);

    auto *generalGroup = new QGroupBox(QStringLiteral("General"), this);
    auto *generalForm = new QFormLayout(generalGroup);
    generalForm->setContentsMargins(12, 12, 12, 12);
    generalForm->setSpacing(8);

    homeUrlEdit_->setPlaceholderText(defaultHomeUrl());
    searchUrlEdit_->setPlaceholderText(defaultSearchUrl());
    searchUrlEdit_->setToolTip(QStringLiteral(
        "Search URL template. Use %1 as the placeholder for the query, e.g.\n"
        "https://www.google.com/search?q=%1\n"
        "https://duckduckgo.com/?q=%1"));

    generalForm->addRow(QStringLiteral("Home page URL:"), homeUrlEdit_);
    generalForm->addRow(QStringLiteral("Search URL template:"), searchUrlEdit_);
    generalForm->addRow(QString(), restoreSessionCheck_);

    auto *downloadsGroup = new QGroupBox(QStringLiteral("Downloads"), this);
    auto *downloadsLayout = new QVBoxLayout(downloadsGroup);
    downloadsLayout->setContentsMargins(12, 12, 12, 12);
    auto *downloadRow = new QHBoxLayout;
    downloadRow->addWidget(new QLabel(QStringLiteral("Save files to:"), downloadsGroup));
    downloadRow->addWidget(downloadDirEdit_, 1);
    downloadRow->addWidget(browseButton_);
    downloadsLayout->addLayout(downloadRow);

    auto *privacyGroup = new QGroupBox(QStringLiteral("Privacy"), this);
    auto *privacyLayout = new QHBoxLayout(privacyGroup);
    privacyLayout->setContentsMargins(12, 12, 12, 12);
    auto *clearHistory = new QPushButton(QStringLiteral("Clear history"), privacyGroup);
    auto *clearBookmarks = new QPushButton(QStringLiteral("Clear bookmarks"), privacyGroup);
    auto *clearBrowsing = new QPushButton(QStringLiteral("Clear cookies && cache"), privacyGroup);
    privacyLayout->addWidget(clearHistory);
    privacyLayout->addWidget(clearBookmarks);
    privacyLayout->addWidget(clearBrowsing);
    privacyLayout->addStretch(1);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults,
        this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(generalGroup);
    layout->addWidget(downloadsGroup);
    layout->addWidget(privacyGroup);
    layout->addStretch(1);
    layout->addWidget(buttons);

    connect(browseButton_, &QPushButton::clicked, this, &SettingsDialog::browseDownloadDirectory);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &SettingsDialog::resetDefaults);

    connect(clearHistory, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, QStringLiteral("Clear history"),
                                  QStringLiteral("Remove all browsing history?"))
            == QMessageBox::Yes) {
            emit clearHistoryRequested();
        }
    });
    connect(clearBookmarks, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, QStringLiteral("Clear bookmarks"),
                                  QStringLiteral("Remove all bookmarks?"))
            == QMessageBox::Yes) {
            emit clearBookmarksRequested();
        }
    });
    connect(clearBrowsing, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, QStringLiteral("Clear cookies & cache"),
                                  QStringLiteral("Remove cookies, cache and site storage for the current profile?"))
            == QMessageBox::Yes) {
            emit clearBrowsingDataRequested();
        }
    });

    loadFromSettings();
}

void SettingsDialog::browseDownloadDirectory()
{
    const QString current = downloadDirEdit_->text().isEmpty()
        ? defaultDownloadDirectory()
        : downloadDirEdit_->text();
    const QString chosen = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Choose download folder"), current);
    if (!chosen.isEmpty()) {
        downloadDirEdit_->setText(chosen);
    }
}

void SettingsDialog::resetDefaults()
{
    homeUrlEdit_->setText(defaultHomeUrl());
    searchUrlEdit_->setText(defaultSearchUrl());
    downloadDirEdit_->setText(defaultDownloadDirectory());
    restoreSessionCheck_->setChecked(true);
}

void SettingsDialog::loadFromSettings()
{
    homeUrlEdit_->setText(homeUrl());
    searchUrlEdit_->setText(searchUrl());
    downloadDirEdit_->setText(downloadDirectory());
    restoreSessionCheck_->setChecked(restoreSessionEnabled());
}

void SettingsDialog::saveToSettings()
{
    auto settings = appSettings();
    const QString home = homeUrlEdit_->text().trimmed();
    const QString search = searchUrlEdit_->text().trimmed();
    const QString downloads = downloadDirEdit_->text().trimmed();

    settings.setValue(kHomeUrlKey, home.isEmpty() ? defaultHomeUrl() : home);
    settings.setValue(kSearchUrlKey, search.isEmpty() ? defaultSearchUrl() : search);
    settings.setValue(kDownloadDirKey, downloads.isEmpty() ? defaultDownloadDirectory() : downloads);
    settings.setValue(kRestoreSessionKey, restoreSessionCheck_->isChecked());
}

void SettingsDialog::accept()
{
    const QString search = searchUrlEdit_->text().trimmed();
    if (!search.isEmpty() && !search.contains(QStringLiteral("%1"))) {
        QMessageBox::warning(this, QStringLiteral("Invalid search URL"),
                             QStringLiteral("Search URL template must contain %1 as the query placeholder."));
        searchUrlEdit_->setFocus();
        searchUrlEdit_->selectAll();
        return;
    }

    saveToSettings();
    emit settingsChanged();
    QDialog::accept();
}
