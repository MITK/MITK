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
#include <cherryPlatformUI.h>
#include <cherryConfig.h>
#include <cherryLog.h>
#include <cherryIBundleStorage.h>
#include <cherryIWorkbench.h>
#include <cherryIWorkbenchPage.h>
#include <cherryIWorkbenchPart.h>

#include <QFileInfo>
#include <QProgressDialog>

namespace cherry
{

QProcess* QtAssistantUtil::assistantProcess = 0;
QString QtAssistantUtil::helpCollectionFile;
QString QtAssistantUtil::defaultHelpUrl;
QStringList QtAssistantUtil::registeredBundles;

void QtAssistantUtil::SetHelpColletionFile(const QString& file)
{
  helpCollectionFile = file;
}

void QtAssistantUtil::OpenActivePartHelp()
{
  //Get Plugin-ID
  QString pluginID;
  cherry::IWorkbench* currentWorkbench = cherry::PlatformUI::GetWorkbench();
  if (currentWorkbench) 
  {
    cherry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
    if (currentWorkbenchWindow)
    {
      cherry::IWorkbenchPage::Pointer currentPage = currentWorkbenchWindow->GetActivePage();
      if (currentPage)
      {
        cherry::IWorkbenchPart::Pointer currentPart = currentPage->GetActivePart();
        if (currentPart) pluginID = QString::fromStdString(currentPart->GetSite()->GetPluginId());
      }
    }
  }
  //End get Plugin-ID

  QString helpUrl = defaultHelpUrl;
  if (!pluginID.isEmpty() && registeredBundles.contains(pluginID))
    helpUrl = "qthelp://"+pluginID+"/bundle/index.html";

  OpenAssistant(helpUrl);
}

void QtAssistantUtil::OpenAssistant(const QString& startPage)
{
  QString startUrl = startPage;
  if (startUrl.isEmpty()) startUrl = defaultHelpUrl;

  if (assistantProcess == 0)
  {
    assistantProcess = new QProcess;
  }

  if (assistantProcess->state() == QProcess::NotRunning)
  {
    QStringList assistantArgs;
    if (!helpCollectionFile.isEmpty())
    {
      assistantArgs << QLatin1String("-collectionFile") 
                     << QLatin1String(helpCollectionFile.toLatin1());
    }

    assistantArgs << QLatin1String("-enableRemoteControl")
                   << QLatin1String("-showUrl")
                   << QLatin1String(startUrl.toLatin1());
    assistantProcess->start(GetAssistantExecutable(), assistantArgs);
  }
  else
  {
    QByteArray ba;
    ba.append("setSource ").append(startUrl.toLatin1()).append('\0');
    assistantProcess->write(ba);
  }

}

void QtAssistantUtil::CloseAssistant()
{
  if (assistantProcess && (assistantProcess->state() != QProcess::NotRunning))
  {
    assistantProcess->close();
  }
  delete assistantProcess;
}

bool QtAssistantUtil::RegisterQCHFiles(const QString& collectionFile,
    const std::vector<IBundle::Pointer>& bundles)
{
  QString assistantExec = GetAssistantExecutable();

  QList<QStringList> argsVector;

  for (std::size_t i = 0; i < bundles.size(); ++i)
  {
    std::vector<std::string> resourceFiles;
    bundles[i]->GetStorage().List("resources", resourceFiles);
    bool qchFileFound = false;
    for (std::size_t j = 0; j < resourceFiles.size(); ++j)
    {
      QString resource = QString::fromStdString(resourceFiles[j]);
      if (resource.endsWith(".qch"))
      {
        qchFileFound = true;
        QStringList args;
        args << QLatin1String("-collectionFile") << collectionFile;
        Poco::Path qchPath = bundles[i]->GetPath();
        qchPath.pushDirectory("resources");
        qchPath.setFileName(resourceFiles[j]);
        args << QLatin1String("-register") << QString::fromStdString(qchPath.toString());
        args << QLatin1String("-quiet");
        argsVector.push_back(args);
      }
    }

    if (qchFileFound)
    {
      registeredBundles.push_back(QString::fromStdString(bundles[i]->GetSymbolicName()));
    }
  }

  bool success = true;
  QProgressDialog progress("Registering help files...", "Abort Registration", 0, argsVector.size());
  progress.setWindowModality(Qt::WindowModal);

  if (argsVector.isEmpty())
  {
    CHERRY_WARN << "No .qch files found. Help contents will not be available.";
  }

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

QString QtAssistantUtil::GetAssistantExecutable()
{
  QFileInfo assistantFile(QT_ASSISTANT_EXECUTABLE);
  return assistantFile.fileName();
}

void QtAssistantUtil::SetDefaultHelpUrl(const QString& defaultUrl)
{
  defaultHelpUrl = defaultUrl;
}

}