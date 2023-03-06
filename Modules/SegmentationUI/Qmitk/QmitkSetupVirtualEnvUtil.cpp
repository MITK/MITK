/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#include "QmitkSetupVirtualEnvUtil.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <itkCommand.h>
#include "mitkLogMacros.h"

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

std::map<std::string, std::string> QmitkSetupVirtualEnvUtil::GetInstallParameters(
  QmitkSetupVirtualEnvUtil::Tool packageName)
{
  std::map<std::string, std::string> parameters;

  switch (packageName)
  {
    case QmitkSetupVirtualEnvUtil::Tool::TOTALSEGMENTATOR:
    {
      parameters["venv_name"] = ".totalsegmentator";
      parameters["pkg_version"] = "1.5.2";
      parameters["pkg_names"] = "Totalsegmentator==1.5.2,scikit-image";
      break;
    }
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
  spExec->Execute(m_BaseDir.toStdString(), "/usr/bin/python3", args); // Setup local virtual environment

  if (folderPath.cd("bin"))
  {
    m_venvPath = folderPath.absolutePath();
    std::string workingDir = m_venvPath.toStdString();
    for (auto& package : packages)
    {
      PipInstall(package, workingDir);
    }
    std::string pythonCode; // python syntax to check if torch is installed with CUDA.
    pythonCode.append("import torch;");
    pythonCode.append("print('Pytorch installed with CUDA') if torch.cuda.is_available else ValueError('PyTorch "
                      "installed without CUDA');");
    ExecutePython(pythonCode, workingDir);
    return true;
  }  
  return false;
}

void QmitkSetupVirtualEnvUtil::PipInstall(const std::string &library, const std::string &workingDir)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&PrintProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("install");
  args.push_back(library);
  spExec->Execute(workingDir, "pip3", args); // Install TotalSegmentator in it.
}

void QmitkSetupVirtualEnvUtil::ExecutePython(const std::string &pythonCode, const std::string &workingDir)
{
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&PrintProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  args.push_back("-c");
  args.push_back(pythonCode);
  spExec->Execute(workingDir, "python3", args);
}
