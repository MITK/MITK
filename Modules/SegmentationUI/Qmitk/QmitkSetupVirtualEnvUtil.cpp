/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#include "QmitkSetupVirtualEnvUtil.h"

#include "mitkLogMacros.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <itkCommand.h>

QmitkSetupVirtualEnvUtil::QmitkSetupVirtualEnvUtil()
{
  m_BaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
              qApp->organizationName() + QDir::separator();
}

QmitkSetupVirtualEnvUtil::QmitkSetupVirtualEnvUtil(const QString baseDir)
{
  m_BaseDir = baseDir;
}

QString QmitkSetupVirtualEnvUtil::GetBaseDir()
{
  return m_BaseDir;
}

QString QmitkSetupVirtualEnvUtil::GetVirtualEnvPath()
{
  return m_venvPath;
}

void QmitkSetupVirtualEnvUtil::SetVirtualEnvPath(const QString &path)
{
  m_venvPath = path;
}

void QmitkSetupVirtualEnvUtil::SetSystemPythonPath(const QString& path)
{
  if (this->IsPythonPath(path))
  {
    m_SysPythonPath = path;
  }
  else
  {
    MITK_INFO << "Python was not detected in "+path.toStdString();
  }
}

std::map<std::string, std::string> QmitkSetupVirtualEnvUtil::GetInstallParameters(
  QmitkSetupVirtualEnvUtil::Tool packageName)
{
  std::map<std::string, std::string> parameters;

  switch (packageName)
  {
    case QmitkSetupVirtualEnvUtil::Tool::TOTALSEGMENTATOR:
    {
      parameters["venv_name"] = ".totalsegmentator";
      parameters["pkg_version"] = "1.5.3";
      parameters["pkg_names"] = "Totalsegmentator==1.5.3,scipy==1.9.1"; // comma separated string
      /*Add other parameters here...*/
      break;
    } /*Add more tools here...*/
    default:
    {
      std::runtime_error("Unsupported Tool type");
      break;
    }
  }
  return parameters;
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

bool QmitkSetupVirtualEnvUtil::SetupVirtualEnv(QmitkSetupVirtualEnvUtil::Tool packageName)
{
  MITK_INFO << m_SysPythonPath.toStdString();
  if (m_SysPythonPath.isEmpty())
  {
    return false;
  }
  std::vector<std::string> packages;
  std::map<std::string, std::string> params = GetInstallParameters(packageName);

  std::stringstream pkgs(params["pkg_names"]);
  while (pkgs.good())
  {
    std::string pkg;
    std::getline(pkgs, pkg, ',');
    packages.push_back(pkg);
  }
  QString venvName = QString::fromStdString(params["venv_name"]);
  QDir folderPath(m_BaseDir);
  folderPath.mkdir(venvName);
  if (!folderPath.cd(venvName))
  {
    return false; // Check if directory creation was successful.
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&PrintProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);

  args.push_back("-m");
  args.push_back("venv");
  args.push_back(venvName.toStdString());
#ifdef _WIN32
  QString pythonFile = m_SysPythonPath + QDir::separator() + "python.exe";
  spExec->Execute(m_BaseDir.toStdString(), pythonFile.toStdString(), args); // Setup local virtual environment
  QString pythonExeFolder = "Scripts";
#else
  QString pythonFile = m_SysPythonPath + QDir::separator() + "python3";
  spExec->Execute(m_BaseDir.toStdString(), "/usr/bin/python3", args); // Setup local virtual environment
  QString pythonExeFolder = "bin";
#endif

  if (folderPath.cd(pythonExeFolder))
  {
    m_venvPath = folderPath.absolutePath();
    std::string workingDir = m_venvPath.toStdString();
    InstallPytorch(workingDir, &PrintProcessEvent);
    for (auto &package : packages)
    {
      PipInstall(package, workingDir, &PrintProcessEvent);
    }
    std::string pythonCode; // python syntax to check if torch is installed with CUDA.
    pythonCode.append("import torch;");
    pythonCode.append("print('Pytorch was installed with CUDA') if torch.cuda.is_available() else print('PyTorch was "
                      "installed WITHOUT CUDA');");
    ExecutePython(pythonCode, workingDir, &PrintProcessEvent);
    return true;
  }
  return false;
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
  args.push_back("light-the-torch");
  spExec->Execute(workingDir, "python", args);
  PipInstall("torch", workingDir, callback, "ltt");
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


bool QmitkSetupVirtualEnvUtil::IsPythonPath(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  fullPath = fullPath.mid(fullPath.indexOf(" ") + 1);
  bool isExists =
#ifdef _WIN32
                  QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
#else
                  QFile::exists(fullPath + QDir::separator() + QString("python3"));
#endif
  return isExists;
}