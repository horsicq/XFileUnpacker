/* Copyright (c) 2025 hors<horsicq@gmail.com>
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

    // Add positional argument for input file
    parser.addPositionalArgument("file", "File to unpack");

    // Process command line arguments
    parser.process(app);

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

    // Cast to archive if it's an archive format
    XArchive *pArchive = dynamic_cast<XArchive *>(pBinary);

    if (!pArchive) {
        printError(QString("Not an archive format: %1").arg(sFileTypeString));
        delete pBinary;
        file.close();
        return 1;
    }

    // Get archive records
    XBinary::PDSTRUCT pdStruct = XBinary::createPdStruct();
    QList<XBinary::ARCHIVERECORD> listRecords = XFormats::getArchiveRecords(fileType, &file, -1, false, -1, &pdStruct);

    printInfo(QString("Number of records: %1").arg(listRecords.count()));

    if (listRecords.isEmpty()) {
        printError("Archive contains no records");
        delete pArchive;
        file.close();
        return 1;
    }

    if (bListOnly) {
        printInfo("Archive contents:");
        printInfo(QString("%-50s %15s %15s %10s").arg("Name").arg("Compressed").arg("Uncompressed").arg("CRC32"));
        printInfo(QString("-").repeated(90));

        for (qint32 i = 0; i < listRecords.count(); i++) {
            XBinary::ARCHIVERECORD record = listRecords.at(i);
            QString sName = record.mapProperties.value(XBinary::FPART_PROP_ORIGINALNAME).toString();
            if (sName.isEmpty()) sName = QString("record_%1").arg(i);
            
            qint64 nCompressedSize = record.mapProperties.value(XBinary::FPART_PROP_COMPRESSEDSIZE).toLongLong();
            if (nCompressedSize == 0) nCompressedSize = record.nStreamSize;
            qint64 nUncompressedSize = record.mapProperties.value(XBinary::FPART_PROP_UNCOMPRESSEDSIZE).toLongLong();
            quint32 nCRC32 = record.mapProperties.value(XBinary::FPART_PROP_CRC_VALUE, 0).toUInt();
            QString sCRC32 = nCRC32 != 0 ? QString::number(nCRC32, 16).toUpper() : QString("-");

            printInfo(QString("%-50s %15lld %15lld %10s")
                      .arg(sName)
                      .arg(nCompressedSize)
                      .arg(nUncompressedSize)
                      .arg(sCRC32));
        }

        printInfo(QString("-").repeated(90));
        printInfo(QString("Total: %1 file(s)").arg(listRecords.count()));
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
            delete pArchive;
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
