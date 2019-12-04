/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "qthandlenewappinstance.h"

#include "qtsingleapplication.h"

#include <QDataStream>

#include <QTemporaryDir>

bool createTemporaryDir(QString &path)
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

QString handleNewAppInstance(QtSingleApplication *singleApp, int argc, char **argv, const QString &newInstanceArg)
{
  if (singleApp->isRunning())
  {
    QStringList args;
    bool newInstance = false;
    for (int i = 1; i < argc; ++i)
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
      if (singleApp->sendMessage(ba))
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
