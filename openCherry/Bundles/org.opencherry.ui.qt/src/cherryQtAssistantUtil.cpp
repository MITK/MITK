/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryQtAssistantUtil.h"

#include <cherryConfig.h>
#include <cherryLog.h>
#include <cherryIBundleStorage.h>

#include <QFileInfo>
#include <QProgressDialog>

namespace cherry
{

QProcess* QtAssistantUtil::assistantProcess = 0;

void QtAssistantUtil::OpenAssistant(const QString& collectionFile, const QString& startPage)
{
  QFileInfo assistantFile(QT_ASSISTANT_EXECUTABLE);
  QString assistantExec = assistantFile.fileName();
  QProcess* process = new QProcess;
  process->start(assistantExec, NULL);
}

bool QtAssistantUtil::RegisterQCHFiles(const QString& collectionFile,
    const std::vector<IBundle::Pointer>& bundles)
{
  QFileInfo assistantFile(QT_ASSISTANT_EXECUTABLE);
  QString assistantExec = assistantFile.fileName();

  QList<QStringList> argsVector;

  for (std::size_t i = 0; i < bundles.size(); ++i)
  {
    std::vector<std::string> resourceFiles;
    bundles[i]->GetStorage().List("resources", resourceFiles);
    for (std::size_t j = 0; j < resourceFiles.size(); ++j)
    {
      QString resource = QString::fromStdString(resourceFiles[j]);
      if (resource.endsWith(".qch"))
      {
        QStringList args;
        args << QLatin1String("-collectionFile") << collectionFile;
        Poco::Path qchPath = bundles[i]->GetPath();
        qchPath.pushDirectory("resources");
        qchPath.setFileName(resourceFiles[j]);
        args << QLatin1String("-register") << QString::fromStdString(qchPath.toString());
        argsVector.push_back(args);
      }
    }
  }

  bool success = true;
  QProgressDialog progress("Registering help files...", "Abort Registration", 0, argsVector.size());
  progress.setWindowModality(Qt::WindowModal);

  for (std::size_t i = 0; i < argsVector.size(); ++i)
  {
    const QStringList& args = argsVector[i];
    progress.setValue(i);
    QString labelText = QString("Registering ") + args[3];
    progress.setLabelText(labelText);

    if (progress.wasCanceled())
    {
      success = false;
      break;
    }

    QProcess* process = new QProcess;
    process->start(assistantExec, args);
    if (!process->waitForStarted())
    {
      success = false;
      CHERRY_ERROR << "Registering compressed help file" << args[3].toStdString() << " failed";
    }
  }
  progress.setValue(argsVector.size());

  return success;
}

}
