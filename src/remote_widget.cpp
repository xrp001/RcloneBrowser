#include "remote_widget.h"
#include "export_dialog.h"
#include "icon_cache.h"
#include "item_model.h"
#include "list_of_job_options.h"
#include "progress_dialog.h"
#include "s3_tags.h"
#include "transfer_dialog.h"
#include "utils.h"

RemoteWidget::RemoteWidget(IconCache *iconCache, const QString &remote,
                           bool isLocal, bool isGoogle,
                           const QString &remoteType, QWidget *parent)
    : QWidget(parent), mRemoteType(remoteType) {
  ui.setupUi(this);

QString root = isLocal ? "/" : QString();

#ifndef Q_OS_WIN
  isLocal = false;
#endif

  auto settings = GetSettings();
  QString rcloneVersion = settings->value("Settings/rcloneVersion").toString();
  settings->setValue("Settings/driveShared", Qt::Unchecked);
  ui.tree->setAlternatingRowColors(
      settings->value("Settings/rowColors", false).toBool());
  ui.checkBoxShared->setChecked(false);
  ui.checkBoxShared->setDisabled(!isGoogle);
  // hide checkBoxShared for non Google remotes
  if (!isGoogle) {
    ui.checkBoxShared->hide();
  }

  QStyle *style = QApplication::style();
  ui.refresh->setIcon(style->standardIcon(QStyle::SP_BrowserReload));
  ui.mkdir->setIcon(style->standardIcon(QStyle::SP_FileDialogNewFolder));
  ui.rename->setIcon(style->standardIcon(QStyle::SP_FileIcon));
  ui.move->setIcon(style->standardIcon(QStyle::SP_DirOpenIcon));
  ui.purge->setIcon(style->standardIcon(QStyle::SP_TrashIcon));
  ui.mount->setIcon(style->standardIcon(QStyle::SP_DriveNetIcon));
  ui.stream->setIcon(style->standardIcon(QStyle::SP_MediaPlay));
  ui.upload->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
  ui.download->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
  ui.download->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
  ui.getSize->setIcon(style->standardIcon(QStyle::SP_FileDialogInfoView));
  ui.getTree->setIcon(style->standardIcon(QStyle::SP_FileDialogListView));
  ui.export_->setIcon(style->standardIcon(QStyle::SP_FileDialogDetailedView));
  ui.link->setIcon(style->standardIcon(QStyle::SP_FileLinkIcon));
  ui.head->setIcon(style->standardIcon(QStyle::SP_MessageBoxInformation));
  ui.tags->setIcon(style->standardIcon(QStyle::SP_DialogOpenButton));

  ui.buttonRefresh->setDefaultAction(ui.refresh);
  ui.buttonMkdir->setDefaultAction(ui.mkdir);
  ui.buttonRename->setDefaultAction(ui.rename);
  ui.buttonMove->setDefaultAction(ui.move);
  ui.buttonPurge->setDefaultAction(ui.purge);
  ui.buttonMount->setDefaultAction(ui.mount);
  ui.buttonStream->setDefaultAction(ui.stream);
  ui.buttonUpload->setDefaultAction(ui.upload);
  ui.buttonDownload->setDefaultAction(ui.download);
  ui.buttonTree->setDefaultAction(ui.getTree);
  ui.buttonLink->setDefaultAction(ui.link);
  ui.buttonHead->setDefaultAction(ui.head);
  QToolButton *buttonTags = new QToolButton(ui.buttons);
  buttonTags->setObjectName("buttonTags");
  buttonTags->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  buttonTags->setDefaultAction(ui.tags);
  ui.horizontalLayout_2->insertWidget(ui.horizontalLayout_2->indexOf(ui.buttonExport),
                                      buttonTags);
  ui.buttonSize->setDefaultAction(ui.getSize);
  ui.buttonExport->setDefaultAction(ui.export_);

  ui.tree->sortByColumn(0, Qt::AscendingOrder);
  ui.tree->header()->setSectionsMovable(false);

  ItemModel *model = new ItemModel(iconCache, remote, this);
  ui.tree->setModel(model);
  QTimer::singleShot(0, ui.tree, SLOT(setFocus()));

  QObject::connect(model, &QAbstractItemModel::layoutChanged, this, [=]() {
    ui.tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.tree->resizeColumnToContents(1);
    ui.tree->resizeColumnToContents(2);
  });

  QObject::connect(
      ui.tree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
      [=](const QItemSelection &selection) {
        for (auto child : findChildren<QAction *>()) {
          child->setDisabled(selection.isEmpty());
        }

        if (selection.isEmpty()) {
          ui.path->clear();
          return;
        }

        QModelIndex index = selection.indexes().front();

        bool topLevel = model->isTopLevel(index);
        bool isFolder = model->isFolder(index);
        bool isS3Remote = mRemoteType.startsWith("s3", Qt::CaseInsensitive);

        QDir path;
        if (model->isLoading(index)) {
          ui.refresh->setDisabled(true);
          ui.move->setDisabled(true);
          ui.rename->setDisabled(true);
          ui.purge->setDisabled(true);
          ui.mount->setDisabled(true);
          ui.stream->setDisabled(true);
          ui.upload->setDisabled(true);
          ui.download->setDisabled(true);
          ui.head->setDisabled(true);
          ui.tags->setDisabled(true);
          ui.checkBoxShared->setDisabled(true);
          path = model->path(model->parent(index));
        } else {
          ui.refresh->setDisabled(false);
          bool driveShared = ui.checkBoxShared->checkState();
          ui.mkdir->setDisabled(driveShared);
          ui.rename->setDisabled(topLevel || driveShared);
          ui.move->setDisabled(topLevel || driveShared);
          ui.purge->setDisabled(topLevel || driveShared);
          ui.upload->setDisabled(driveShared);

#if defined(Q_OS_WIN32)
          // check if required version
          unsigned int result =
              compareVersion(rcloneVersion.toStdString(), "1.50");
          if (result == 2) {
            ui.mount->setDisabled(true);
          } else {
            ui.mount->setDisabled(!isFolder);
          };
#else
// mount is not supported by rclone on these systems
#if defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
          ui.mount->setDisabled(true);
#else
          ui.mount->setDisabled(!isFolder);
#endif
#endif

          ui.stream->setDisabled(isFolder);
          ui.head->setDisabled(topLevel || isFolder || !isS3Remote ||
                               driveShared);
          ui.tags->setDisabled(topLevel || isFolder || !isS3Remote ||
                               driveShared);
          ui.checkBoxShared->setDisabled(!isGoogle);
          path = model->path(index);
        }

        ui.getSize->setDisabled(!isFolder);
        ui.getTree->setDisabled(!isFolder);
        ui.export_->setDisabled(!isFolder);
        ui.path->setText(isLocal ? QDir::toNativeSeparators(path.path())
                                 : path.path());
      });

  QObject::connect(ui.refresh, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    model->refresh(index);
  });

  QObject::connect(ui.mkdir, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    if (!model->isFolder(index)) {
      index = index.parent();
    }
    QDir path = model->path(index);
    QString pathMsg =
        isLocal ? QDir::toNativeSeparators(path.path()) : path.path();
    QString name = QInputDialog::getText(
        this, "New Folder", QString("Create folder in %1").arg(pathMsg));
    if (!name.isEmpty()) {
      QString folder = path.filePath(name);
      QString folderMsg = isLocal ? QDir::toNativeSeparators(folder) : folder;

      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList() << "mkdir" << GetRcloneConf()
                                         << GetDriveSharedWithMe()
                                         << GetDefaultRcloneOptionsList()
                                         << remote + ":" + folder);
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("New Folder", "Creating...", folderMsg, &process,
                              this);
      if (progress.exec() == QDialog::Accepted) {
        model->refresh(index);
      }
    }
  });

  QObject::connect(ui.rename, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QString name = model->data(index, Qt::DisplayRole).toString();
    name = QInputDialog::getText(this, "Rename",
                                 QString("New name for %1").arg(pathMsg),
                                 QLineEdit::Normal, name);
    if (!name.isEmpty()) {
      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(
          QStringList() << "moveto" << GetRcloneConf() << GetDriveSharedWithMe()
                        << GetDefaultRcloneOptionsList() << remote + ":" + path
                        << remote + ":" +
                               model->path(index.parent()).filePath(name));
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("Rename", "Renaming...", pathMsg, &process, this);
      if (progress.exec() == QDialog::Accepted) {
        model->rename(index, name);
      }
    }
  });

  QObject::connect(ui.move, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QString name = model->path(index.parent()).path() + "/";
    name = QInputDialog::getText(this, "Move",
                                 QString("New location for %1").arg(pathMsg),
                                 QLineEdit::Normal, name);
    if (!name.isEmpty()) {
      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(
          QStringList() << "move" << GetRcloneConf() << GetDriveSharedWithMe()
                        << GetDefaultRcloneOptionsList() << remote + ":" + path
                        << remote + ":" + name);
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("Move", "Moving...", pathMsg, &process, this);
      if (progress.exec() == QDialog::Accepted) {
        model->refresh(index);
      }
    }
  });

  QObject::connect(ui.purge, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    int button = QMessageBox::question(
        this, "Delete",
        QString("Are you sure you want to delete %1 ?").arg(pathMsg),
        QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::Yes) {
      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList()
                           << (model->isFolder(index) ? "purge" : "delete")
                           << GetRcloneConf() << GetDriveSharedWithMe()
                           << GetDefaultRcloneOptionsList()
                           << remote + ":" + path);
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("Delete", "Deleting...", pathMsg, &process, this);
      if (progress.exec() == QDialog::Accepted) {
        QModelIndex parent = index.parent();
        QModelIndex next = parent.model()->index(index.row() + 1, 0);
        ui.tree->selectionModel()->select(next.isValid() ? next : parent,
                                          QItemSelectionModel::SelectCurrent);
        model->removeRow(index.row(), parent);
      }
    }
  });

  QObject::connect(ui.mount, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

#if defined(Q_OS_WIN32)
    QString folder =
        QInputDialog::getText(this, "Mount",
                              QString("(Make sure you have WinFsp-FUSE "
                                      "installed)\n\nDrive to mount %1 to")
                                  .arg(remote),
                              QLineEdit::Normal, "Z:");
#else
        QString folder = QFileDialog::getExistingDirectory(this, QString("Mount %1").arg(remote));
#endif

    if (!folder.isEmpty()) {
      emit addMount(remote + ":" + path, folder);
    }
  });

  QObject::connect(ui.stream, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QString path = model->path(index).path();

    bool streamConfirmed =
        settings->value("Settings/streamConfirmed", false).toBool();
    QString stream = settings->value("Settings/stream", "mpv -").toString();
    if (!streamConfirmed) {
      QString result = QInputDialog::getText(
          this, "Stream",
          "Enter stream command (file will be passed in STDIN):",
          QLineEdit::Normal, stream);
      if (result.isEmpty()) {
        return;
      }

      stream = result;

      settings->setValue("Settings/stream", stream);
      settings->setValue("Settings/streamConfirmed", true);
    }

    emit addStream(remote + ":" + path, stream);
  });

  QObject::connect(ui.checkBoxShared, &QCheckBox::toggled, ui.shared,
                   &QAction::toggled);

  QObject::connect(ui.shared, &QAction::toggled, this, [=](const bool checked) {
    auto settings = GetSettings();
    settings->setValue("Settings/driveShared", checked);
    ui.checkBoxShared->setChecked(checked);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QModelIndex top = index;
    while (!model->isTopLevel(top)) {
      top = top.parent();
    }
    ui.tree->selectionModel()->clear();
    ui.tree->selectionModel()->select(top, QItemSelectionModel::Select |
                                               QItemSelectionModel::Rows);
    model->refresh(top);
  });

  QObject::connect(ui.link, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(
        QStringList() << "link" << GetRcloneConf() << GetDriveSharedWithMe()
                      << GetDefaultRcloneOptionsList() << remote + ":" + path);
    process.setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog progress("Fetch Public Link", "Fetching link for...",
                            pathMsg, &process, this, false, true);
    progress.expand();
    progress.allowToClose();
    progress.exec();
  });

  QObject::connect(ui.head, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(QStringList()
                         << "lsjson" << "--stat" << "--metadata"
                         << GetRcloneConf() << GetDriveSharedWithMe()
                         << GetDefaultRcloneOptionsList()
                         << remote + ":" + path);
    process.setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog progress("Head Object", "Fetching object metadata...",
                            pathMsg, &process, this, false);
    progress.expand();
    progress.allowToClose();
    progress.resize(1000, 600);
    progress.exec();
  });

  QObject::connect(ui.tags, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    S3ObjectTagsResult current = GetS3ObjectTags(remote, path);
    QApplication::restoreOverrideCursor();

    if (!current.ok) {
      QMessageBox::warning(this, "Tags",
                           QString("Failed to fetch S3 object tags for:\n%1\n\n%2")
                               .arg(pathMsg)
                               .arg(current.error));
      return;
    }

    QDialog dialog(this);
    QDialog *dialogPtr = &dialog;
    dialog.setWindowTitle("Tags");
    dialog.resize(720, 420);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QWidget *header = new QWidget(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *title =
        new QLabel(QString("S3 object tags for <b>%1</b>").arg(pathMsg), &dialog);
    title->setTextFormat(Qt::RichText);
    title->setWordWrap(true);

    QPushButton *setTagsButton = new QPushButton("Set Tags...", &dialog);
    setTagsButton->setIcon(
        style->standardIcon(QStyle::SP_FileDialogDetailedView));

    headerLayout->addWidget(title, 1);
    headerLayout->addWidget(setTagsButton, 0, Qt::AlignTop);
    layout->addWidget(header);

    QFrame *panel = new QFrame(&dialog);
    panel->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);

    QTableWidget *table = new QTableWidget(&dialog);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << "Key" << "Value");
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setAlternatingRowColors(true);

    QLabel *emptyLabel = new QLabel("No tags have been set for this object yet.",
                                    &dialog);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    emptyLabel->setStyleSheet(
        "color: palette(mid); padding: 32px; font-size: 14px;");

    auto refreshTagsView = [=](const QList<S3ObjectTag> &tags) {
      table->clearContents();
      table->setRowCount(tags.count());
      for (int i = 0; i < tags.count(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(tags[i].key));
        table->setItem(i, 1, new QTableWidgetItem(tags[i].value));
      }
      bool hasTags = !tags.isEmpty();
      table->setVisible(hasTags);
      emptyLabel->setVisible(!hasTags);
    };

    panelLayout->addWidget(table);
    panelLayout->addWidget(emptyLabel);
    layout->addWidget(panel, 1);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttons);

    QList<S3ObjectTag> currentTags = current.tags;
    refreshTagsView(currentTags);

    QObject::connect(setTagsButton, &QPushButton::clicked, &dialog,
                     [=, &currentTags]() {
                       bool ok = false;
                       QString text = QInputDialog::getMultiLineText(
                           dialogPtr, "Set Tags",
                           QString("Manage S3 object tags for:\n%1\n\nOne tag "
                                   "per line: key:value")
                               .arg(pathMsg),
                           FormatS3ObjectTags(currentTags), &ok);
                       if (!ok) {
                         return;
                       }

                       S3ObjectTagsResult parsed =
                           ParseS3ObjectTagsInput(text);
                       if (!parsed.ok) {
                         QMessageBox::warning(dialogPtr, "Set Tags",
                                              parsed.error);
                         return;
                       }

                       QApplication::setOverrideCursor(Qt::WaitCursor);
                       S3ObjectTagsResult updated =
                           SetS3ObjectTags(remote, path, parsed.tags);
                       QApplication::restoreOverrideCursor();

                       if (!updated.ok) {
                         QMessageBox::warning(
                             dialogPtr, "Set Tags",
                             QString("Failed to set S3 object tags for:\n%1\n\n%2")
                                 .arg(pathMsg)
                                 .arg(updated.error));
                         return;
                       }

                       currentTags = updated.tags;
                       refreshTagsView(currentTags);
                     });

    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog,
                     &QDialog::reject);
    dialog.exec();
  });

  QObject::connect(ui.upload, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    if (!model->isFolder(index)) {
      index = index.parent();
    }
    QDir path = model->path(index);

    TransferDialog t(false, false, remote, path, true, this);
    if (t.exec() == QDialog::Accepted) {
      QString src = t.getSource();
      QString dst = t.getDest();

      QStringList args = t.getOptions();
      emit addTransfer(QString("%1 from %2").arg(t.getMode()).arg(src), src,
                       dst, args);
    }
  });

  QObject::connect(ui.download, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QDir path = model->path(index);

    TransferDialog t(true, false, remote, path, model->isFolder(index), this);
    if (t.exec() == QDialog::Accepted) {
      QString src = t.getSource();
      QString dst = t.getDest();

      QStringList args = t.getOptions();
      emit addTransfer(QString("%1 %2").arg(t.getMode()).arg(src), src, dst,
                       args);
    }
  });

  QObject::connect(ui.getTree, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));
    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(
        QStringList() << "tree"
                      << "-d" << GetRcloneConf() << GetDriveSharedWithMe()
                      << GetDefaultRcloneOptionsList() << remote + ":" + path);
    process.setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog progress("Show directories tree", "Processing...", pathMsg,
                            &process, this, false);
    progress.expand();
    progress.allowToClose();
    progress.resize(1000, 600);
    progress.exec();
  });

  QObject::connect(ui.getSize, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));
    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(
        QStringList() << "size" << GetRcloneConf() << GetDriveSharedWithMe()
                      << GetDefaultRcloneOptionsList() << remote + ":" + path);
    process.setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog progress("Get Size", "Calculating...", pathMsg, &process,
                            this, false);
    progress.expand();
    progress.allowToClose();
    progress.exec();
  });

  QObject::connect(ui.export_, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool driveShared = ui.checkBoxShared->checkState();
    (driveShared ? settings->setValue("Settings/driveShared", Qt::Checked)
                 : settings->setValue("Settings/driveShared", Qt::Unchecked));

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QDir path = model->path(index);
    ExportDialog e(remote, path, this);
    if (e.exec() == QDialog::Accepted) {
      QString dst = e.getDestination();
      bool txt = e.onlyFilenames();

      QFile *file = new QFile(dst);
      if (!file->open(QFile::WriteOnly)) {
        QMessageBox::warning(
            this, "Error",
            QString("Cannot open file '%1' for writing!").arg(dst));
        delete file;
        return;
      }

      QRegExp re(R"(^(\d+) (\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d)\.\d+ (.+)$)");

      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList()
                           << GetRcloneConf() << GetDriveSharedWithMe()
                           << GetDefaultRcloneOptionsList() << e.getOptions());
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("Export", "Exporting...", dst, &process, this);
      file->setParent(&progress);

      QObject::connect(&progress, &ProgressDialog::outputAvailable, this,
                       [=](const QString &output) {
                         QTextStream out(file);
                         out.setCodec("UTF-8");

                         for (const auto &line : output.split('\n')) {
                           if (re.exactMatch(line.trimmed())) {
                             QStringList cap = re.capturedTexts();

                             if (txt) {
                               out << cap[3] << '\n';
                             } else {
                               QString name = cap[3];
                               if (name.contains(' ') || name.contains(',') ||
                                   name.contains('"')) {
                                 name = '"' + name.replace("\"", "\"\"") + '"';
                               }
                               out << name << ',' << '"' << cap[2] << '"' << ','
                                   << cap[1].toULongLong() << '\n';
                             }
                           }
                         }
                       });

      progress.exec();
    }
  });

  QObject::connect(
      model, &ItemModel::drop, this,
      [=](const QDir &path, const QModelIndex &parent) {
        auto settings = GetSettings();
        bool driveShared = ui.checkBoxShared->checkState();
        (driveShared
             ? settings->setValue("Settings/driveShared", Qt::Checked)
             : settings->setValue("Settings/driveShared", Qt::Unchecked));

        qApp->setActiveWindow(this);
        QDir destPath = model->path(parent);
        QString dest = QFileInfo(path.path()).isDir()
                           ? destPath.filePath(path.dirName())
                           : destPath.path();

        TransferDialog t(false, true, remote, dest, true, this);
        t.setSource(path.path());

        if (t.exec() == QDialog::Accepted) {
          QString src = t.getSource();
          QString dst = t.getDest();

          QStringList args = t.getOptions();
          emit addTransfer(QString("%1 from %2").arg(t.getMode()).arg(src), src,
                           dst, args);
        }
      });

  QObject::connect(
      ui.tree, &QWidget::customContextMenuRequested, this,
      [=](const QPoint &pos) {
        auto settings = GetSettings();
        bool driveShared = ui.checkBoxShared->checkState();
        (driveShared
             ? settings->setValue("Settings/driveShared", Qt::Checked)
             : settings->setValue("Settings/driveShared", Qt::Unchecked));

        QMenu menu;
        menu.addAction(ui.refresh);
        menu.addAction(ui.getSize);
        menu.addAction(ui.getTree);
        menu.addAction(ui.export_);
        menu.addSeparator();
        menu.addAction(ui.mkdir);
        menu.addAction(ui.rename);
        menu.addAction(ui.move);
        menu.addAction(ui.purge);
        menu.addSeparator();
        menu.addAction(ui.mount);
        menu.addAction(ui.stream);
        menu.addAction(ui.upload);
        menu.addAction(ui.download);
        menu.addAction(ui.link);
        menu.addAction(ui.head);
        menu.addAction(ui.tags);
        menu.exec(ui.tree->viewport()->mapToGlobal(pos));
      });

  if (isLocal) {
    QHash<QString, QPersistentModelIndex> drives;

    // QDir::drives is fast
    for (const auto &drive : QDir::drives()) {
      QString path = drive.path();
      QModelIndex index = model->addRoot(QDir::toNativeSeparators(path), path);
      drives.insert(path, index);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)) && !(defined Q_OS_WIN)
    QThread *thread = new QThread(this);
    thread->start();

    QObject *worker = new QObject();
    worker->moveToThread(thread);

    QTimer::singleShot(0, worker, [=]() {
      QStorageInfo info;
      info.refresh();

      // QStorageInfo::mountedVolumes is slow :(
      for (const auto &volume : info.mountedVolumes()) {
        QString name = volume.name();
        if (!name.isEmpty()) {
          QString path = volume.rootPath();
          QString item =
              QString("%1 (%2)").arg(QDir::toNativeSeparators(path)).arg(name);
          QTimer::singleShot(0, this,
                             [=]() { model->rename(drives[path], item); });
        }
      }

      thread->quit();
      thread->deleteLater();
      worker->deleteLater();
    });
#endif

    ui.tree->selectionModel()->selectionChanged(QItemSelection(),
                                                QItemSelection());
  } else {
    QModelIndex index = model->addRoot("/", root);
    ui.tree->selectionModel()->select(
        index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.tree->expand(index);
  }

  QShortcut *close = new QShortcut(QKeySequence::Close, this);
  QObject::connect(close, &QShortcut::activated, this, [=]() {
    auto tabs = qobject_cast<QTabWidget *>(parent);
    tabs->removeTab(tabs->indexOf(this));
  });
}

RemoteWidget::~RemoteWidget() {}
