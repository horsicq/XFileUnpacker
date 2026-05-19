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

#include "ui_guimainwindow.h"

#include <QFileInfo>

GuiMainWindow::GuiMainWindow(QWidget *pParent) : QMainWindow(pParent), ui(new Ui::GuiMainWindow), g_pRecentFilesMenu(nullptr)
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
    g_xOptions.addID(XOptions::ID_VIEW_COLUMNS, QStringLiteral("0 | 1 | 2"));
    g_xOptions.addID(XOptions::ID_FILE_SAVELASTDIRECTORY, true);
    g_xOptions.addID(XOptions::ID_FILE_SAVERECENTFILES, true);
    g_xOptions.load();

    ui->centralwidget->setGlobal(&g_xShortcuts, &g_xOptions);

    connect(&g_xOptions, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
    connect(ui->centralwidget, SIGNAL(fileActivated(QString)), this, SLOT(openFile(QString)));
    connect(ui->centralwidget, SIGNAL(directoryActivated(QString)), this, SLOT(onDirectoryActivated(QString)));

    g_pRecentFilesMenu = g_xOptions.createRecentFilesMenu(this);
    ui->menuFile->insertMenu(ui->actionExit, g_pRecentFilesMenu);
    ui->menuFile->insertSeparator(ui->actionExit);
    updateRecentFilesMenu();

    QString sLastDirectory = g_xOptions.getLastDirectory();
    if (!sLastDirectory.isEmpty()) {
        ui->centralwidget->setRootPath(sLastDirectory);
    }

    adjustView();

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

    if (!fi.exists()) {
        return;
    }

    if (fi.isDir()) {
        onDirectoryActivated(fi.absoluteFilePath());
        return;
    }

    if (!fi.isFile()) {
        return;
    }

    ui->centralwidget->setCurrentPath(fi.absoluteFilePath());
    setWindowTitle(XOptions::getTitle(X_APPLICATIONDISPLAYNAME, X_APPLICATIONVERSION) +
                   QStringLiteral(" - ") + fi.fileName());

    g_xOptions.setLastFileName(fi.absoluteFilePath());
    updateRecentFilesMenu();
}

void GuiMainWindow::onDirectoryActivated(const QString &sDirectoryName)
{
    QFileInfo fi(sDirectoryName);

    if (fi.isDir()) {
        ui->centralwidget->setRootPath(fi.absoluteFilePath());
        g_xOptions.setLastDirectory(fi.absoluteFilePath());
        setWindowTitle(XOptions::getTitle(X_APPLICATIONDISPLAYNAME, X_APPLICATIONVERSION) +
                       QStringLiteral(" - ") + fi.fileName());
    }
}

void GuiMainWindow::on_actionOpen_triggered()
{
    QString sDirectory = g_xOptions.getLastDirectory();
    QString sFileName = QFileDialog::getOpenFileName(this, tr("Open file") + QStringLiteral("..."), sDirectory, tr("All files") + QStringLiteral(" (*)"));

    if (!sFileName.isEmpty()) {
        openFile(sFileName);
    }
}

void GuiMainWindow::on_actionAbout_triggered()
{
    DialogAbout di(this);
    di.exec();
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
