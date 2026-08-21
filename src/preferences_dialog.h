#pragma once

#include "pch.h"
#include "ui_preferences_dialog.h"

class PreferencesDialog : public QDialog {
  Q_OBJECT

public:
  PreferencesDialog(QWidget *parent = nullptr);
  ~PreferencesDialog();

  QString getRclone() const;
  QString getRcloneConf() const;
  QString getStream() const;
  QString getMount() const;
  QString getDefaultDownloadDir() const;
  QString getDefaultUploadDir() const;
  QString getDefaultDownloadOptions() const;
  QString getDefaultUploadOptions() const;
  QString getDefaultRcloneOptions() const;

  bool getCheckRcloneBrowserUpdates() const;
  bool getCheckRcloneUpdates() const;

  bool getAlwaysShowInTray() const;
  bool getCloseToTray() const;
  bool getNotifyFinishedTransfers() const;

  bool getShowFolderIcons() const;
  bool getShowFileIcons() const;
  bool getRowColors() const;
  bool getShowHidden() const;
  QString getTheme() const;
  QString getIconSize() const;
  QString getLanguage() const;

  bool getUseProxy() const;
  QString getHttpProxy() const;
  QString getHttpsProxy() const;
  QString getNoProxy() const;

private:
  void checkRcloneBrowserUpdates();
  void setRcloneBrowserUpdateStatus(const QString &text);
  void checkRcloneUpdates();
  void setRcloneUpdateStatus(const QString &text);

  Ui::PreferencesDialog ui;
  QNetworkAccessManager mUpdateManager;
  QPointer<QNetworkReply> mUpdateReply;
  QTimer mUpdateTimeout;
  QNetworkAccessManager mRcloneUpdateManager;
  QPointer<QNetworkReply> mRcloneUpdateReply;
  QTimer mRcloneUpdateTimeout;
};
