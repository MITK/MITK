/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingToolInstaller.h"

#include "QmitkToolInstallDialog.h"
#include <itkCommand.h>
#include <mitkCoreServices.h>
#include <mitkProcessExecutor.h>
#include <QDir>
#include <QStandardPaths>
#include <QMutex>

const QString QmitkSAMToolInstaller::VENV_NAME = ".sam";
const QString QmitkSAMToolInstaller::SAM_VERSION = "1.0";
const QStringList QmitkSAMToolInstaller::PACKAGES = {
  QString("git+https://github.com/MIC-DKFZ/agent-sam.git@v0.2")};
const QString QmitkSAMToolInstaller::STORAGE_DIR =
  QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() + qApp->organizationName() +
  QDir::separator();

QmitkSAMToolInstaller::QmitkSAMToolInstaller() : QmitkSetupVirtualEnvUtil(STORAGE_DIR){};


QString QmitkSAMToolInstaller::GetVirtualEnvPath()
{
  return STORAGE_DIR + VENV_NAME;
}

void QmitkSAMToolInstaller::PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *)
{
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  std::string eventOut;

  if (const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e); pEvent != nullptr)
  {
    eventOut = pEvent->GetOutput();
  }
  else if (const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e); pErrEvent != nullptr)
  {
    eventOut = pErrEvent->GetOutput();
  }
  auto consoleOutput = QmitkToolInstallDialog::GetConsoleOutput();
  QMetaObject::invokeMethod(consoleOutput, "appendHtml", Q_ARG(QString, QString::fromStdString(eventOut)));
}
