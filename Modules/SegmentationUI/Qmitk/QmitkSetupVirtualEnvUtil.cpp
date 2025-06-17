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
#include <QDirIterator>
#include <QApplication>
#include <QProcess>
#include <QStringDecoder>

QmitkSetupVirtualEnvUtil::QmitkSetupVirtualEnvUtil()
{
  m_BaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
              qApp->organizationName() + QDir::separator();
}

bool QmitkSetupVirtualEnvUtil::SetupVirtualEnv(const QString &venvName,
                                               const QStringList &packages,
                                               std::function<bool()> validator,
                                               CallbackType printCallback,
                                               const QString &torchVersion)
{
  if (this->GetSystemPythonPath().isEmpty())
  {
    return false;
  }
  if (!QmitkSetupVirtualEnvUtil::IsVenvInstalled(GetSystemPythonPath()))
  {
    return false;
  }
  QDir folderPath(this->GetBaseDir());
  folderPath.mkdir(venvName);
  if (!folderPath.cd(venvName))
  {
    return false; // Check if directory creation was successful.
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(printCallback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);

  args.push_back("-m");
  args.push_back("venv");
  args.push_back(venvName.toStdString());
#ifdef _WIN32
  QString pythonFile = GetSystemPythonPath() + QDir::separator() + "python.exe";
  QString pythonExeFolder = "Scripts";
#else
  QString pythonFile = GetSystemPythonPath() + QDir::separator() + "python3";
  QString pythonExeFolder = "bin";
#endif
  spExec->Execute(GetBaseDir().toStdString(), pythonFile.toStdString(), args); // Setup local virtual environment
  if (folderPath.cd(pythonExeFolder))
  {
    this->SetPythonPath(folderPath.absolutePath());
    this->SetPipPath(folderPath.absolutePath());
    this->InstallPytorch(printCallback, torchVersion);
    for (auto &package : packages)
    {
      this->PipInstall(package.toStdString(), printCallback);
    }
    return validator();
  }
  return false;
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

void QmitkSetupVirtualEnvUtil::InstallPytorch(const std::string &workingDir, CallbackType callback, const QString &torchVersion)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(callback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("-m");
  args.push_back("pip");
  args.push_back("install");
  args.push_back("light-the-torch==0.8.0");
  spExec->Execute(workingDir, "python", args);
  std::string torchVersionSpec = torchVersion.isEmpty() ? "torch>=2.0.0" : ("torch==" + torchVersion.toStdString());
  PipInstall(torchVersionSpec, workingDir, callback, "ltt");
}

void QmitkSetupVirtualEnvUtil::InstallPytorch(const QString &torchVersion)
{
  this->InstallPytorch(GetPythonPath().toStdString(), &PrintProcessEvent, torchVersion);
}

void QmitkSetupVirtualEnvUtil::InstallPytorch(CallbackType callback, const QString &torchVersion)
{
  this->InstallPytorch(GetPythonPath().toStdString(), callback, torchVersion);
}

bool QmitkSetupVirtualEnvUtil::IsVenvInstalled(const QString &pythonPath)
{
  bool isVenvInstalled = false;
  QProcess pyProcess;
#ifdef _WIN32
  const QString PYTHON_EXE = "python.exe";
#else
  const QString PYTHON_EXE = "python3";
#endif
  pyProcess.start(pythonPath + QDir::separator() + PYTHON_EXE,
                  QStringList() << "-m" << "venv", QProcess::ReadOnly); //insufficient args to provoke stderr out
  if (pyProcess.waitForFinished())
  {
    auto venvCaptured = QString(QStringDecoder(QStringDecoder::Utf8)(pyProcess.readAllStandardError()));
    isVenvInstalled = venvCaptured.startsWith(QString("usage")); // if venv found, shows correct usage instructions
  }
  return isVenvInstalled;
}

void QmitkSetupVirtualEnvUtil::PipInstall(const std::string &library,
                                          const std::string &workingDir,
                                          CallbackType callback,
                                          const std::string &command)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(callback);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("install");
  args.push_back(library);
  if (command == "ltt")
    args.push_back("torchvision"); // done together for exact version resolution
  spExec->Execute(workingDir, command, args);
}

void QmitkSetupVirtualEnvUtil::PipInstall(const std::string &library, CallbackType callback,
  const std::string& command)
{
  this->PipInstall(library, this->GetPipPath().toStdString(), callback, command);
}

void QmitkSetupVirtualEnvUtil::ExecutePython(const std::string &pythonCode,
                                             const std::string &workingDir,
                                             CallbackType callback,
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

void QmitkSetupVirtualEnvUtil::ExecutePython(const std::string &args, CallbackType callback,
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
      auto pyVersionCaptured = QString(QStringDecoder(QStringDecoder::Utf8)(pyProcess.readAllStandardOutput())).toStdString();
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

QString QmitkSetupVirtualEnvUtil::GetPipPackageVersion(const QString &pythonPath, const QString &packageName)
{
#ifdef _WIN32
  const QString PIP_EXE = "pip.exe";
#else
  const QString PIP_EXE = "pip3";
#endif
  QString version;
  QProcess pipProcess;
  pipProcess.start(pythonPath + QDir::separator() + PIP_EXE, QStringList() << "show" << packageName, QProcess::ReadOnly);
  if (pipProcess.waitForFinished())
  {
    while (pipProcess.canReadLine())
    {
      QString line = pipProcess.readLine();
      if (line.startsWith("Version"))
      {
        version = (line.split(":")[1]).trimmed();
        break;
      }
    }
  }
  return version;
}

QStringList QmitkSetupVirtualEnvUtil::AutoParsePythonPaths()
{
  static QStringList foundDirs;
  if (foundDirs.empty())
  {
    QString homeDir = QDir::homePath();
    std::vector<QString> searchDirs;
#ifdef _WIN32
      searchDirs.push_back(QString("C:") + QDir::separator() + QString("ProgramData") + QDir::separator() +
                           QString("anaconda3"));
#else
      // Add search locations for possible standard python paths here
      searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
      searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
      searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
      searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#endif
    for (QString searchDir : searchDirs)
    {
      if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
      {
        if (QDir(searchDir).exists())
        {
          foundDirs.push_back("(base): " + searchDir);
          searchDir.append((QDir::separator() + QString("envs")));
        }
      }
      for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
      {
        subIt.next();
        QString envName = subIt.fileName();
        if (!envName.startsWith('.')) // Filter out irrelevant hidden folders, if any.
        {
          foundDirs.push_back("(" + envName + "): " + subIt.filePath());
        }
      }
    }
  }
  return foundDirs;
}
