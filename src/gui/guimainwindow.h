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
#ifndef GUIMAINWINDOW_H
#define GUIMAINWINDOW_H

#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMimeData>

#include <memory>

#include "../global.h"
#include "dialogabout.h"
#include "xoptions.h"
#include "xshortcuts.h"

namespace Ui {
class GuiMainWindow;
}

class GuiMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GuiMainWindow(QWidget *pParent = nullptr);
    ~GuiMainWindow() override;

private slots:
    void openFile(const QString &sFileName);
    void onArchiveRecordsLoaded(qint32 nNumberOfRecords);
    void onArchiveRecordChanged(const QString &sRecordFileName, qint64 nFileSize);
    void on_actionOpen_triggered();
    void on_actionExtract_triggered();
    void on_actionTest_triggered();
    void on_actionInfo_triggered();
    void on_actionRefresh_triggered();
    void on_actionOptions_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void adjustView();

protected:
    void dragEnterEvent(QDragEnterEvent *pEvent) override;
    void dragMoveEvent(QDragMoveEvent *pEvent) override;
    void dropEvent(QDropEvent *pEvent) override;

private:
    void updateRecentFilesMenu();
    void updateActions();
    void updateStatusBar();
    QString getCurrentFileName() const;
    bool extractArchive(const QString &sFileName, const QString &sResultFolder);

    Ui::GuiMainWindow *ui;
    XOptions g_xOptions;
    XShortcuts g_xShortcuts;
    std::unique_ptr<QFile> g_pArchiveFile;
    QString g_sCurrentFileName;
    QString g_sCurrentRecordFileName;
    qint64 g_nCurrentRecordFileSize;
    QMenu *g_pRecentFilesMenu;
    QLabel *g_pLabelObjects;
    QLabel *g_pLabelCurrentFile;
};

#endif  // GUIMAINWINDOW_H
