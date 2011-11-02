/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryQtAssistantUtil.h"
#include <berryPlatformUI.h>
#include <berryConfig.h>
#include <berryLog.h>
#include <berryIBundleStorage.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPart.h>

#include <QFileInfo>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

namespace berry
{

QProcess* QtAssistantUtil::assistantProcess = 0;
QString QtAssistantUtil::helpCollectionFile;
QString QtAssistantUtil::defaultHelpUrl;
QSet<QString> QtAssistantUtil::registeredBundles;

void QtAssistantUtil::SetHelpCollectionFile(const QString& file)
{
  helpCollectionFile = file;
}

QString QtAssistantUtil::GetHelpCollectionFile()
{
  return helpCollectionFile;
}

void QtAssistantUtil::OpenActivePartHelp()
{
  //Get Plugin-ID
  QString pluginID;
  berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
  if (currentWorkbench) 
  {
    berry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
    if (currentWorkbenchWindow)
    {
      berry::IWorkbenchPage::Pointer currentPage = currentWorkbenchWindow->GetActivePage();
      if (currentPage)
      {
        berry::IWorkbenchPart::Pointer currentPart = currentPage->GetActivePart();
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

bool QtAssistantUtil::RegisterQCHFiles(const std::vector<IBundle::Pointer>& bundles)
{
  QStringList qchFiles = ExtractQCHFiles(bundles);
  // unregister old files
  CallQtAssistant(qchFiles, false);
  return CallQtAssistant(qchFiles, true);
}

bool QtAssistantUtil::RegisterQCHFiles(const QStringList& qchFiles)
{
  return CallQtAssistant(qchFiles, true);
}

bool QtAssistantUtil::UnregisterQCHFiles(const QStringList& qchFiles)
{
  return CallQtAssistant(qchFiles, false);
}

QStringList QtAssistantUtil::ExtractQCHFiles(const std::vector<IBundle::Pointer>& bundles)
{
  QStringList result;

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
        Poco::Path qchPath = bundles[i]->GetPath();
        qchPath.pushDirectory("resources");
        qchPath.setFileName(resourceFiles[j]);
        result << QString::fromStdString(qchPath.toString());
      }
    }

    if (qchFileFound)
    {
      registeredBundles.insert(QString::fromStdString(bundles[i]->GetSymbolicName()));
    }
  }

  return result;
}

bool QtAssistantUtil::CallQtAssistant(const QStringList& qchFiles, bool registerFile)
{
  if (qchFiles.empty()) return true;

  bool success = true;

  QList<QStringList> argsVector;
  foreach (QString qchFile, qchFiles)
  {
    QStringList args;
    if (!helpCollectionFile.isEmpty())
    {
      args << QLatin1String("-collectionFile") << helpCollectionFile;
    }

    if (registerFile)
    {
      args << QLatin1String("-register");
    }
    else
    {
      args << QLatin1String("-unregister");
    }
    args << qchFile;
    // This is necessary on Windows to suppress the pop-up dialogs on registering
    // or unregistering .qch files. Unfortunately, it also suppresses specific
    // error messages.
    args << QLatin1String("-quiet");
    //BERRY_INFO << "Registering " << qchFile.toStdString() << " with " << helpCollectionFile.toStdString();
    argsVector.push_back(args);
  }

  QString progressLabel(registerFile ? "Registering help files..." : "Unregistering help files...");
  QString progressCancel(registerFile ? "Abort Registration" : "Abort Unregistration");
  QProgressDialog progress(progressLabel, progressCancel, 0, argsVector.size());
  progress.setWindowModality(Qt::WindowModal);

  QString assistantExec = GetAssistantExecutable();
  QString errorString;
  int exitCode = 0;
  for (int i = 0; i < argsVector.size(); ++i)
  {
    const QStringList& args = argsVector[i];
    progress.setValue(i);
    QString labelText = (registerFile ? QString("Registering ") : QString("Unregistering ")) + args[3];
    progress.setLabelText(labelText);

    if (progress.wasCanceled())
    {
      success = false;
      break;
    }

    QProcess* process = new QProcess;
    //qDebug() << "***** " << assistantExec << args;
    process->start(assistantExec, args);
    BERRY_INFO << (registerFile ? "Registering " : "Unregistering ")
               << "Qt compressed help file: " << qchFiles[i].toStdString();
    if (!process->waitForFinished())
    {
      success = false;
      if (registerFile)
      {
        BERRY_ERROR << "Registering compressed help file" << args[3].toStdString() << " failed";
      }
      else
      {
        BERRY_ERROR << "Unregistering compressed help file" << args[3].toStdString() << " failed";
      }
      errorString = process->errorString();
    }

    //qDebug() << process->readAll();

    if (process->error() != QProcess::UnknownError)
    {
      errorString = process->errorString();
      success = false;
    }

    // Report errors only if we are registering files. If for example the plugin containing the
    // original .qhc file has been updated, unregistering .qch files may fail but it should
    // not be treated as an error.
    if (process->exitCode() != 0 && registerFile)
    {
      exitCode = process->exitCode();
      errorString = process->readAllStandardError();
    }

    if (success && exitCode == 0)
    {
      // Use a hack to get the plug-in id from the qch path
      QString strId = QFileInfo(qchFiles[i]).dir().dirName();
      if (strId.isEmpty())
      {
        BERRY_ERROR << "Could not get last directory name from: " << qchFiles[i].toStdString();
      }
      else
      {
        bool okay = true;
        long pluginId = strId.toLong(&okay);
        if (okay)
        {
          QSharedPointer<ctkPlugin> plugin = berry::Platform::GetCTKPlugin(pluginId);
          if (plugin)
          {
            if (registerFile)
            {
              registeredBundles.insert(plugin->getSymbolicName());
            }
            else
            {
              registeredBundles.remove(plugin->getSymbolicName());
            }
          }
        }
        else
        {
          BERRY_WARN << "Could convert last directory name into an integer (legacy BlueBerry plug-in?): " << qchFiles[i].toStdString();
        }
      }
    }
  }
  progress.setValue(argsVector.size());

  if (!errorString.isEmpty() || exitCode)
  {
    QString errText;
    if (errorString.isEmpty())
    {
      if (registerFile) errText += "Registering ";
      else errText += "Unregistering ";
      errText += "one or more help files failed.";
      errText += "\nYou may not have write permissions in " + QDir::toNativeSeparators(QDir::homePath());
    }
    else
    {
      errText += errorString;
    }
    QMessageBox::warning(0, "Help System Error", errText);
  }

  return success;
}

QString QtAssistantUtil::GetAssistantExecutable()
{
  QFileInfo assistantFile(QT_ASSISTANT_EXECUTABLE);
  QFileInfo localAssistant(QCoreApplication::applicationDirPath() + "/" + assistantFile.fileName() );
  
  if (localAssistant.isExecutable())
  {  
    return localAssistant.absoluteFilePath();
  } 
  else
  {
    return assistantFile.absoluteFilePath();
  }
}

void QtAssistantUtil::SetDefaultHelpUrl(const QString& defaultUrl)
{
  defaultHelpUrl = defaultUrl;
}

}
