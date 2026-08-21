#include "preferences_dialog.h"
#include "utils.h"

namespace {
const char kRcloneBrowserLatestReleaseApi[] =
    "https://api.github.com/repos/xrp001/RcloneBrowser/releases/latest";
const char kRcloneBrowserLatestReleasePage[] =
    "https://github.com/xrp001/RcloneBrowser/releases/latest";
const char kRcloneLatestReleaseApi[] =
    "https://api.github.com/repos/rclone/rclone/releases/latest";
const char kRcloneDownloadsPage[] = "https://rclone.org/downloads/";
} // namespace

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  QObject::connect(ui.rcloneBrowse, &QPushButton::clicked, this, [=]() {
    QString rclone = QFileDialog::getOpenFileName(
        this, tr("Select rclone executable"), ui.rclone->text());
    if (rclone.isEmpty()) {
      return;
    }

    if (!QFileInfo(rclone).isExecutable()) {
      QMessageBox::critical(this, tr("Error"),
                            tr("File %1 is not executable").arg(rclone));
      return;
    }

    if (QFileInfo(rclone) == QFileInfo(qApp->applicationFilePath())) {
      QMessageBox::critical(
          this, tr("Error"),
          tr("You selected the Rclone Browser executable!\nPlease select the "
             "rclone executable instead."));
      return;
    }

    ui.rclone->setText(rclone);
  });

  QObject::connect(ui.rcloneConfBrowse, &QPushButton::clicked, this, [=]() {
    QString rcloneConf = QFileDialog::getOpenFileName(
        this, tr("Select .rclone.conf location"), ui.rcloneConf->text());
    if (rcloneConf.isEmpty()) {
      return;
    }

    ui.rcloneConf->setText(rcloneConf);
  });

  QObject::connect(
      ui.defaultDownloadDirBrowse, &QPushButton::clicked, this, [=]() {
        QString defaultDownloadDir = QFileDialog::getExistingDirectory(
            this, tr("Select default download directory"),
            ui.defaultDownloadDir->text());

        if (defaultDownloadDir.isEmpty()) {
          return;
        }

        ui.defaultDownloadDir->setText(defaultDownloadDir);
      });

  QObject::connect(
      ui.defaultUploadDirBrowse, &QPushButton::clicked, this, [=]() {
        QString defaultUploadDir = QFileDialog::getExistingDirectory(
            this, tr("Select default upload directory"),
            ui.defaultUploadDir->text());

        if (defaultUploadDir.isEmpty()) {
          return;
        }

        ui.defaultUploadDir->setText(defaultUploadDir);
      });

  auto settings = GetSettings();
  ui.rclone->setText(
      QDir::toNativeSeparators(settings->value("Settings/rclone").toString()));
  ui.rcloneConf->setText(QDir::toNativeSeparators(
      settings->value("Settings/rcloneConf").toString()));
  ui.stream->setText(settings->value("Settings/stream").toString());

#if defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
  ui.mount->setText(
      settings
          ->value("Settings/mount",
                  "* mount is not supported by rclone on this system *")
          .toString());
  ui.mount->setDisabled(true);
#else
  ui.mount->setText(
      settings->value("Settings/mount", "--vfs-cache-mode writes").toString());
#endif

  ui.defaultDownloadDir->setText(QDir::toNativeSeparators(
      settings->value("Settings/defaultDownloadDir").toString()));
  ui.defaultUploadDir->setText(QDir::toNativeSeparators(
      settings->value("Settings/defaultUploadDir").toString()));
  ui.defaultDownloadOptions->setText(
      settings->value("Settings/defaultDownloadOptions").toString());
  ui.defaultUploadOptions->setText(
      settings->value("Settings/defaultUploadOptions").toString());
  ui.defaultRcloneOptions->setText(
      settings->value("Settings/defaultRcloneOptions").toString());

  ui.checkRcloneBrowserUpdates->setChecked(
      settings->value("Settings/checkRcloneBrowserUpdates", true).toBool());
  ui.checkRcloneUpdates->setChecked(
      settings->value("Settings/checkRcloneUpdates", true).toBool());

  mUpdateTimeout.setSingleShot(true);
  QObject::connect(ui.checkRcloneBrowserUpdatesNow, &QPushButton::clicked, this,
                   &PreferencesDialog::checkRcloneBrowserUpdates);
  QObject::connect(&mUpdateTimeout, &QTimer::timeout, this, [=]() {
    if (!mUpdateReply) {
      return;
    }
    QNetworkReply *reply = mUpdateReply;
    mUpdateReply = nullptr;
    reply->abort();
    reply->deleteLater();
    ui.checkRcloneBrowserUpdatesNow->setEnabled(true);
    setRcloneBrowserUpdateStatus(tr("Update check timed out after 1 minute."));
  });

  mRcloneUpdateTimeout.setSingleShot(true);
  QObject::connect(ui.checkRcloneUpdatesNow, &QPushButton::clicked, this,
                   &PreferencesDialog::checkRcloneUpdates);
  QObject::connect(&mRcloneUpdateTimeout, &QTimer::timeout, this, [=]() {
    if (!mRcloneUpdateReply) {
      return;
    }
    QNetworkReply *reply = mRcloneUpdateReply;
    mRcloneUpdateReply = nullptr;
    reply->abort();
    reply->deleteLater();
    ui.checkRcloneUpdatesNow->setEnabled(true);
    setRcloneUpdateStatus(tr("Update check timed out after 1 minute."));
  });

  if (settings->contains("Settings/latestRcloneBrowserVersion")) {
    const QString latest =
        settings->value("Settings/latestRcloneBrowserVersion").toString();
    const QString checkedAt =
        settings->value("Settings/latestRcloneBrowserVersionCheckedAt").toString();
    if (compareVersion(latest.toStdString(), RCLONE_BROWSER_VERSION) == 1) {
      setRcloneBrowserUpdateStatus(
          tr("New release: v%1 — <a href=\"%2\">Download</a> (checked %3)")
              .arg(latest, kRcloneBrowserLatestReleasePage, checkedAt));
    } else {
      setRcloneBrowserUpdateStatus(
          tr("Latest release: v%1 (checked %2)").arg(latest, checkedAt));
    }
  }

  if (settings->contains("Settings/latestRcloneVersion")) {
    const QString latest =
        settings->value("Settings/latestRcloneVersion").toString();
    const QString current = settings->value("Settings/rcloneVersion").toString();
    const QString checkedAt =
        settings->value("Settings/latestRcloneVersionCheckedAt").toString();
    if (!current.isEmpty() &&
        compareVersion(latest.toStdString(), current.toStdString()) == 1) {
      setRcloneUpdateStatus(
          tr("New rclone release: v%1 — <a href=\"%2\">Download</a> (checked %3)")
              .arg(latest, kRcloneDownloadsPage, checkedAt));
    } else {
      setRcloneUpdateStatus(
          tr("Latest rclone release: v%1 (checked %2)").arg(latest, checkedAt));
    }
  }

  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    ui.alwaysShowInTray->setChecked(
        settings->value("Settings/alwaysShowInTray", false).toBool());
    ui.closeToTray->setChecked(
        settings->value("Settings/closeToTray", false).toBool());
    ui.notifyFinishedTransfers->setChecked(
        settings->value("Settings/notifyFinishedTransfers", true).toBool());
  } else {
    ui.alwaysShowInTray->setChecked(false);
    ui.alwaysShowInTray->setDisabled(true);
    ui.closeToTray->setChecked(false);
    ui.closeToTray->setDisabled(true);
    ui.notifyFinishedTransfers->setChecked(false);
    ui.notifyFinishedTransfers->setDisabled(true);
  }

  ui.showFolderIcons->setChecked(
      settings->value("Settings/showFolderIcons", true).toBool());
  ui.showFileIcons->setChecked(
      settings->value("Settings/showFileIcons", true).toBool());
  ui.rowColors->setChecked(
      settings->value("Settings/rowColors", true).toBool());
  ui.showHidden->setChecked(
      settings->value("Settings/showHidden", true).toBool());

  QString theme = settings->value("Settings/theme").toString();
  if (theme.isEmpty()) {
    theme = settings->value("Settings/darkMode", false).toBool()
                ? "graphite"
                : "glacier";
  }
  const QStringList themes = {"glacier", "harbor", "sand", "graphite",
                              "midnight"};
  int themeIndex = themes.indexOf(theme);
  ui.theme->setCurrentIndex(themeIndex < 0 ? 0 : themeIndex);

#if defined(Q_OS_MACOS)
  QString sysInfo = QSysInfo::productVersion();
  if (sysInfo != "10.9" && sysInfo != "10.10" && sysInfo != "10.11" &&
      sysInfo != "10.12" && sysInfo != "10.13") {
    ui.labelTheme->hide();
    ui.theme->hide();
    ui.themeInfo->hide();
  }
#endif

  const QString language = settings->value("Settings/language", "en").toString();
  ui.language->setCurrentIndex(language == "zh_CN" ? 1 : 0);

  if ((settings->value("Settings/iconSize").toString()) == "small") {
    ui.cb_small->setChecked(true);
  } else {
    if (settings->value("Settings/iconSize").toString() == "large") {
      ui.cb_large->setChecked(true);
    } else {
      ui.cb_medium->setChecked(true);
    }
  }

  ui.info_2->setText(
      tr("See rclone <a "
         "href=\"https://github.com/rclone/rclone/blob/master/docs/content/"
         "faq.md#can-i-use-rclone-with-an-http-proxy\">FAQ</a> for details."));
  ui.info_2->setTextFormat(Qt::RichText);
  ui.info_2->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui.info_2->setOpenExternalLinks(true);

  if (settings->value("Settings/useProxy").toBool()) {
    ui.useProxy->setChecked(true);
  } else {
    ui.useSystemSettings->setChecked(true);
  }
  ui.http_proxy->setText(settings->value("Settings/http_proxy").toString());
  ui.https_proxy->setText(settings->value("Settings/https_proxy").toString());
  ui.no_proxy->setText(settings->value("Settings/no_proxy").toString());
}

PreferencesDialog::~PreferencesDialog() {}

void PreferencesDialog::checkRcloneBrowserUpdates() {
  if (mUpdateReply) {
    return;
  }

  ui.checkRcloneBrowserUpdatesNow->setEnabled(false);
  setRcloneBrowserUpdateStatus(tr("Checking for updates..."));

  QNetworkRequest request{QUrl(kRcloneBrowserLatestReleaseApi)};
  request.setRawHeader("User-Agent", "RcloneBrowser");
  QNetworkReply *reply = mUpdateManager.get(request);
  mUpdateReply = reply;
  mUpdateTimeout.start(60000);

  QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
    if (mUpdateReply != reply) {
      return;
    }

    mUpdateTimeout.stop();
    mUpdateReply = nullptr;
    ui.checkRcloneBrowserUpdatesNow->setEnabled(true);

    if (reply->error() != QNetworkReply::NoError) {
      setRcloneBrowserUpdateStatus(
          tr("Update check failed: %1").arg(reply->errorString()));
      reply->deleteLater();
      return;
    }

    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
      setRcloneBrowserUpdateStatus(
          tr("Update check failed: HTTP %1").arg(status));
      reply->deleteLater();
      return;
    }

    QJsonParseError jsonError;
    const QJsonDocument document =
        QJsonDocument::fromJson(reply->readAll(), &jsonError);
    reply->deleteLater();

    if (jsonError.error != QJsonParseError::NoError ||
        !document.isObject()) {
      setRcloneBrowserUpdateStatus(tr("Update check returned invalid data."));
      return;
    }

    QString latest = document.object().value("tag_name").toString().trimmed();
    if (latest.startsWith('v', Qt::CaseInsensitive)) {
      latest.remove(0, 1);
    }

    QRegExp versionPattern("^[0-9]+(\\.[0-9]+)*$");
    if (!versionPattern.exactMatch(latest)) {
      setRcloneBrowserUpdateStatus(tr("Update check returned invalid data."));
      return;
    }

    const QString checkedAt =
        QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate);
    auto settings = GetSettings();
    settings->setValue("Settings/latestRcloneBrowserVersion", latest);
    settings->setValue("Settings/latestRcloneBrowserVersionCheckedAt", checkedAt);

    const unsigned int result =
        compareVersion(latest.toStdString(), RCLONE_BROWSER_VERSION);
    if (result == 1) {
      setRcloneBrowserUpdateStatus(
          tr("New release: v%1 — <a href=\"%2\">Download</a> (checked %3)")
              .arg(latest, kRcloneBrowserLatestReleasePage, checkedAt));
    } else {
      setRcloneBrowserUpdateStatus(
          tr("Latest release: v%1 (checked %2)").arg(latest, checkedAt));
    }
  });
}

void PreferencesDialog::setRcloneBrowserUpdateStatus(const QString &text) {
  ui.rcloneBrowserUpdateStatus->setText(text);
}

void PreferencesDialog::checkRcloneUpdates() {
  if (mRcloneUpdateReply) {
    return;
  }

  ui.checkRcloneUpdatesNow->setEnabled(false);
  setRcloneUpdateStatus(tr("Checking for rclone updates..."));

  QNetworkRequest request{QUrl(kRcloneLatestReleaseApi)};
  request.setRawHeader("User-Agent", "RcloneBrowser");
  QNetworkReply *reply = mRcloneUpdateManager.get(request);
  mRcloneUpdateReply = reply;
  mRcloneUpdateTimeout.start(60000);

  QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
    if (mRcloneUpdateReply != reply) {
      return;
    }

    mRcloneUpdateTimeout.stop();
    mRcloneUpdateReply = nullptr;
    ui.checkRcloneUpdatesNow->setEnabled(true);

    if (reply->error() != QNetworkReply::NoError) {
      setRcloneUpdateStatus(
          tr("rclone update check failed: %1").arg(reply->errorString()));
      reply->deleteLater();
      return;
    }

    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
      setRcloneUpdateStatus(
          tr("rclone update check failed: HTTP %1").arg(status));
      reply->deleteLater();
      return;
    }

    QJsonParseError jsonError;
    const QJsonDocument document =
        QJsonDocument::fromJson(reply->readAll(), &jsonError);
    reply->deleteLater();
    if (jsonError.error != QJsonParseError::NoError ||
        !document.isObject()) {
      setRcloneUpdateStatus(
          tr("rclone update check returned invalid data."));
      return;
    }

    QString latest = document.object().value("tag_name").toString().trimmed();
    if (latest.startsWith('v', Qt::CaseInsensitive)) {
      latest.remove(0, 1);
    }
    latest.replace("-DEV", "");

    QRegExp versionPattern("^[0-9]+(\\.[0-9]+)*$");
    if (!versionPattern.exactMatch(latest)) {
      setRcloneUpdateStatus(
          tr("rclone update check returned invalid data."));
      return;
    }

    const QString checkedAt =
        QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate);
    auto settings = GetSettings();
    settings->setValue("Settings/latestRcloneVersion", latest);
    settings->setValue("Settings/latestRcloneVersionCheckedAt", checkedAt);

    const QString current = settings->value("Settings/rcloneVersion").toString();
    if (!current.isEmpty() &&
        compareVersion(latest.toStdString(), current.toStdString()) == 1) {
      setRcloneUpdateStatus(
          tr("New rclone release: v%1 — <a href=\"%2\">Download</a> (checked %3)")
              .arg(latest, kRcloneDownloadsPage, checkedAt));
    } else if (!current.isEmpty()) {
      setRcloneUpdateStatus(
          tr("Latest rclone release: v%1; installed: v%2 (checked %3)")
              .arg(latest, current, checkedAt));
    } else {
      setRcloneUpdateStatus(
          tr("Latest rclone release: v%1 (checked %2)").arg(latest, checkedAt));
    }
  });
}

void PreferencesDialog::setRcloneUpdateStatus(const QString &text) {
  ui.rcloneUpdateStatus->setText(text);
}

QString PreferencesDialog::getRclone() const {
  return QDir::fromNativeSeparators(ui.rclone->text());
}

QString PreferencesDialog::getRcloneConf() const {
  return QDir::fromNativeSeparators(ui.rcloneConf->text());
}

QString PreferencesDialog::getStream() const { return ui.stream->text(); }

QString PreferencesDialog::getMount() const { return ui.mount->text(); }

QString PreferencesDialog::getDefaultDownloadDir() const {
  return QDir::fromNativeSeparators(ui.defaultDownloadDir->text());
}

QString PreferencesDialog::getDefaultUploadDir() const {
  return QDir::fromNativeSeparators(ui.defaultUploadDir->text());
}

QString PreferencesDialog::getDefaultDownloadOptions() const {
  return ui.defaultDownloadOptions->text();
}

QString PreferencesDialog::getDefaultUploadOptions() const {
  return ui.defaultUploadOptions->text();
}

QString PreferencesDialog::getDefaultRcloneOptions() const {
  return ui.defaultRcloneOptions->text();
}

bool PreferencesDialog::getCheckRcloneBrowserUpdates() const {
  return ui.checkRcloneBrowserUpdates->isChecked();
}

bool PreferencesDialog::getCheckRcloneUpdates() const {
  return ui.checkRcloneUpdates->isChecked();
}

bool PreferencesDialog::getAlwaysShowInTray() const {
  return ui.alwaysShowInTray->isChecked();
}

bool PreferencesDialog::getCloseToTray() const {
  return ui.closeToTray->isChecked();
}

bool PreferencesDialog::getNotifyFinishedTransfers() const {
  return ui.notifyFinishedTransfers->isChecked();
}

bool PreferencesDialog::getShowFolderIcons() const {
  return ui.showFolderIcons->isChecked();
}

bool PreferencesDialog::getShowFileIcons() const {
  return ui.showFileIcons->isChecked();
}

bool PreferencesDialog::getRowColors() const {
  return ui.rowColors->isChecked();
}

bool PreferencesDialog::getShowHidden() const {
  return ui.showHidden->isChecked();
}

QString PreferencesDialog::getTheme() const {
  const QStringList themes = {"glacier", "harbor", "sand", "graphite",
                              "midnight"};
  return themes.value(ui.theme->currentIndex(), "glacier");
}

QString PreferencesDialog::getIconSize() const {
  if (ui.cb_small->isChecked()) {
    return "small";
  } else {
    if (ui.cb_large->isChecked()) {
      return "large";
    } else {
      return "medium";
    }
  }
}

QString PreferencesDialog::getLanguage() const {
  return ui.language->currentIndex() == 1 ? "zh_CN" : "en";
}

QString PreferencesDialog::getHttpProxy() const {
  return ui.http_proxy->text();
}

QString PreferencesDialog::getHttpsProxy() const {
  return ui.https_proxy->text();
}

QString PreferencesDialog::getNoProxy() const { return ui.no_proxy->text(); }

bool PreferencesDialog::getUseProxy() const {
  if (ui.useSystemSettings->isChecked()) {
    return false;
  } else {
    return true;
  }
}
