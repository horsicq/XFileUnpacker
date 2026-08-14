/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "guimainwindow.h"

#include "dialogoptions.h"
#include "dialogunpackfile.h"
#include "dialogxfileinfo.h"
#include "ui_guimainwindow.h"
#include "xformats.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyle>
#include <QTemporaryDir>

GuiMainWindow::GuiMainWindow(QWidget *pParent)
    : QMainWindow(pParent), ui(new Ui::GuiMainWindow), g_nCurrentRecordFileSize(0), g_pRecentFilesMenu(nullptr)
{
    ui->setupUi(this);

    setWindowTitle(XOptions::getTitle(X_APPLICATIONDISPLAYNAME, X_APPLICATIONVERSION));
    setAcceptDrops(true);

    g_xOptions.setName(X_OPTIONSFILE);
    g_xOptions.addID(XOptions::ID_VIEW_STAYONTOP, false);
    g_xOptions.addID(XOptions::ID_VIEW_STYLE, QStringLiteral("Fusion"));
    g_xOptions.addID(XOptions::ID_VIEW_LANG, QStringLiteral("System"));
    g_xOptions.addID(XOptions::ID_VIEW_QSS, QStringLiteral(""));
    g_xOptions.addID(XOptions::ID_VIEW_FONT_CONTROLS, XOptions::getDefaultFont().toString());
    g_xOptions.addID(XOptions::ID_VIEW_FONT_TREEVIEWS, XOptions::getMonoFont().toString());
    g_xOptions.addID(XOptions::ID_VIEW_FONT_TABLEVIEWS, XOptions::getMonoFont().toString());
    g_xOptions.addID(XOptions::ID_VIEW_COLUMNS, QStringLiteral("0 | 9 | 1 | 2"));
    g_xOptions.addID(XOptions::ID_VIEW_COLUMN_SIZES, QStringLiteral(""));
    g_xOptions.addID(XOptions::ID_FILE_SAVELASTDIRECTORY, true);
    g_xOptions.addID(XOptions::ID_FILE_SAVERECENTFILES, true);
    g_xOptions.addID(XOptions::ID_SCAN_ENGINE_DIE_ENABLED, true);
    g_xOptions.addID(XOptions::ID_SCAN_ENGINE_NFD_ENABLED, true);
    XScanEngineOptionsWidget::setDefaultValues(&g_xOptions);
    g_xOptions.load();

    ui->centralwidget->setGlobal(&g_xShortcuts, &g_xOptions);

    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->actionExtract->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionTest->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    ui->actionInfo->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    ui->actionRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->actionOptions->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));

    g_pLabelObjects = new QLabel(this);
    g_pLabelCurrentFile = new QLabel(this);
    ui->statusbar->addWidget(g_pLabelObjects);
    ui->statusbar->addPermanentWidget(g_pLabelCurrentFile);

    connect(&g_xOptions, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
    connect(ui->centralwidget, SIGNAL(recordsLoaded(qint32)), this, SLOT(onArchiveRecordsLoaded(qint32)));
    connect(ui->centralwidget, SIGNAL(currentRecordChanged(QString,qint64)), this, SLOT(onArchiveRecordChanged(QString,qint64)));
    connect(ui->centralwidget, SIGNAL(extractAllRequested()), this, SLOT(on_actionExtract_triggered()));
    connect(ui->centralwidget, SIGNAL(testRequested()), this, SLOT(on_actionTest_triggered()));

    g_pRecentFilesMenu = g_xOptions.createRecentFilesMenu(this);
    ui->menuFile->insertMenu(ui->actionExit, g_pRecentFilesMenu);
    ui->menuFile->insertSeparator(ui->actionExit);
    updateRecentFilesMenu();

    adjustView();
    updateActions();
    updateStatusBar();

    if (QCoreApplication::arguments().count() > 1) {
        openFile(QCoreApplication::arguments().at(1));
    }
}

GuiMainWindow::~GuiMainWindow()
{
    g_xOptions.save();

    delete ui;
}

void GuiMainWindow::openFile(const QString &sFileName)
{
    QFileInfo fi(sFileName);

    if (!fi.exists() || !fi.isFile()) {
        return;
    }

    std::unique_ptr<QFile> pArchiveFile(new QFile(fi.absoluteFilePath()));

    if (!pArchiveFile->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file") + QStringLiteral(": ") + QDir::toNativeSeparators(fi.absoluteFilePath()));
        return;
    }

    XBinary::FT fileType = XFormats::getPrefFileType(pArchiveFile.get(), XBinary::FT_FLAG_ARCHIVES);

    g_pArchiveFile.swap(pArchiveFile);
    g_sCurrentFileName = fi.absoluteFilePath();
    g_sCurrentRecordFileName.clear();
    g_nCurrentRecordFileSize = 0;

    ui->centralwidget->setData(fileType, g_pArchiveFile.get());

    setWindowTitle(XOptions::getTitle(X_APPLICATIONDISPLAYNAME, X_APPLICATIONVERSION) + QStringLiteral(" - ") + fi.fileName());

    g_xOptions.setLastFileName(fi.absoluteFilePath());
    updateRecentFilesMenu();

    updateActions();
    updateStatusBar();
}

void GuiMainWindow::onArchiveRecordsLoaded(qint32 nNumberOfRecords)
{
    Q_UNUSED(nNumberOfRecords)

    updateActions();
    updateStatusBar();
}

void GuiMainWindow::onArchiveRecordChanged(const QString &sRecordFileName, qint64 nFileSize)
{
    g_sCurrentRecordFileName = sRecordFileName;
    g_nCurrentRecordFileSize = nFileSize;

    updateStatusBar();
}

void GuiMainWindow::on_actionOpen_triggered()
{
    QString sDirectory = g_xOptions.getLastDirectory();
    QString sFileName = QFileDialog::getOpenFileName(this, tr("Open file") + QStringLiteral("..."), sDirectory, tr("All files") + QStringLiteral(" (*)"));

    if (!sFileName.isEmpty()) {
        openFile(sFileName);
    }
}

void GuiMainWindow::on_actionExtract_triggered()
{
    QString sFileName = getCurrentFileName();

    if (sFileName.isEmpty() || (!XFormats::isArchive(sFileName))) {
        return;
    }

    QString sResultFolder = QFileDialog::getExistingDirectory(this, tr("Extract to directory"), QFileInfo(sFileName).absolutePath());

    if (sResultFolder.isEmpty()) {
        return;
    }

    if (extractArchive(sFileName, sResultFolder)) {
        QMessageBox::information(this, tr("Extract"), tr("Extracted to %1").arg(QDir::toNativeSeparators(sResultFolder)));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Cannot extract archive"));
    }
}

void GuiMainWindow::on_actionTest_triggered()
{
    QString sFileName = getCurrentFileName();

    if (sFileName.isEmpty() || (!XFormats::isArchive(sFileName))) {
        return;
    }

    QTemporaryDir temporaryDir;

    if (!temporaryDir.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot create temporary directory"));
        return;
    }

    if (extractArchive(sFileName, temporaryDir.path())) {
        QMessageBox::information(this, tr("Test"), tr("There are no errors"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Archive test failed"));
    }
}

void GuiMainWindow::on_actionInfo_triggered()
{
    QString sFileName = getCurrentFileName();

    if (sFileName.isEmpty()) {
        return;
    }

    QFile file(sFileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file") + QStringLiteral(": ") + QDir::toNativeSeparators(sFileName));
        return;
    }

    DialogXFileInfo dialogXFileInfo(this);
    dialogXFileInfo.setGlobal(&g_xShortcuts, &g_xOptions);
    dialogXFileInfo.setData(&file, XFormats::getPrefFileType(&file, XBinary::FT_FLAG_FORMATS), QStringLiteral("Info"), true);

    dialogXFileInfo.exec();
}

void GuiMainWindow::on_actionRefresh_triggered()
{
    ui->centralwidget->reloadData(true);

    updateActions();
    updateStatusBar();
}

void GuiMainWindow::on_actionAbout_triggered()
{
    DialogAbout di(this);
    di.exec();
}

void GuiMainWindow::on_actionOptions_triggered()
{
    DialogOptions dialogOptions(this, &g_xOptions, XOptions::GROUPID_SCAN);
    dialogOptions.setGlobal(&g_xShortcuts, &g_xOptions);

    dialogOptions.exec();

    adjustView();
}

void GuiMainWindow::on_actionExit_triggered()
{
    this->close();
}

void GuiMainWindow::adjustView()
{
    if (g_xOptions.isIDPresent(XOptions::ID_VIEW_STAYONTOP)) {
        g_xOptions.adjustStayOnTop(this);
    }

    g_xOptions.adjustWidget(this, XOptions::ID_VIEW_FONT_CONTROLS);

    ui->centralwidget->adjustView();
}

void GuiMainWindow::dragEnterEvent(QDragEnterEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

void GuiMainWindow::dragMoveEvent(QDragMoveEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

void GuiMainWindow::dropEvent(QDropEvent *pEvent)
{
    const QMimeData *mimeData = pEvent->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        if (urlList.count()) {
            QString sFileName = urlList.at(0).toLocalFile();
            openFile(sFileName);
        }
    }
}

void GuiMainWindow::updateRecentFilesMenu()
{
    if (g_pRecentFilesMenu) {
        g_pRecentFilesMenu->setEnabled(g_xOptions.getRecentFiles().count());
    }
}

void GuiMainWindow::updateActions()
{
    bool bIsArchive = g_pArchiveFile && g_pArchiveFile->isOpen() && !g_sCurrentFileName.isEmpty();

    ui->actionExtract->setEnabled(bIsArchive);
    ui->actionTest->setEnabled(bIsArchive);
    ui->actionInfo->setEnabled(bIsArchive);
}

void GuiMainWindow::updateStatusBar()
{
    const QList<XBinary::ARCHIVERECORD> *pArchiveRecords = ui->centralwidget->getArchiveRecords();
    qint32 nNumberOfRecords = pArchiveRecords ? pArchiveRecords->count() : 0;

    if (g_pArchiveFile && g_pArchiveFile->isOpen()) {
        g_pLabelObjects->setText(QString("%1 %2").arg(QString::number(nNumberOfRecords), tr("record(s)")));
    } else {
        g_pLabelObjects->setText(tr("No archive loaded"));
    }

    QString sInfo;

    if (!g_sCurrentRecordFileName.isEmpty()) {
        sInfo = QString("%1  %2").arg(g_sCurrentRecordFileName, XBinary::bytesCountToString(g_nCurrentRecordFileSize, 1024));
    } else if (g_pArchiveFile && g_pArchiveFile->isOpen()) {
        sInfo = QString("%1  %2").arg(QDir::toNativeSeparators(g_sCurrentFileName), XBinary::bytesCountToString(g_pArchiveFile->size(), 1024));
    }

    g_pLabelCurrentFile->setText(sInfo);
    g_pLabelCurrentFile->setToolTip(sInfo);
}

QString GuiMainWindow::getCurrentFileName() const
{
    if (g_pArchiveFile && g_pArchiveFile->isOpen()) {
        return g_sCurrentFileName;
    }

    return QString();
}

bool GuiMainWindow::extractArchive(const QString &sFileName, const QString &sResultFolder)
{
    DialogUnpackFile dialogUnpackFile(this);
    dialogUnpackFile.setData(sFileName, sResultFolder);
    dialogUnpackFile.showDialogDelay();

    return dialogUnpackFile.isSuccess();
}
