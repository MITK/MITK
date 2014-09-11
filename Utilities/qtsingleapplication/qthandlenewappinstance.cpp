/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "qthandlenewappinstance.h"

#include "qtsingleapplication.h"

#if QT_VERSION < 0x050000

#include <QDir>
#include <stdlib.h> // mkdtemp
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
//#include <private/qfsfileengine_p.h>
#endif

bool createTemporaryDir(QString& path)
{
  QString baseName = QCoreApplication::applicationName();
  if (baseName.isEmpty())
  {
    baseName = QLatin1String("mitk_temp");
  }

  QString templateName = QDir::tempPath() + QLatin1Char('/') + baseName + QLatin1String("-XXXXXX");
  bool success = false;

#ifdef Q_OS_WIN
  QString buffer = templateName;
  // Windows' mktemp believes 26 temp files per process ought to be enough for everyone (!)
  // Let's add a few random chars then, before the XXXXXX template.
  for (int i = 0 ; i < 4 ; ++i)
    buffer += QChar((qrand() & 0xffff) % (26) + 'A');
  if (!buffer.endsWith(QLatin1String("XXXXXX")))
    buffer += QLatin1String("XXXXXX");
  //QFileSystemEntry baseEntry(buffer);
  //QFileSystemEntry::NativePath basePath = baseEntry.nativeFilePath();
  QString basePath = QDir::toNativeSeparators(buffer);
  wchar_t* array = (wchar_t*)basePath.utf16();
  if (_wmktemp(array) && ::CreateDirectoryW(array, 0))
  {
    success = true;
    //QFileSystemEntry entry(QString::fromWCharArray(array), QFileSystemEntry::FromNativePath());
    //path = entry.filePath();
    path = QDir::fromNativeSeparators(QString::fromUtf16((const ushort*)array));
  }
#else
  QByteArray buffer = QFile::encodeName(templateName);
  if (!buffer.endsWith("XXXXXX"))
    buffer += "XXXXXX";
  if (mkdtemp(buffer.data())) { // modifies buffer
    success = true;
    path = QFile::decodeName(buffer.constData());
  }
#endif

  return success;
}

#else

#include <QTemporaryDir>

bool createTemporaryDir(QString& path)
{
  QString baseName = QCoreApplication::applicationName();
  if (baseName.isEmpty())
  {
    baseName = QLatin1String("mitk_temp");
  }

  QString templateName = QDir::tempPath() + QLatin1Char('/') + baseName + QLatin1String("-XXXXXX");
  QTemporaryDir tmpDir(templateName);
  tmpDir.setAutoRemove(false);

  if (tmpDir.isValid())
  {
    path = tmpDir.path();
    return true;
  }
  return false;
}

#endif

QString handleNewAppInstance(QtSingleApplication* singleApp, int argc, char** argv, const QString& newInstanceArg)
{
  if (singleApp->isRunning())
  {
    QStringList args;
    bool newInstance = false;
    for (int i = 0; i < argc; ++i)
    {
      args << argv[i];
      if (args.back().endsWith(newInstanceArg))
      {
        newInstance = true;
      }
    }

    if (newInstance)
    {
      QString path;
      if (!createTemporaryDir(path))
      {
        qCritical("Could not create temporary storage path for new application instance.");
        exit(EXIT_FAILURE);
      }
      qWarning("Forcing new application instance. The application data will be written to a temporary directory.");
      return path;
    }
    else
    {
      QByteArray ba;
      QDataStream msg(&ba, QIODevice::WriteOnly);
      msg << QString("$cmdLineArgs"); // This message contains command line arguments
      msg << args;
      if(singleApp->sendMessage(ba))
      {
        exit(EXIT_SUCCESS);
      }
      else
      {
        qCritical("The running application seems to be frozen.");
        exit(EXIT_FAILURE);
      }
    }
  }
  return QString();
}

