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
#include <QThread>
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

void testProgressCallback()
{
    printInfo("Testing progress callback...");
    
    XBinary::PDSTRUCT pdStruct = XBinary::createPdStruct();
    pdStruct.pCallback = progressCallback;
    pdStruct.pCallbackUserData = nullptr;
    pdStruct.nLastCallbackTime = 0;
    
    // Initialize progress for record 0
    XBinary::setPdStructInit(&pdStruct, 0, 100);
    pdStruct._pdRecord[0].sStatus = "Processing files";
    
    // Simulate progress updates
    for (qint32 i = 0; i <= 100; i += 5) {
        pdStruct._pdRecord[0].nCurrent = i;
        progressCallback(nullptr, &pdStruct);
        
        // Sleep for a short time to make progress visible
        QThread::msleep(100);
    }
    
    QTextStream(stdout) << "\n";
    printInfo("Progress callback test completed");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(Global::NAME);
    QCoreApplication::setApplicationVersion(Global::VERSION);
    QCoreApplication::setOrganizationName(APP_ORGANIZATION);

    QCommandLineParser parser;
    parser.setApplicationDescription(Global::DESCRIPTION);
    parser.addHelpOption();
    parser.addVersionOption();

    // Add command line options
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                    "Output directory for extracted files",
                                    "directory");
    parser.addOption(outputOption);

    QCommandLineOption extractOption(QStringList() << "x" << "extract",
                                     "Extract/unpack archive (default action)");
    parser.addOption(extractOption);

    QCommandLineOption listOption(QStringList() << "l" << "list",
                                  "List archive contents without extracting");
    parser.addOption(listOption);

    QCommandLineOption testOption(QStringList() << "t" << "test",
                                  "Test archive integrity by extracting to temporary location");
    parser.addOption(testOption);

    QCommandLineOption testProgressOption(QStringList() << "test-progress",
                                         "Test progress callback display");
    parser.addOption(testProgressOption);

    // Add positional argument for input file
    parser.addPositionalArgument("file", "File to unpack");

    // Process command line arguments
    parser.process(app);

    // Check for test progress option first (doesn't need a file)
    if (parser.isSet(testProgressOption)) {
        testProgressCallback();
        return 0;
    }

    const QStringList listArgs = parser.positionalArguments();
    if (listArgs.isEmpty()) {
        printError("No input file specified");
        parser.showHelp(1);
        return 1;
    }

    QString sInputFile = listArgs.first();
    QFileInfo fileInfo(sInputFile);

    if (!fileInfo.exists()) {
        printError(QString("File not found: %1").arg(sInputFile));
        return 1;
    }

    if (!fileInfo.isFile()) {
        printError(QString("Not a file: %1").arg(sInputFile));
        return 1;
    }

    bool bListOnly = parser.isSet(listOption);
    bool bTest = parser.isSet(testOption);
    bool bExtract = parser.isSet(extractOption);
    QString sOutputDir = parser.value(outputOption);

    // Default action is extract if no action specified
    if (!bListOnly && !bTest && !bExtract) {
        bExtract = true;
    }

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

    if (bListOnly) {
        XModel_ArchiveRecords model(pBinary->getAvailableFPARTProperties(), &listRecords);
        XOptions::printModel(&model);
    } else if (bTest) {
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
    } else if (bExtract) {
        if (sOutputDir.isEmpty()) {
            sOutputDir = fileInfo.absolutePath();
        }

        printInfo(QString("Extracting to: %1").arg(sOutputDir));

        QString sExtractPath = sOutputDir + QDir::separator() + fileInfo.completeBaseName();
        QDir().mkpath(sExtractPath);

        printInfo(QString("Unpacking archive..."));

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
