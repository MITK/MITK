/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkTotalSegmentatorToolInstaller.h"
#include <itkCommand.h>
#include "QmitkToolInstallDialog.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QMutex>

const QString QmitkTotalSegmentatorToolInstaller::VENV_NAME = ".totalsegmentator_v2";
const QString QmitkTotalSegmentatorToolInstaller::TOTALSEGMENTATOR_VERSION = "2.4.0";
const std::vector<QString> QmitkTotalSegmentatorToolInstaller::PACKAGES = {
    "Totalsegmentator==" + TOTALSEGMENTATOR_VERSION,
    "numpy<2",
    "SimpleITK<=2.3.1",
    "p-tqdm<=1.4.0",
    "xvfbwrapper<=0.2.9",
    "rt_utils<=1.2.7",
    "dicom2nifti<=2.4.11",
    "pyarrow<=16.1.0",
    "setuptools"};/* just in case*/
const QString QmitkTotalSegmentatorToolInstaller::STORAGE_DIR =
  QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
                            qApp->organizationName() + QDir::separator();

 QmitkTotalSegmentatorToolInstaller::QmitkTotalSegmentatorToolInstaller() : QmitkSetupVirtualEnvUtil(STORAGE_DIR) {}


bool QmitkTotalSegmentatorToolInstaller::SetupVirtualEnv(const QString &venvName)
{
  if (GetSystemPythonPath().isEmpty())
  {
    return false;
  }
  if (!QmitkSetupVirtualEnvUtil::IsVenvInstalled(GetSystemPythonPath()))
  {
    return false;
  }
  QDir folderPath(GetBaseDir());
  folderPath.mkdir(venvName);
  if (!folderPath.cd(venvName))
  {
    return false; // Check if directory creation was successful.
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
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
    this->InstallPytorch(&QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
    for (auto &package : PACKAGES)
    {
      this->PipInstall(package.toStdString(), &QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
    }
    std::string pythonCode; // python syntax to check if torch is installed with CUDA.
    pythonCode.append("import torch;");
    pythonCode.append(
      "print('TotalSegmentator was installed with CUDA') if torch.cuda.is_available() else print('TotalSegmentator was "
      "installed WITHOUT CUDA');");
    this->ExecutePython(pythonCode, &QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
    return true;
  }
  return false;
}

void QmitkTotalSegmentatorToolInstaller::PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *)
{
  std::string eventOut;
  const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);
  if (pEvent)
  {
    eventOut = pEvent->GetOutput();
  }
  const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e);
  if (pErrEvent)
  {
    eventOut = pErrEvent->GetOutput();
  }
  static QMutex mutex;
  mutex.lock();
  QMetaObject::invokeMethod(QmitkToolInstallDialog::m_ConsoleOut,
                            "append",
                            Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromStdString(eventOut))
                            );
  mutex.unlock();
}

QString QmitkTotalSegmentatorToolInstaller::GetVirtualEnvPath()
{
  return STORAGE_DIR + VENV_NAME;
}
