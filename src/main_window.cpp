#include "main_window.h"
#include "job_options.h"
#include "job_widget.h"
#include "list_of_job_options.h"
#include "mount_widget.h"
#include "preferences_dialog.h"
#include "remote_widget.h"
#include "stream_widget.h"
#include "transfer_dialog.h"
#include "utils.h"
#ifdef Q_OS_MACOS
#include "osx_helper.h"
#endif

namespace {
void ApplyTheme(const QString &theme) {
  qApp->setStyle(QStyleFactory::create("Fusion"));

  QPalette palette;
  if (theme == "harbor") {
    palette.setColor(QPalette::Window, QColor(232, 240, 244));
    palette.setColor(QPalette::WindowText, QColor(24, 48, 61));
    palette.setColor(QPalette::Base, QColor(248, 252, 253));
    palette.setColor(QPalette::AlternateBase, QColor(224, 237, 241));
    palette.setColor(QPalette::Text, QColor(24, 48, 61));
    palette.setColor(QPalette::Button, QColor(238, 246, 248));
    palette.setColor(QPalette::ButtonText, QColor(24, 48, 61));
    palette.setColor(QPalette::Highlight, QColor(25, 107, 123));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, QColor(19, 92, 111));
    palette.setColor(QPalette::Light, QColor(255, 255, 255));
    palette.setColor(QPalette::Midlight, QColor(211, 229, 234));
    palette.setColor(QPalette::Mid, QColor(157, 185, 194));
    palette.setColor(QPalette::Dark, QColor(71, 113, 126));
  } else if (theme == "sand") {
    palette.setColor(QPalette::Window, QColor(244, 240, 232));
    palette.setColor(QPalette::WindowText, QColor(55, 45, 36));
    palette.setColor(QPalette::Base, QColor(255, 253, 248));
    palette.setColor(QPalette::AlternateBase, QColor(239, 232, 219));
    palette.setColor(QPalette::Text, QColor(55, 45, 36));
    palette.setColor(QPalette::Button, QColor(249, 245, 237));
    palette.setColor(QPalette::ButtonText, QColor(55, 45, 36));
    palette.setColor(QPalette::Highlight, QColor(153, 82, 49));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, QColor(135, 67, 38));
    palette.setColor(QPalette::Light, QColor(255, 255, 255));
    palette.setColor(QPalette::Midlight, QColor(232, 222, 205));
    palette.setColor(QPalette::Mid, QColor(190, 171, 145));
    palette.setColor(QPalette::Dark, QColor(117, 91, 68));
  } else if (theme == "graphite") {
    palette.setColor(QPalette::Window, QColor(38, 42, 46));
    palette.setColor(QPalette::WindowText, QColor(238, 241, 243));
    palette.setColor(QPalette::Base, QColor(27, 30, 33));
    palette.setColor(QPalette::AlternateBase, QColor(45, 50, 54));
    palette.setColor(QPalette::Text, QColor(238, 241, 243));
    palette.setColor(QPalette::Button, QColor(49, 54, 59));
    palette.setColor(QPalette::ButtonText, QColor(238, 241, 243));
    palette.setColor(QPalette::Highlight, QColor(73, 146, 153));
    palette.setColor(QPalette::HighlightedText, QColor(10, 22, 24));
    palette.setColor(QPalette::Link, QColor(112, 191, 198));
    palette.setColor(QPalette::Light, QColor(77, 83, 89));
    palette.setColor(QPalette::Midlight, QColor(59, 65, 70));
    palette.setColor(QPalette::Mid, QColor(84, 91, 97));
    palette.setColor(QPalette::Dark, QColor(145, 154, 161));
  } else if (theme == "midnight") {
    palette.setColor(QPalette::Window, QColor(22, 31, 46));
    palette.setColor(QPalette::WindowText, QColor(232, 239, 248));
    palette.setColor(QPalette::Base, QColor(14, 21, 33));
    palette.setColor(QPalette::AlternateBase, QColor(27, 39, 57));
    palette.setColor(QPalette::Text, QColor(232, 239, 248));
    palette.setColor(QPalette::Button, QColor(31, 45, 64));
    palette.setColor(QPalette::ButtonText, QColor(232, 239, 248));
    palette.setColor(QPalette::Highlight, QColor(68, 137, 196));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, QColor(111, 181, 237));
    palette.setColor(QPalette::Light, QColor(58, 78, 103));
    palette.setColor(QPalette::Midlight, QColor(40, 57, 79));
    palette.setColor(QPalette::Mid, QColor(72, 91, 115));
    palette.setColor(QPalette::Dark, QColor(137, 157, 181));
  } else {
    palette.setColor(QPalette::Window, QColor(244, 246, 248));
    palette.setColor(QPalette::WindowText, QColor(35, 43, 52));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(248, 250, 252));
    palette.setColor(QPalette::Text, QColor(35, 43, 52));
    palette.setColor(QPalette::Button, QColor(248, 250, 252));
    palette.setColor(QPalette::ButtonText, QColor(35, 43, 52));
    palette.setColor(QPalette::Highlight, QColor(53, 126, 174));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, QColor(32, 105, 160));
    palette.setColor(QPalette::Light, QColor(255, 255, 255));
    palette.setColor(QPalette::Midlight, QColor(231, 237, 242));
    palette.setColor(QPalette::Mid, QColor(190, 202, 212));
    palette.setColor(QPalette::Dark, QColor(91, 119, 139));
  }

  palette.setColor(QPalette::ToolTipBase, palette.color(QPalette::WindowText));
  palette.setColor(QPalette::ToolTipText, palette.color(QPalette::Base));
  palette.setColor(QPalette::BrightText, QColor(218, 72, 72));
  palette.setColor(QPalette::Disabled, QPalette::Text,
                   palette.color(QPalette::Mid));
  palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                   palette.color(QPalette::Mid));
  qApp->setPalette(palette);

  qApp->setStyleSheet(R"(
    QMainWindow, QDialog { background-color: palette(window); }
    QMenuBar {
      background-color: palette(button);
      color: palette(button-text);
      border-bottom: 1px solid palette(mid);
      padding: 2px;
    }
    QMenuBar::item {
      color: palette(button-text);
      border-radius: 4px;
      padding: 5px 9px;
    }
    QMenuBar::item:selected {
      background-color: palette(midlight);
      color: palette(window-text);
    }
    QMenuBar::item:disabled { color: palette(dark); }
    QMenu {
      background-color: palette(base);
      color: palette(text);
      border: 1px solid palette(mid);
      padding: 4px;
    }
    QMenu::item {
      color: palette(text);
      border-radius: 4px;
      padding: 6px 28px 6px 24px;
    }
    QMenu::item:selected {
      background-color: palette(highlight);
      color: palette(highlighted-text);
    }
    QMenu::item:disabled { color: palette(dark); }
    QPushButton, QToolButton {
      background-color: palette(button);
      color: palette(button-text);
      border: 1px solid palette(mid);
      border-radius: 5px;
      padding: 4px 7px;
    }
    QPushButton:hover, QToolButton:hover {
      background-color: palette(midlight);
      border-color: palette(dark);
    }
    QPushButton:pressed, QToolButton:pressed,
    QPushButton:checked, QToolButton:checked {
      background-color: palette(highlight);
      color: palette(highlighted-text);
      border-color: palette(highlight);
    }
    QPushButton:focus, QToolButton:focus { border-color: palette(highlight); }
    QPushButton:disabled, QToolButton:disabled {
      background-color: palette(window);
      color: palette(mid);
      border-color: palette(midlight);
    }
    QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QSpinBox,
    QDoubleSpinBox, QDateEdit, QTimeEdit, QDateTimeEdit {
      background-color: palette(base);
      color: palette(text);
      border: 1px solid palette(mid);
      border-radius: 5px;
      padding: 4px 6px;
      selection-background-color: palette(highlight);
      selection-color: palette(highlighted-text);
    }
    QLineEdit:hover, QTextEdit:hover, QPlainTextEdit:hover,
    QComboBox:hover, QSpinBox:hover, QDoubleSpinBox:hover {
      border-color: palette(dark);
    }
    QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus,
    QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
      border-color: palette(highlight);
    }
    QComboBox::drop-down { border: 0; width: 22px; }
    QTreeView, QListView, QTableView {
      background-color: palette(base);
      alternate-background-color: palette(alternate-base);
      color: palette(text);
      border: 1px solid palette(mid);
      border-radius: 5px;
      outline: 0;
    }
    QTreeView::item, QListView::item, QTableView::item { padding: 3px; }
    QTreeView::item:hover, QListView::item:hover, QTableView::item:hover {
      background-color: palette(midlight);
    }
    QTreeView::item:selected, QListView::item:selected,
    QTableView::item:selected {
      background-color: palette(highlight);
      color: palette(highlighted-text);
    }
    QHeaderView::section {
      background-color: palette(midlight);
      color: palette(window-text);
      border: 0;
      border-right: 1px solid palette(mid);
      border-bottom: 1px solid palette(mid);
      padding: 5px 7px;
    }
    QTabWidget::pane {
      background-color: palette(base);
      border: 1px solid palette(mid);
      border-radius: 5px;
      top: -1px;
    }
    QTabBar::tab {
      background-color: palette(midlight);
      color: palette(window-text);
      border: 1px solid palette(mid);
      border-bottom: 0;
      border-top-left-radius: 5px;
      border-top-right-radius: 5px;
      padding: 6px 11px;
      margin-right: 2px;
    }
    QTabBar::tab:hover { background-color: palette(button); }
    QTabBar::tab:selected {
      background-color: palette(base);
      color: palette(link);
    }
    QGroupBox {
      border: 1px solid palette(mid);
      border-radius: 6px;
      margin-top: 8px;
      padding-top: 7px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 9px;
      padding: 0 4px;
      color: palette(window-text);
    }
    QCheckBox, QRadioButton { spacing: 6px; }
    QCheckBox::indicator, QRadioButton::indicator { width: 15px; height: 15px; }
    QProgressBar {
      background-color: palette(midlight);
      color: palette(text);
      border: 0;
      border-radius: 4px;
      text-align: center;
    }
    QProgressBar::chunk { background-color: palette(highlight); border-radius: 4px; }
    QStatusBar {
      background-color: palette(button);
      border-top: 1px solid palette(mid);
      color: palette(window-text);
    }
    QToolTip {
      background-color: palette(tooltip-base);
      color: palette(tooltip-text);
      border: 1px solid palette(dark);
      padding: 4px 6px;
    }
    QScrollBar:vertical { background: palette(window); width: 12px; margin: 0; }
    QScrollBar::handle:vertical {
      background: palette(mid);
      border-radius: 5px;
      min-height: 24px;
      margin: 2px;
    }
    QScrollBar::handle:vertical:hover { background: palette(dark); }
    QScrollBar:horizontal { background: palette(window); height: 12px; margin: 0; }
    QScrollBar::handle:horizontal {
      background: palette(mid);
      border-radius: 5px;
      min-width: 24px;
      margin: 2px;
    }
    QScrollBar::handle:horizontal:hover { background: palette(dark); }
    QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
  )");
}
} // namespace
MainWindow::MainWindow() {
  ui.setupUi(this);

  if (IsPortableMode()) {
    this->setWindowTitle("Rclone Browser - portable mode");
  } else {
    this->setWindowTitle("Rclone Browser");
  }

#if defined(Q_OS_WIN)
  // disable "?" WindowContextHelpButton
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

#if !defined(Q_OS_MACOS)
  auto settings = GetSettings();
  QString theme = settings->value("Settings/theme").toString();
  if (theme.isEmpty()) {
    theme = settings->value("Settings/darkMode", false).toBool()
                ? "graphite"
                : "glacier";
  }
  ApplyTheme(theme);
#else
  QString sysInfo = QSysInfo::productVersion();
  if (sysInfo == "10.9" || sysInfo == "10.10" || sysInfo == "10.11" ||
      sysInfo == "10.12" || sysInfo == "10.13") {
    auto settings = GetSettings();
    QString theme = settings->value("Settings/theme").toString();
    if (theme.isEmpty()) {
      theme = settings->value("Settings/darkMode", false).toBool()
                  ? "graphite"
                  : "glacier";
    }
    ApplyTheme(theme);
  }
#endif

  mSystemTray.setIcon(qApp->windowIcon());
  {
    auto settings = GetSettings();
    if (settings->contains("MainWindow/geometry")) {
      restoreGeometry(settings->value("MainWindow/geometry").toByteArray());
    }
    SetRclone(settings->value("Settings/rclone").toString());
    SetRcloneConf(settings->value("Settings/rcloneConf").toString());

    mAlwaysShowInTray =
        settings->value("Settings/alwaysShowInTray", false).toBool();
    mCloseToTray = settings->value("Settings/closeToTray", false).toBool();
    mNotifyFinishedTransfers =
        settings->value("Settings/notifyFinishedTransfers", true).toBool();

    mSystemTray.setVisible(mAlwaysShowInTray);

    // during first run the lastUsed keys might not exist
    if (!(settings->contains("Settings/lastUsedSourceFolder"))) {
      // if lastUsedSourceFolder does not exist create new empty key
      settings->setValue("Settings/lastUsedSourceFolder", "");
    };
    if (!(settings->contains("Settings/lastUsedDestFolder"))) {
      // if lastUsedDestFolder does not exist create new empty key
      settings->setValue("Settings/lastUsedDestFolder", "");
    };
    if (!(settings->contains("Settings/defaultDownloadOptions"))) {
      // if defaultDownloadOptions does not exist create new empty key
      settings->setValue("Settings/defaultDownloadOptions", "");
    };
#ifdef Q_OS_MACOS
    // for macOS by default exclude .DS_Store files from uploads
    if (!(settings->contains("Settings/defaultUploadOptions"))) {
      // if defaultDownloadOptions does not exist create new empty key
      settings->setValue("Settings/defaultUploadOptions",
                         "--exclude .DS_Store");
    };
#else
    if (!(settings->contains("Settings/defaultUploadOptions"))) {
      // if defaultDownloadOptions does not exist create new empty key
      settings->setValue("Settings/defaultUploadOptions", "");
    };
#endif
    if (!(settings->contains("Settings/defaultRcloneOptions"))) {
      // if defaultRcloneOptions does not exist create new empty key
      settings->setValue("Settings/defaultRcloneOptions", "--fast-list");
    };
  }

  QObject::connect(ui.preferences, &QAction::triggered, this, [=]() {
    PreferencesDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
      auto settings = GetSettings();
      settings->setValue("Settings/rclone", dialog.getRclone().trimmed());
      settings->setValue("Settings/rcloneConf",
                         dialog.getRcloneConf().trimmed());
      settings->setValue("Settings/stream", dialog.getStream());
      settings->setValue("Settings/mount", dialog.getMount());
      settings->setValue("Settings/defaultDownloadDir",
                         dialog.getDefaultDownloadDir().trimmed());
      settings->setValue("Settings/defaultUploadDir",
                         dialog.getDefaultUploadDir().trimmed());
      settings->setValue("Settings/defaultDownloadOptions",
                         dialog.getDefaultDownloadOptions().trimmed());
      settings->setValue("Settings/defaultUploadOptions",
                         dialog.getDefaultUploadOptions().trimmed());
      settings->setValue("Settings/defaultRcloneOptions",
                         dialog.getDefaultRcloneOptions().trimmed());

      settings->setValue("Settings/checkRcloneBrowserUpdates",
                         dialog.getCheckRcloneBrowserUpdates());
      settings->setValue("Settings/checkRcloneUpdates",
                         dialog.getCheckRcloneUpdates());

      settings->setValue("Settings/alwaysShowInTray",
                         dialog.getAlwaysShowInTray());
      settings->setValue("Settings/closeToTray", dialog.getCloseToTray());
      settings->setValue("Settings/notifyFinishedTransfers",
                         dialog.getNotifyFinishedTransfers());

      settings->setValue("Settings/showFolderIcons",
                         dialog.getShowFolderIcons());
      settings->setValue("Settings/showFileIcons", dialog.getShowFileIcons());
      settings->setValue("Settings/rowColors", dialog.getRowColors());
      settings->setValue("Settings/showHidden", dialog.getShowHidden());
      const QString theme = dialog.getTheme();
      const bool darkTheme = theme == "graphite" || theme == "midnight";
      settings->setValue("Settings/theme", theme);
      settings->setValue("Settings/darkMode", darkTheme);
      settings->setValue("Settings/darkModeIni", darkTheme);
      settings->setValue("Settings/iconSize", dialog.getIconSize().trimmed());

#if !defined(Q_OS_MACOS)
      ApplyTheme(theme);
#else
      const QString sysInfo = QSysInfo::productVersion();
      if (sysInfo == "10.9" || sysInfo == "10.10" || sysInfo == "10.11" ||
          sysInfo == "10.12" || sysInfo == "10.13") {
        ApplyTheme(theme);
      }
#endif

      const QString previousLanguage =
          settings->value("Settings/language", "en").toString();
      const QString currentLanguage = dialog.getLanguage();
      settings->setValue("Settings/language", currentLanguage);

      settings->setValue("Settings/useProxy", dialog.getUseProxy());
      settings->setValue("Settings/http_proxy",
                         dialog.getHttpProxy().trimmed());
      settings->setValue("Settings/https_proxy",
                         dialog.getHttpsProxy().trimmed());
      settings->setValue("Settings/no_proxy", dialog.getNoProxy().trimmed());

      SetRclone(dialog.getRclone());
      SetRcloneConf(dialog.getRcloneConf());
      mFirstTime = true;
      rcloneGetVersion();

      mAlwaysShowInTray = dialog.getAlwaysShowInTray();
      mCloseToTray = dialog.getCloseToTray();
      mNotifyFinishedTransfers = dialog.getNotifyFinishedTransfers();

      mSystemTray.setVisible(mAlwaysShowInTray);

      if (previousLanguage != currentLanguage) {
        QMessageBox::information(
            this, tr("Language Changed"),
            tr("The application language will be updated after restarting "
               "Rclone Browser."));
      }
    }
  });

  QObject::connect(ui.quit, &QAction::triggered, this, [=]() {
    mCloseToTray = false;
    close();
  });

  QObject::connect(ui.about, &QAction::triggered, this, [=]() {
    QMessageBox::about(
        this, "Rclone Browser",
        QString(
            R"(<h3>GUI for rclone, v)" RCLONE_BROWSER_VERSION "</h3>"
            R"(<p>Build time: )" RCLONE_BROWSER_BUILD_TIME "</p>"
            R"(<p>Copyright &copy; 2026</p>)"

            R"(<p>Current maintainer<br /><a href="https://github.com/xrp001/RcloneBrowser">xrp001</a></p>)"

            R"(<p>Current project page<br /><a href="https://github.com/xrp001/RcloneBrowser">https://github.com/xrp001/RcloneBrowser</a></p>)"

            R"(<p>Previous development and maintenance<br /><a href="https://github.com/kapitainsky/RcloneBrowser">kapitainsky</a></p>)"

            R"(<p>Original version<br /><a href="https://mmozeiko.github.io/RcloneBrowser">Martins Mozeiko</a></p>)"));
  });
  QObject::connect(ui.aboutQt, &QAction::triggered, qApp,
                   &QApplication::aboutQt);

  QObject::connect(
      ui.remotes, &QListWidget::currentItemChanged, this,
      [=](QListWidgetItem *current) { ui.open->setEnabled(current != NULL); });
  QObject::connect(ui.remotes, &QListWidget::itemActivated, ui.open,
                   &QPushButton::clicked);

  QObject::connect(ui.config, &QPushButton::clicked, this,
                   &MainWindow::rcloneConfig);
  QObject::connect(ui.refresh, &QPushButton::clicked, this,
                   &MainWindow::rcloneListRemotes);

  QObject::connect(ui.open, &QPushButton::clicked, this, [=]() {
    auto item = ui.remotes->selectedItems().front();
    QString type = item->data(Qt::UserRole).toString();
    QString name = item->text();
    bool isLocal = type == "local";
    bool isGoogle = type == "drive";

    auto remote =
        new RemoteWidget(&mIcons, name, isLocal, isGoogle, type, ui.tabs);
    QObject::connect(remote, &RemoteWidget::addMount, this,
                     &MainWindow::addMount);
    QObject::connect(remote, &RemoteWidget::addStream, this,
                     &MainWindow::addStream);
    QObject::connect(remote, &RemoteWidget::addTransfer, this,
                     &MainWindow::addTransfer);

    int index = ui.tabs->addTab(remote, name);
    ui.tabs->setCurrentIndex(index);
  });

  QObject::connect(ui.tabs, &QTabWidget::tabCloseRequested, ui.tabs,
                   &QTabWidget::removeTab);

  QObject::connect(ui.tasksListWidget, &QListWidget::currentItemChanged, this,
                   [=](QListWidgetItem *current) {
                     ui.buttonDeleteTask->setEnabled(current != nullptr);
                     ui.buttonEditTask->setEnabled(current != nullptr);
                     ui.buttonRunTask->setEnabled(current != nullptr);
                     ui.buttonDryrunTask->setEnabled(current != nullptr);
                   });

  QObject::connect(ui.buttonRunTask, &QPushButton::clicked, this, [=]() {
    JobOptionsListWidgetItem *item = static_cast<JobOptionsListWidgetItem *>(
        ui.tasksListWidget->currentItem());
    runItem(item);
  });
  QObject::connect(ui.buttonDryrunTask, &QPushButton::clicked, this, [=]() {
    JobOptionsListWidgetItem *item = static_cast<JobOptionsListWidgetItem *>(
        ui.tasksListWidget->currentItem());
    runItem(item, true);
  });

  //    QObject::connect(ui.tasksListWidget, &QListWidget::itemDoubleClicked,
  //    this, [=]()
  //    {
  //        editSelectedTask();
  //    });

  QObject::connect(ui.buttonEditTask, &QPushButton::clicked, this,
                   [=]() { editSelectedTask(); });

  QObject::connect(ui.buttonDeleteTask, &QPushButton::clicked, this, [=]() {
    JobOptionsListWidgetItem *item = static_cast<JobOptionsListWidgetItem *>(
        ui.tasksListWidget->currentItem());
    JobOptions *jo = item->GetData();
    ListOfJobOptions::getInstance()->Forget(jo);
  });

  QObject::connect(ListOfJobOptions::getInstance(),
                   &ListOfJobOptions::tasksListUpdated, this,
                   &MainWindow::listTasks);

  QStyle *style = QApplication::style();
  ui.buttonDeleteTask->setIcon(style->standardIcon(QStyle::SP_TrashIcon));
  ui.buttonEditTask->setIcon(style->standardIcon(QStyle::SP_FileIcon));
  ui.buttonRunTask->setIcon(style->standardIcon(QStyle::SP_CommandLink));
  mUploadIcon = style->standardIcon(QStyle::SP_ArrowUp);
  mDownloadIcon = style->standardIcon(QStyle::SP_ArrowDown);

  ui.tabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(1, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(1, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(2, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(2, QTabBar::LeftSide, nullptr);
  ui.tabs->setCurrentIndex(0);

  listTasks();

  QObject::connect(&mSystemTray, &QSystemTrayIcon::activated, this,
                   [=](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::DoubleClick ||
                         reason == QSystemTrayIcon::Trigger) {
                       showNormal();
                       mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
                       osxShowDockIcon();
#endif
                     }
                   });

  QObject::connect(&mSystemTray, &QSystemTrayIcon::messageClicked, this, [=]() {
    showNormal();
    mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
    osxShowDockIcon();
#endif

    ui.tabs->setCurrentIndex(1);
    if (mLastFinished) {
      mLastFinished->showDetails();
      ui.jobsArea->ensureWidgetVisible(mLastFinished);
    }
  });

  QMenu *trayMenu = new QMenu(this);
  QObject::connect(
      trayMenu->addAction("&Show"), &QAction::triggered, this, [=]() {
        MainWindow::setWindowState((windowState() & ~Qt::WindowMinimized) |
                                   Qt::WindowActive);
        MainWindow::show();  // bring window to top on macOS
        MainWindow::raise(); // bring window from minimized state on macOS
        MainWindow::activateWindow(); // bring window to front/unminimize on
                                      // windows
        mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
        osxShowDockIcon();
#endif
      });
  QObject::connect(trayMenu->addAction("&Quit"), &QAction::triggered, this,
                   &QWidget::close);
  mSystemTray.setContextMenu(trayMenu);

  mStatusMessage = new QLabel();
  ui.statusBar->addWidget(mStatusMessage);
  ui.statusBar->setStyleSheet("QStatusBar::item { border: 0; }");

  QTimer::singleShot(0, ui.remotes, SLOT(setFocus()));

  QString rclone = GetRclone();
  if (rclone.isEmpty()) {
    rclone = QStandardPaths::findExecutable("rclone");
    if (rclone.isEmpty()) {
      QMessageBox::information(
          this, "Error",
          "Cannot check rclone version!\nPlease verify rclone location.");
      emit ui.preferences->trigger();
    } else {
      auto settings = GetSettings();
      settings->setValue("Settings/rclone", rclone);
      SetRclone(rclone);
    }
  } else {
    rcloneGetVersion();
  }
}

MainWindow::~MainWindow() {
  auto settings = GetSettings();
  settings->setValue("MainWindow/geometry", saveGeometry());
}

void MainWindow::rcloneGetVersion() {
  bool firstTime = mFirstTime;
  mFirstTime = false;

  QProcess *p = new QProcess();

  QObject::connect(
      p,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int code, QProcess::ExitStatus) {
        if (code == 0) {
          QString version = p->readAllStandardOutput().trimmed();

          // extract rclone version - numbers only
          QString rclone_info1 = version;
          QString rclone_version_no;
          int lineBreak = rclone_info1.indexOf('\n');
          if (lineBreak != -1) {
            rclone_info1.remove(lineBreak, rclone_info1.length() - lineBreak);
            rclone_version_no = rclone_info1;
            rclone_version_no.replace("rclone v", "");
            rclone_version_no.replace("-DEV", "");
          } else {
            // for very old rclone versions format was one line only
            rclone_version_no = rclone_info1.trimmed();
            rclone_version_no.replace("rclone v", "");
            rclone_version_no.replace("-DEV", "");
          }
          // save current version no in settings
          auto settings = GetSettings();
          settings->setValue("Settings/rcloneVersion", rclone_version_no);

#if defined(Q_OS_WIN32)
          // check if required version
          unsigned int result =
              compareVersion(rclone_version_no.toStdString(), "1.50");

          if (result == 2) {
            QMessageBox::warning(
                this, "",
                "For mount functionality to work you need "
                "rclone version at least v1.50 "
                "and your current version is v" +
                    rclone_version_no +
                    ". Mount will be disabled. \n\nPlease consider upgrading.");
          };
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
          QStringList lines = version.split("\n", QString::SkipEmptyParts);
#else
          QStringList lines = version.split("\n", Qt::SkipEmptyParts);
#endif
          QString rclone_info2;
          QString rclone_info3;

          int counter = 0;
          foreach (QString line, lines) {
            line = line.trimmed();
            if (counter == 1)
              rclone_info2 = line.replace("- ", "");
            if (counter == 2)
              rclone_info3 = line.replace("- ", "");
            counter++;
          };

          QFileInfo appBundlePath;
#ifdef Q_OS_MACOS
          if (IsPortableMode()) {

            QFileInfo applicationPath = qApp->applicationFilePath();
            QFileInfo MacOSPath = applicationPath.dir().path();
            QFileInfo ContentsPath = MacOSPath.dir().path();
            appBundlePath = ContentsPath.dir().path();

            mStatusMessage->setText(
                rclone_info1 + " in " +
                QDir::toNativeSeparators(GetRclone().replace(
                    appBundlePath.fileName() + "/Contents/MacOS/../../../",
                    "")) +
                ", " + rclone_info2 + ", " + rclone_info3);

          } else {

            mStatusMessage->setText(rclone_info1 + " in " +
                                    QDir::toNativeSeparators(GetRclone()) +
                                    ", " + rclone_info2 + ", " + rclone_info3);
          }
#else
#ifdef Q_OS_WIN
          mStatusMessage->setText(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone()) + ", " +
                                  rclone_info2 + ", " + rclone_info3);
#else
          if (IsPortableMode()) {
            QString xdg_config_home = qgetenv("XDG_CONFIG_HOME");
            QString appImageConfigFolder = xdg_config_home.right(xdg_config_home.length()-xdg_config_home.lastIndexOf("/"));

            mStatusMessage->setText(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone().replace(appImageConfigFolder + "/..",  "")) + ", " +
                                  rclone_info2 + ", " + rclone_info3);
          } else {
            mStatusMessage->setText(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone()) + ", " +
                                  rclone_info2 + ", " + rclone_info3);
         }
#endif
#endif

          rcloneListRemotes();
        } else {
          if (p->error() != QProcess::FailedToStart) {
            if (getConfigPassword(p)) {
              rcloneGetVersion();
            } else {
              close();
            }
            p->deleteLater();
            return;
          }

          if (firstTime) {
            if (p->error() == QProcess::FailedToStart) {
              QMessageBox::information(
                  this, "Error",
                  "Wrong rclone executable or rclone not found!\nPlease select "
                  "its location in next dialog.");
            } else {
              QMessageBox::information(this, "Error",
                                       "Cannot check rclone version!\nPlease "
                                       "verify rclone location.");
            }
            emit ui.preferences->trigger();
          }
        }

        auto settings = GetSettings();

        /// check rclone version

        // get already stored rclone version no
        QString rclone_version_no =
            settings->value("Settings/rcloneVersion").toString();

        // during first run the key might not exist yet
        if (!(settings->contains("Settings/checkRcloneUpdates"))) {
          // if checkRcloneUpdates does not exist create new key
          settings->setValue("Settings/checkRcloneUpdates", true);
        };

        bool checkRcloneUpdates =
            settings->value("Settings/checkRcloneUpdates").toBool();

        // if check updates enabled in settings
        if (checkRcloneUpdates) {
          QString last_check;
          QString current_date = QDate::currentDate().toString();

          if (!(settings->contains("Settings/lastRcloneUpdateCheck"))) {
            // if lastRcloneUpdateCheck does not exist create new key
            settings->setValue("Settings/lastRcloneUpdateCheck", current_date);
          } else { // read last check date
            last_check =
                settings->value("Settings/lastRcloneUpdateCheck").toString();
          };

          // dont check if already checked today (once per day only)
          if (!(last_check == current_date)) {
            // remmber when last checked
            settings->setValue("Settings/lastRcloneUpdateCheck", current_date);

            QString url =
                "https://api.github.com/repos/rclone/rclone/releases/latest";
            QNetworkAccessManager manager;
            QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
            QEventLoop event;
            connect(response, SIGNAL(finished()), &event, SLOT(quit()));
            event.exec();
            QByteArray content = response->readAll();
            QJsonParseError jsonError;

            QJsonDocument document = QJsonDocument::fromJson(
                content, &jsonError); // parse and capture the error flag

            if (jsonError.error == QJsonParseError::NoError) {

              if (document.object().contains("tag_name")) {

                QJsonValue tag_name = document.object().value("tag_name");

                QString rclone_latest_version_no = tag_name.toString(QString());

                rclone_latest_version_no.replace("v", "");
                rclone_latest_version_no.replace("-DEV", "");
                rclone_latest_version_no = rclone_latest_version_no.trimmed();

                // check if new version available and if yes display information
                unsigned int result =
                    compareVersion(rclone_latest_version_no.toStdString(),
                                   rclone_version_no.toStdString());
                // latest version is greater than current
                if (result == 1) {

                  QMessageBox::information(
                      this, "",
                      QString(
                          R"(<p>New rclone version is available</p>)"
                          R"(<p>You have: v)" +
                          rclone_version_no +
                          "<br />"
                          R"(New version: v)" +
                          rclone_latest_version_no +
                          "</p>"
                          R"(<p>Visit rclone <a href="https://rclone.org/downloads/">downloads</a> page to upgrade</p>)"));
                };
              };
            };
          };
        };

        /// check rclone browser version

        // during first run the key might not exist yet
        if (!(settings->contains("Settings/checkRcloneBrowserUpdates"))) {
          // if checkRcloneBrowserUpdates does not exist create new key
          settings->setValue("Settings/checkRcloneBrowserUpdates", true);
        };

        bool checkRcloneBrowserUpdates =
            settings->value("Settings/checkRcloneBrowserUpdates").toBool();

        // if check updates enabled in settings
        if (checkRcloneBrowserUpdates) {
          QString last_check;
          QString current_date = QDate::currentDate().toString();

          if (!(settings->contains("Settings/lastRcloneBrowserUpdateCheck"))) {
            // if lastRcloneBrowserUpdateCheck does not exist create new key
            settings->setValue("Settings/lastRcloneBrowserUpdateCheck",
                               current_date);
          } else { // read last check date
            last_check =
                settings->value("Settings/lastRcloneBrowserUpdateCheck")
                    .toString();
          };

          // dont check if already checked today (once per day only)
          if (!(last_check == current_date)) {
            // remmber when last checked
            settings->setValue("Settings/lastRcloneBrowserUpdateCheck",
                               current_date);

            // get latest version available
            QString url = "https://api.github.com/repos/kapitainsky/"
                          "rclonebrowser/releases/latest";
            QNetworkAccessManager manager;
            QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
            QEventLoop event;
            connect(response, SIGNAL(finished()), &event, SLOT(quit()));
            event.exec();
            QByteArray content = response->readAll();

            QJsonParseError jsonError;
            QJsonDocument document = QJsonDocument::fromJson(
                content, &jsonError); // parse and capture the error flag

            if (jsonError.error == QJsonParseError::NoError) {
              if (document.object().contains("tag_name")) {
                QJsonValue tag_name = document.object().value("tag_name");
                QString rclone_browser_latest_version_no =
                    tag_name.toString(QString());
                rclone_browser_latest_version_no =
                    rclone_browser_latest_version_no.trimmed();

                // check if new version available and if yes display information
                unsigned int result = compareVersion(
                    rclone_browser_latest_version_no.toStdString(),
                    RCLONE_BROWSER_VERSION);
                // latest version is greater than current
                if (result == 1) {
                  QMessageBox::information(
                      this, "",
                      QString(
                          R"(<p>New Rclone Browser version is available</p>)"
                          R"(<p>You have: v)" RCLONE_BROWSER_VERSION "<br />"
                          R"(New version: v)" +
                          rclone_browser_latest_version_no +
                          "</p>"
                          R"(<p>Visit <a href="https://github.com/kapitainsky/RcloneBrowser/releases/latest">releases</a> page to download</p>)"));
                };
              };
            };
          };
        };

        p->deleteLater();
      });

  UseRclonePassword(p);
  p->start(GetRclone(),
           QStringList() << "version"
                         << "--ask-password=false",
           QIODevice::ReadOnly);
}

void MainWindow::rcloneConfig() {

  // for macOS and Linux we have to take care of possible spaces in rclone and
  // rclone.conf paths by using "" around them
  QString terminalRcloneCmd;
  if (!GetRcloneConf().isEmpty()) {
    terminalRcloneCmd = "\"" + GetRclone() + "\"" + " config" + " --config " +
                        "\"" + GetRcloneConf().at(1) + "\"";
  } else {
    terminalRcloneCmd = "\"" + GetRclone() + "\"" + " config";
  }

#if defined(Q_OS_WIN32) && (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
  QProcess::startDetached(GetRclone(), QStringList()
                                           << "config" << GetRcloneConf());
  return;
#else

  QProcess *p = new QProcess(this);

  QObject::connect(p,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int code, QProcess::ExitStatus) {
                     if (code == 0) {
                       emit rcloneListRemotes();
                     }
                     p->deleteLater();
                   });
#endif

#if defined(Q_OS_WIN32)
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  p->setCreateProcessArgumentsModifier(
      [](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NEW_CONSOLE;
        args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
      });
  p->setProgram(GetRclone());
  p->setArguments(QStringList() << "config" << GetRcloneConf());
#endif

#elif defined(Q_OS_MACOS)
  auto tmp = new QFile("/tmp/rclone_config.command");
  tmp->open(QIODevice::WriteOnly);
  QTextStream(tmp) << "#!/bin/sh\n" << terminalRcloneCmd << "\n";
  tmp->close();
  tmp->setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser |
                      QFileDevice::ExeUser | QFileDevice::ReadGroup |
                      QFileDevice::ExeGroup | QFileDevice::ReadOther |
                      QFileDevice::ExeOther);
  p->setProgram("open");
  p->setArguments(QStringList() << tmp->fileName());
#else
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString terminal = env.value("TERMINAL");
  if (terminal.isEmpty()) {
    terminal = QStandardPaths::findExecutable("gnome-terminal");
    if (terminal.isEmpty()) {
      terminal = QStandardPaths::findExecutable("xfce4-terminal");
      if (terminal.isEmpty()) {
        terminal = QStandardPaths::findExecutable("xterm");
        if (terminal.isEmpty()) {
          terminal = QStandardPaths::findExecutable("x-terminal-emulator");
          if (terminal.isEmpty()) {
            terminal = QStandardPaths::findExecutable("konsole");
            if (terminal.isEmpty()) {
              QMessageBox::critical(this, "Error",
                                    "Not sure how to launch terminal!\n"
                                    "Please set path to terminal executable in "
                                    "$TERMINAL environment variable.",
                                    QMessageBox::Ok);
              return;
            }
          }
        }
      }
    }
  }

  p->setArguments(QStringList() << "-e" << terminalRcloneCmd);
  p->setProgram(terminal);
#endif

#if !defined(Q_OS_WIN32) || (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  UseRclonePassword(p);
  p->start(QIODevice::NotOpen);
#endif
}

void MainWindow::rcloneListRemotes() {
  ui.remotes->clear();

  QProcess *p = new QProcess();

  QObject::connect(
      p,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int code, QProcess::ExitStatus) {
        if (code == 0) {
          QStyle *style = qApp->style();

          QString bytes = p->readAllStandardOutput().trimmed();
          QStringList items = bytes.split('\n');

          auto settings = GetSettings();
          bool darkModeIni = settings->value("Settings/darkModeIni").toBool();
          QString iconSize = settings->value("Settings/iconSize").toString();

          for (const QString &line : items) {
            if (line.isEmpty()) {
              continue;
            }

            QStringList parts = line.split(':');
            if (parts.count() != 2) {
              continue;
            }

            QString name = parts[0].trimmed();
            QString type = parts[1].trimmed();
            QString tooltip = type;

            QString img_add = "";
            int size;

            // medium scale by default
            double darkModeIconScale = 1.333;
            double lightModeiconScale = 2;
            // to avoid "variable not used" compiler error
            if (darkModeIconScale == lightModeiconScale) {
            };

            // set icons scale based on iconSize value
            if (iconSize == "small") {
              lightModeiconScale = 1.5;
              darkModeIconScale = 1;
            }

            if (iconSize == "medium") {
              lightModeiconScale = 2;
              darkModeIconScale = 1.333;
            }

            if (iconSize == "large") {
              lightModeiconScale = 3;
              darkModeIconScale = 2;
            }

#if !defined(Q_OS_MACOS)
            // _inv only for dark mode
            // we use darkModeIni to apply mode active at startup
            if (darkModeIni) {
              img_add = "_inv";

            } else {
              img_add = "";
            }
#if defined(Q_OS_WIN)
            // on Windows dark theme changes PM_ListViewIconSize size
            // so we have to adjust
            if (darkModeIni) {
              size = darkModeIconScale *
                     style->pixelMetric(QStyle::PM_ListViewIconSize);
            } else {
              size = lightModeiconScale *
                     style->pixelMetric(QStyle::PM_ListViewIconSize);
            }
#else
             // for Linux/BSD PM_ListViewIconSize stays the same
             size = lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
#endif
#else
             QString sysInfo = QSysInfo::productVersion();
             // dark mode on older macOS
             if (sysInfo == "10.9" ||
                 sysInfo == "10.10" ||
                 sysInfo == "10.11" ||
                 sysInfo == "10.12" ||
                 sysInfo == "10.13") {

               // on older macOS we also have to adjust icon size per mode
               if (darkModeIni) {
                 size = darkModeIconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
                 img_add = "_inv";
               } else {
                 size = lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
                 img_add = "";
               }

             } else {
               // for macOS > 10.13 native dark mode does not change IconSize base
               size = 1.5 * lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
             }
#endif
            ui.remotes->setIconSize(QSize(size, size));

            QString path =
                ":/remotes/images/" + type.replace(' ', '_') + img_add + ".png";
            QIcon icon(QFile(path).exists()
                           ? path
                           : ":/remotes/images/unknown" + img_add + ".png");

            QListWidgetItem *item = new QListWidgetItem(icon, name);
            item->setData(Qt::UserRole, type);
            item->setToolTip(tooltip);
            ui.remotes->addItem(item);
          }
        } else {
          if (p->error() != QProcess::FailedToStart) {
            if (getConfigPassword(p)) {
              rcloneListRemotes();
            }
          }
        }
        p->deleteLater();
      });

  UseRclonePassword(p);
  p->start(GetRclone(),
           QStringList() << "listremotes" << GetRcloneConf() << "--long"
                         << "--ask-password=false",
           QIODevice::ReadOnly);
}

bool MainWindow::getConfigPassword(QProcess *p) {
  QString output = p->readAllStandardError().trimmed();
  if (output.indexOf("RCLONE_CONFIG_PASS") > 0) {
    bool ok;
    QString password = QInputDialog::getText(
        this, qApp->applicationDisplayName(),
        "Enter password for .rclone.conf configuration file:",
        QLineEdit::Password, QString(), &ok);
    if (ok) {
      SetRclonePassword(password);
      return true;
    }
  } else if (output.indexOf("unknown command \"listremotes\"") > 0) {
    QMessageBox::critical(this, qApp->applicationDisplayName(),
                          "It seems rclone version you are using is too "
                          "old.\nPlease upgrade to the latest version");
    return false;
  }
  return false;
}

bool MainWindow::canClose() {
  if (mJobCount == 0) {
    return true;
  }

  bool wasVisible = isVisible();

  ui.tabs->setCurrentIndex(1);
  showNormal();

  int button =
      QMessageBox::question(this, "Rclone Browser",
                            QString("There are %1 job(s) running.\n"
                                    "Do you want to stop them and quit?")
                                .arg(mJobCount),
                            QMessageBox::Yes | QMessageBox::No);

  if (!wasVisible) {
    hide();
  }

  if (button == QMessageBox::Yes) {
    for (int i = 0; i < ui.jobs->count(); i++) {
      QWidget *widget = ui.jobs->itemAt(i)->widget();
      if (auto mount = qobject_cast<MountWidget *>(widget)) {
        mount->cancel();
      } else if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        transfer->cancel();
      } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
        stream->cancel();
      }
    }
    return true;
  }

  return false;
}

void MainWindow::closeEvent(QCloseEvent *ev) {
  if (mCloseToTray && isVisible()) {
#ifdef Q_OS_MACOS
    osxHideDockIcon();
#endif
    mSystemTray.show();
    hide();
    ev->ignore();
    return;
  }

  if (canClose()) {
    QApplication::quit();
  } else {
    ev->ignore();
  }
}

void MainWindow::listTasks() {
  ui.tasksListWidget->clear();

  ListOfJobOptions *ljo = ListOfJobOptions::getInstance();

  for (JobOptions *jo : ljo->getTasks()) {
    JobOptionsListWidgetItem *item = new JobOptionsListWidgetItem(
        jo,
        jo->jobType == JobOptions::JobType::Download ? mDownloadIcon
                                                     : mUploadIcon,
        jo->description);
    ui.tasksListWidget->addItem(item);
  }
}

void MainWindow::runItem(JobOptionsListWidgetItem *item, bool dryrun) {
  if (item == nullptr)
    return;
  JobOptions *jo = item->GetData();
  jo->dryRun = dryrun;
  QStringList args = jo->getOptions();
  addTransfer(QString("%1 %2").arg(jo->operation).arg(jo->source), jo->source,
              jo->dest, args);
}

void MainWindow::editSelectedTask() {
  auto selection = ui.tasksListWidget->selectionModel()->currentIndex();
  JobOptionsListWidgetItem *item = static_cast<JobOptionsListWidgetItem *>(
      ui.tasksListWidget->currentItem());
  JobOptions *jo = item->GetData();
  bool isDownload = (jo->jobType == JobOptions::Download);
  QString remote = isDownload ? jo->source : jo->dest;
  QString path = isDownload ? jo->dest : jo->source;
  // qDebug() << "remote:" + remote;
  // qDebug() << "path:" + path;
  TransferDialog td(isDownload, false, remote, path, jo->isFolder, this, jo,
                    true);
  td.exec();
  // restore the selection to help user keep track of what s/he was doing
  ui.tasksListWidget->selectionModel()->select(selection,
                                               QItemSelectionModel::Select);
  // edit mode on the TransferDialog suppresses the usual Accept buttons
  // and the Save Task button closes it... so there is nothing more to do here
}

void MainWindow::addTransfer(const QString &message, const QString &source,
                             const QString &dest, const QStringList &args) {
  QProcess *transfer = new QProcess(this);
  transfer->setProcessChannelMode(QProcess::MergedChannels);

  auto widget = new JobWidget(transfer, message, args, source, dest);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QObject::connect(
      widget, &JobWidget::finished, this, [=](const QString &info) {
        if (mNotifyFinishedTransfers) {
          qApp->alert(this);
          mLastFinished = widget;
          mSystemTray.showMessage("Transfer finished", info);
        }

        if (--mJobCount == 0) {
          ui.tabs->setTabText(1, "Jobs");
        } else {
          ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
        }
      });

  QObject::connect(widget, &JobWidget::closed, this, [=]() {
    if (widget == mLastFinished) {
      mLastFinished = nullptr;
    }
    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;
    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }
  });

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);
  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  UseRclonePassword(transfer);
  transfer->start(GetRclone(), GetRcloneConf() + args, QIODevice::ReadOnly);
}

void MainWindow::addMount(const QString &remote, const QString &folder) {
  QProcess *mount = new QProcess(this);
  mount->setProcessChannelMode(QProcess::MergedChannels);

  auto widget = new MountWidget(mount, remote, folder);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QObject::connect(widget, &MountWidget::finished, this, [=]() {
    if (--mJobCount == 0) {
      ui.tabs->setTabText(1, "Jobs");
    } else {
      ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
    }
  });

  QObject::connect(widget, &MountWidget::closed, this, [=]() {
    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;
    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }
  });

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);
  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  auto settings = GetSettings();
  QString opt = settings->value("Settings/mount").toString();
  bool driveShared = settings->value("Settings/driveShared").toBool();

  QStringList args;
  args << "mount";

#if defined(Q_OS_WIN32)
  args << "--rc";
  args << "--rc-addr";

  // calculate remote control interface port based on mount drive letter
  // this way every mount will have unique port assigned
  int port_offset = folder[0].toLatin1();
  unsigned short int rclone_rc_port_base = 19000;
  unsigned short int rclone_rc_port = rclone_rc_port_base + port_offset;
  args << "localhost:" + QVariant(rclone_rc_port).toString();
#endif

  // for google drive "shared with me" without --read-only writes go created in
  // main google drive it is more logical to mount it as read only so there is
  // no confusion
  if (driveShared) {
    args << "--drive-shared-with-me";
    args << "--read-only";
  };

  //	 default mount is now more generic. all options can be passed via
  // preferences mount field
  //       args << "--vfs-cache-mode";
  //       args << "writes";

  args.append(GetRcloneConf());
  if (!opt.isEmpty()) {
    args.append(opt.split(' '));
  }
  args << remote << folder;

  UseRclonePassword(mount);
  mount->start(GetRclone(), args, QIODevice::ReadOnly);
}

void MainWindow::addStream(const QString &remote, const QString &stream) {
  auto player = new QProcess();
  auto rclone = new QProcess();
  rclone->setStandardOutputProcess(player);

  QObject::connect(
      player,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int status, QProcess::ExitStatus) {
        player->deleteLater();
        if (status != 0 && player->error() == QProcess::FailedToStart) {
          QMessageBox::critical(
              this, "Error",
              QString("Failed to start '%1' player process").arg(stream));
          auto settings = GetSettings();
          settings->remove("Settings/streamConfirmed");
        }
      });

  auto widget = new StreamWidget(rclone, player, remote, stream);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QObject::connect(widget, &StreamWidget::finished, this, [=]() {
    if (--mJobCount == 0) {
      ui.tabs->setTabText(1, "Jobs");
    } else {
      ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
    }
  });

  QObject::connect(widget, &StreamWidget::closed, this, [=]() {
    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;
    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }
  });

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);
  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  #if defined(Q_OS_WIN)
  player->setProgram("cmd");
  player->setArguments(QStringList() << "/C" << stream);
  #else
  player->setProgram("/bin/sh");
  player->setArguments(QStringList() << "-c" << stream);
  #endif
  player->start(QProcess::ReadOnly);
  UseRclonePassword(rclone);
  rclone->start(GetRclone(),
                QStringList() << "cat" << GetRcloneConf() << remote,
                QProcess::WriteOnly);
}
