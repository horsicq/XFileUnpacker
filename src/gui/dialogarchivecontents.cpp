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
#include "dialogarchivecontents.h"

#include "ui_dialogarchivecontents.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSet>

#include "dialogunpackfile.h"
#include "xformats.h"

DialogArchiveContents::DialogArchiveContents(QWidget *pParent) : XShortcutsDialog(pParent, true), ui(new Ui::DialogArchiveContents), g_pXOptions(nullptr)
{
    ui->setupUi(this);
    g_fileType = XBinary::FT_UNKNOWN;

    connect(ui->widgetArchive, SIGNAL(extractAllRequested()), this, SLOT(onExtractAllRequested()));
    connect(ui->widgetArchive, SIGNAL(testRequested()), this, SLOT(onTestRequested()));
    connect(ui->widgetArchive, SIGNAL(recordsLoaded(qint32)), this, SLOT(onArchiveRecordsLoaded(qint32)));
}

DialogArchiveContents::~DialogArchiveContents()
{
    delete ui;
}

void DialogArchiveContents::setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions)
{
    g_pXOptions = pXOptions;
    ui->widgetArchive->setGlobal(pShortcuts, pXOptions);
}

bool DialogArchiveContents::setFileName(const QString &sFileName)
{
    g_sFileName = sFileName;
    g_file.setFileName(sFileName);

    if (!g_file.open(QIODevice::ReadOnly)) {
        return false;
    }

    g_fileType = XFormats::getPrefFileType(&g_file, XBinary::FT_FLAG_ARCHIVES);

    ui->widgetArchive->setData(g_fileType, &g_file);

    setWindowTitle(QFileInfo(sFileName).fileName());

    updateSummary(g_fileType);

    return true;
}

void DialogArchiveContents::onArchiveRecordsLoaded(qint32 nNumberOfRecords)
{
    Q_UNUSED(nNumberOfRecords)

    if (g_file.isOpen()) {
        updateSummary(g_fileType);
    }
}

void DialogArchiveContents::updateSummary(XBinary::FT fileType)
{
    XBinary::FILEFORMATINFO fileFormatInfo = XFormats::getFileFormatInfo(fileType, &g_file);

    // the explorer widget has already listed the records with the streaming
    // unpack API; reuse them instead of parsing the archive a second time
    const QList<XBinary::ARCHIVERECORD> *pListRecords = ui->widgetArchive->getArchiveRecords();
    qint32 nNumberOfRecords = pListRecords->count();

    qint64 nTotalSize = 0;
    qint64 nTotalPacked = 0;
    bool bPackedSizeKnown = true;

    // solid formats report the same shared compressed stream on every member of a
    // block, so count each distinct stream region once (avoids a bogus >100% ratio)
    QSet<QString> stSeenStreams;

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const XBinary::ARCHIVERECORD &record = pListRecords->at(i);

        nTotalSize += record.mapProperties.value(XBinary::FPART_PROP_UNCOMPRESSEDSIZE).toLongLong();

        qint64 nPacked = 0;
        bool bRecordPackedSizeKnown = true;

        if (record.mapProperties.contains(XBinary::FPART_PROP_COMPRESSEDSIZE)) {
            nPacked = record.mapProperties.value(XBinary::FPART_PROP_COMPRESSEDSIZE).toLongLong();
        } else if (record.nStreamSize > 0) {
            nPacked = record.nStreamSize;
        } else {
            bRecordPackedSizeKnown = false;
        }

        bool bIsFolder = record.mapProperties.value(XBinary::FPART_PROP_ISFOLDER).toBool();

        if (!bIsFolder && !bRecordPackedSizeKnown) {
            bPackedSizeKnown = false;
            continue;
        }

        if ((record.nStreamSize > 0) && (!bIsFolder)) {
            QString sStreamKey = QString("%1:%2").arg(record.nStreamOffset).arg(record.nStreamSize);

            if (!stSeenStreams.contains(sStreamKey)) {
                stSeenStreams.insert(sStreamKey);
                nTotalPacked += nPacked;
            }
        } else {
            nTotalPacked += nPacked;
        }
    }

    QString sSummary = XBinary::fileTypeIdToString(fileType);
    QString sFormatInfo = XBinary::getFileFormatInfoString(&fileFormatInfo);

    if (!sFormatInfo.isEmpty()) {
        sSummary += QString(": %1").arg(sFormatInfo);
    }

    sSummary += QString(" | %1: %2").arg(tr("Records"), QString::number(nNumberOfRecords));
    sSummary += QString(" | %1: %2").arg(tr("Size"), XBinary::bytesCountToString(nTotalSize, 1024));
    sSummary += QString(" | %1: %2").arg(
        tr("Packed size"),
        bPackedSizeKnown ? XBinary::bytesCountToString(nTotalPacked, 1024) : tr("Unknown"));

    if (bPackedSizeKnown && (nTotalSize > 0)) {
        double dRatio = (double(nTotalPacked) * 100.0) / double(nTotalSize);
        sSummary += QString(" | %1: %2%").arg(tr("Ratio"), QString::number(dRatio, 'f', 1));
    }

    sSummary += QString(" | %1: %2").arg(tr("Physical size"), XBinary::bytesCountToString(g_file.size(), 1024));

    ui->labelSummary->setText(sSummary);
}

void DialogArchiveContents::onExtractAllRequested()
{
    QString sDirectory = QFileInfo(g_sFileName).absolutePath();

    if (g_pXOptions) {
        QString sLastDirectory = g_pXOptions->getLastDirectory();

        if (!sLastDirectory.isEmpty()) {
            sDirectory = sLastDirectory;
        }
    }

    QString sResultFolder = QFileDialog::getExistingDirectory(this, tr("Extract to directory"), sDirectory);

    if (sResultFolder.isEmpty()) {
        return;
    }

    DialogUnpackFile dialogUnpackFile(this);
    QMap<XBinary::UNPACK_PROP, QVariant> mapProperties;
    const QString sPassword = ui->widgetArchive->getPassword();
    if (!sPassword.isEmpty()) {
        mapProperties.insert(XBinary::UNPACK_PROP_PASSWORD, sPassword);
    }
    dialogUnpackFile.setData(g_sFileName, sResultFolder, mapProperties);
    dialogUnpackFile.showDialogDelay();

    if (dialogUnpackFile.isSuccess()) {
        QMessageBox::information(this, tr("Extract"), tr("Extracted to %1").arg(QDir::toNativeSeparators(sResultFolder)));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Cannot extract archive"));
    }
}

void DialogArchiveContents::onTestRequested()
{
    QMap<XBinary::UNPACK_PROP, QVariant> mapProperties;
    const QString sPassword = ui->widgetArchive->getPassword();
    if (!sPassword.isEmpty()) {
        mapProperties.insert(XBinary::UNPACK_PROP_PASSWORD, sPassword);
    }

    DialogUnpackFile dialogUnpackFile(this);
    dialogUnpackFile.setDataTest(g_sFileName, mapProperties);
    dialogUnpackFile.showDialogDelay();

    if (dialogUnpackFile.isSuccess()) {
        qint32 nNumberOfRecords = ui->widgetArchive->getArchiveRecords()->count();
        QMessageBox::information(this, tr("Test"), tr("There are no errors") + QString(" (%1 %2)").arg(QString::number(nNumberOfRecords), tr("record(s)")));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Archive test failed"));
    }
}

void DialogArchiveContents::on_pushButtonClose_clicked()
{
    this->close();
}
