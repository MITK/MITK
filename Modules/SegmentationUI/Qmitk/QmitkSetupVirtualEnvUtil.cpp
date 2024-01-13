/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#include "QmitkSetupVirtualEnvUtil.h"

#include "mitkLog.h"
#include <QStandardPaths>
#include <itkCommand.h>
#include <regex>
#include <QDir>
#include <QApplication>
#include <QProcess>
#include <QTextCodec>

QmitkSetupVirtualEnvUtil::QmitkSetupVirtualEnvUtil()
{
  m_BaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
              qApp->organizationName() + QDir::separator();
}

QmitkSetupVirtualEnvUtil::QmitkSetupVirtualEnvUtil(const QString &baseDir)
{
  m_BaseDir = baseDir;
}

QString& QmitkSetupVirtualEnvUtil::GetBaseDir()
{
  return m_BaseDir;
}

QString QmitkSetupVirtualEnvUtil::GetVirtualEnvPath()
{
  return m_venvPath;
}

QString& QmitkSetupVirtualEnvUtil::GetSystemPythonPath()
{
  return m_SysPythonPath;
}

QString& QmitkSetupVirtualEnvUtil::GetPythonPath()
{
  return m_PythonPath;
}

QString& QmitkSetupVirtualEnvUtil::GetPipPath()
{
  return m_PipPath;
}

void QmitkSetupVirtualEnvUtil::SetVirtualEnvPath(const QString &path)
{
  m_venvPath = path;
}

void QmitkSetupVirtualEnvUtil::SetPipPath(const QString &path)
{
  m_PipPath = path;
}

void QmitkSetupVirtualEnvUtil::SetPythonPath(const QString &path)
{
  if (this->IsPythonPath(path))
  {
    m_PythonPath = path;
  }
  else
  {
    MITK_INFO << "Python was not detected in " + path.toStdString();
  }
}

void QmitkSetupVirtualEnvUtil::SetSystemPythonPath(const QString &path)
{
  if (this->IsPythonPath(path))
  {
    m_SysPythonPath = path;
  }
  else
  {
    MITK_INFO << "Python was not detected in " + path.toStdString();
  }
}

void QmitkSetupVirtualEnvUtil::PrintProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
{
  std::string testCOUT;
  std::string testCERR;
  const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);
  if (pEvent)
  {
    testCOUT = testCOUT + pEvent->GetOutput();
    MITK_INFO << testCOUT;
  }
  const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e);
  if (pErrEvent)
  {
    testCERR = testCERR + pErrEvent->GetOutput();
    MITK_ERROR << testCERR;
  }
}

void QmitkSetupVirtualEnvUtil::InstallPytorch(const std::string &workingDir,
                                              void (*callback)(itk::Object *, const itk::EventObject &, void *))
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(callback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("-m");
  args.push_back("pip");
  args.push_back("install");
  args.push_back("light-the-torch==0.7.5");
  spExec->Execute(workingDir, "python", args);
  PipInstall("torch==2.0.0", workingDir, callback, "ltt");
  PipInstall("torchvision==0.15.0", workingDir, callback, "ltt");
}

void QmitkSetupVirtualEnvUtil::InstallPytorch()
{
  this->InstallPytorch(GetPythonPath().toStdString(), &PrintProcessEvent);
}

void QmitkSetupVirtualEnvUtil::PipInstall(const std::string &library,
                                          const std::string &workingDir,
                                          void (*callback)(itk::Object *, const itk::EventObject &, void *),
                                          const std::string &command)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(callback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("install");
  args.push_back(library);
  spExec->Execute(workingDir, command, args);
}

void QmitkSetupVirtualEnvUtil::PipInstall(const std::string &library,
  void (*callback)(itk::Object*, const itk::EventObject&, void*),
  const std::string& command)
{
  this->PipInstall(library, this->GetPipPath().toStdString(), callback, command);
}

void QmitkSetupVirtualEnvUtil::ExecutePython(const std::string &pythonCode,
                                             const std::string &workingDir,
                                             void (*callback)(itk::Object *, const itk::EventObject &, void *),
                                             const std::string &command)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(callback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("-c");
  args.push_back(pythonCode);
  spExec->Execute(workingDir, command, args);
}

void QmitkSetupVirtualEnvUtil::ExecutePython(const std::string &args,
                                             void (*callback)(itk::Object *, const itk::EventObject &, void *),
                                             const std::string &command)
{
  this->ExecutePython(args, this->GetPythonPath().toStdString(), callback, command);
}

bool QmitkSetupVirtualEnvUtil::IsPythonPath(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  bool isExists =
#ifdef _WIN32
    QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
#else
    QFile::exists(fullPath + QDir::separator() + QString("python3"));
#endif
  return isExists;
}

std::pair<QString, QString> QmitkSetupVirtualEnvUtil::GetExactPythonPath(const QString &pyEnv)
{
  QString fullPath = pyEnv;
  bool pythonDoesExist = false;
  bool isSupportedVersion = false;
#ifdef _WIN32
  const std::string PYTHON_EXE = "python.exe";
  // check if python exist in given folder.
  pythonDoesExist = QFile::exists(fullPath + QDir::separator() + QString::fromStdString(PYTHON_EXE));
  if (!pythonDoesExist && // check if in Scripts already, if not go there
      !(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    pythonDoesExist = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  }
#else
  const std::string PYTHON_EXE = "python3";
  pythonDoesExist = QFile::exists(fullPath + QDir::separator() + QString::fromStdString(PYTHON_EXE));
  if (!pythonDoesExist &&
      !(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    pythonDoesExist = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  }
#endif
  std::pair<QString, QString> pythonPath;
  if (pythonDoesExist)
  {
    std::regex sanitizer(R"(3\.(\d+))");
    QProcess pyProcess;
    pyProcess.start(fullPath + QDir::separator() + QString::fromStdString(PYTHON_EXE),
                          QStringList() << "--version",
                          QProcess::ReadOnly);
    if (pyProcess.waitForFinished())
    {
      auto pyVersionCaptured = QTextCodec::codecForName("UTF-8")->toUnicode(pyProcess.readAllStandardOutput()).toStdString();
      std::smatch match; // Expecting "Python 3.xx.xx" or "Python 3.xx"
      if (std::regex_search(pyVersionCaptured, match, sanitizer) && !match.empty())
      {
        std::string pyVersionNumber = match[0];
        int pySubVersion = std::stoi(match[1]);
        isSupportedVersion = (pySubVersion > 8) ? (pySubVersion < 13) : false;
        pythonPath.second = QString::fromStdString(pyVersionNumber);
      }
    }
  }
  pythonPath.first = pythonDoesExist &&isSupportedVersion ? fullPath : "";
  return pythonPath;
}
