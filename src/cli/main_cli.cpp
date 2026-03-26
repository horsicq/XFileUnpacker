/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include "../global.h"
#include "xformats.h"
#include "xarchives.h"
#include "xoptions.h"
#include "xmodel_archiverecords.h"

void printError(const QString &sMessage)
{
    QTextStream stream(stderr);
    stream << "Error: " << sMessage << Qt::endl;
}

void printInfo(const QString &sMessage)
{
    QTextStream stream(stdout);
    stream << sMessage << Qt::endl;
}

void progressCallback(void *pUserData, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pUserData)
    
    if (pPdStruct) {
        qint64 nTotalCurrent = 0;
        qint64 nTotalAll = 0;
        QStringList listStatuses;
        
        // Sum all valid records
        for (qint32 i = 0; i < XBinary::N_NUMBER_PDRECORDS; i++) {
            if (pPdStruct->_pdRecord[i].bIsValid) {
                nTotalCurrent += pPdStruct->_pdRecord[i].nCurrent;
                nTotalAll += pPdStruct->_pdRecord[i].nTotal;
                
                QString sStatus = pPdStruct->_pdRecord[i].sStatus;
                if (!sStatus.isEmpty()) {
                    listStatuses.append(sStatus);
                }
            }
        }
        
        if (nTotalAll > 0) {
            qint32 nPercent = (nTotalCurrent * 100) / nTotalAll;
            QString sStatus = listStatuses.join("/");
            
            QTextStream stream(stdout);
            stream << QString("\rProgress: %1% (%2/%3)")
                      .arg(nPercent)
                      .arg(nTotalCurrent)
                      .arg(nTotalAll);
            
            if (!sStatus.isEmpty()) {
                stream << " - " << sStatus;
            }
            
            stream.flush();
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(Global::NAME);
    QCoreApplication::setApplicationVersion(Global::VERSION);
    QCoreApplication::setOrganizationName(APP_ORGANIZATION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QString("%1\n\n"
                "Commands:\n"
                "  e  Extract files from archive (without directory names)\n"
                "  l  List contents of archive\n"
                "  t  Test integrity of archive\n"
                "  x  eXtract files with full paths")
            .arg(Global::DESCRIPTION));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption outputOption(QStringList() << "o",
                                    "Set output directory.",
                                    "directory");
    parser.addOption(outputOption);

    QCommandLineOption passwordOption(QStringList() << "p",
                                      "Set password.",
                                      "password");
    parser.addOption(passwordOption);

    QCommandLineOption yesOption(QStringList() << "y",
                                 "Assume Yes on all queries.");
    parser.addOption(yesOption);

    parser.addPositionalArgument("command", "Command to execute: e, l, t, x");
    parser.addPositionalArgument("archive", "Archive file to process");

    parser.process(app);

    const QStringList listArgs = parser.positionalArguments();
    if (listArgs.size() < 2) {
        printError(QString("Usage: %1 <command> <archive> [switches]").arg(Global::NAME));
        parser.showHelp(1);
        return 1;
    }

    const QString sCommand = listArgs.at(0);
    const QString sInputFile = listArgs.at(1);

    const QStringList validCommands = {"e", "l", "t", "x"};
    if (!validCommands.contains(sCommand)) {
        printError(QString("Unknown command: %1").arg(sCommand));
        parser.showHelp(1);
        return 1;
    }

    QFileInfo fileInfo(sInputFile);

    if (!fileInfo.exists()) {
        printError(QString("File not found: %1").arg(sInputFile));
        return 1;
    }

    if (!fileInfo.isFile()) {
        printError(QString("Not a file: %1").arg(sInputFile));
        return 1;
    }

    QString sOutputDir = parser.value(outputOption);

    printInfo(QString("Processing file: %1").arg(fileInfo.absoluteFilePath()));
    printInfo(QString("File size: %1 bytes").arg(fileInfo.size()));

    QFile file(sInputFile);
    if (!file.open(QIODevice::ReadOnly)) {
        printError(QString("Cannot open file: %1").arg(sInputFile));
        return 1;
    }

    // Detect file type
    QSet<XBinary::FT> stFileTypes = XFormats::getFileTypes(&file, true);
    XBinary::FT fileType = XBinary::_getPrefFileType(&stFileTypes);
    
    QString sFileTypeString = XBinary::fileTypeIdToString(fileType);

    printInfo(QString("File type: %1").arg(sFileTypeString));

    // Get binary object for validation
    XBinary *pBinary = XFormats::getClass(fileType, &file);

    if (!pBinary) {
        printError(QString("Not an archive or unsupported format: %1").arg(sFileTypeString));
        file.close();
        return 1;
    }

    // Get archive records
    XBinary::PDSTRUCT pdStruct = XBinary::createPdStruct();
    pdStruct.pCallback = progressCallback;
    pdStruct.pCallbackUserData = nullptr;
    pdStruct.nLastCallbackTime = 0;
    
    QList<XBinary::ARCHIVERECORD> listRecords = XFormats::getArchiveRecords(fileType, &file, -1, false, -1, &pdStruct);

    QTextStream(stdout) << "\n";  // Clear progress line
    printInfo(QString("Number of records: %1").arg(listRecords.count()));

    if (listRecords.isEmpty()) {
        printError("Archive contains no records");
        delete pBinary;
        file.close();
        return 1;
    }

    if (sCommand == "l") {
        XModel_ArchiveRecords model(pBinary->getAvailableFPARTProperties(), &listRecords);
        XOptions::printModel(&model);
    } else if (sCommand == "t") {
        printInfo("Testing archive integrity...");

        QString sTempDir = QDir::tempPath() + QDir::separator() + "xfileunpacker_test_" + QString::number(QDateTime::currentMSecsSinceEpoch());
        QDir().mkpath(sTempDir);

        printInfo(QString("Extracting to temporary location: %1").arg(sTempDir));

        XFormats xformats;
        bool bTestResult = xformats.unpackDeviceToFolder(fileType, &file, sTempDir, &pdStruct);

        qint32 nExtractedFiles = 0;
        if (bTestResult) {
            QDir testDir(sTempDir);
            QFileInfoList listFiles = testDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
            nExtractedFiles = listFiles.count();
        }

        QDir(sTempDir).removeRecursively();

        if (bTestResult && nExtractedFiles > 0) {
            printInfo(QString("Test PASSED: Successfully extracted %1 file(s)").arg(nExtractedFiles));
        } else {
            printError("Test FAILED: Could not extract archive");
            delete pBinary;
            file.close();
            return 1;
        }
    } else if (sCommand == "e" || sCommand == "x") {
        if (sOutputDir.isEmpty()) {
            sOutputDir = fileInfo.absolutePath();
        }

        // 'e' extracts flat (no subdirectory per archive), 'x' preserves full paths
        QString sExtractPath = (sCommand == "x")
                               ? sOutputDir + QDir::separator() + fileInfo.completeBaseName()
                               : sOutputDir;

        QDir().mkpath(sExtractPath);

        printInfo(QString("Extracting to: %1").arg(sExtractPath));

        XFormats xformats;
        bool bUnpackResult = xformats.unpackDeviceToFolder(fileType, &file, sExtractPath, &pdStruct);

        if (bUnpackResult) {
            printInfo(QString("\nExtracted %1 file(s) successfully").arg(listRecords.count()));
        } else {
            printError("Failed to extract archive");
            delete pBinary;
            file.close();
            return 1;
        }
    }

    delete pBinary;
    file.close();

    printInfo("Operation completed successfully");

    return 0;
}
