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
const QStringList QmitkTotalSegmentatorToolInstaller::PACKAGES = {
    "Totalsegmentator==" + TOTALSEGMENTATOR_VERSION,
    "acvl_utils==0.2",
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




void QmitkTotalSegmentatorToolInstaller::PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *)
{
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  std::string eventOut;

  if (const auto* pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent*>(&e); pEvent != nullptr)
  {
    eventOut = pEvent->GetOutput();
  }
  else if (const auto* pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent*>(&e); pErrEvent != nullptr)
  {
    eventOut = pErrEvent->GetOutput();
  }
  auto consoleOutput = QmitkToolInstallDialog::GetConsoleOutput();
  QMetaObject::invokeMethod(consoleOutput, "appendHtml", Q_ARG(QString, QString::fromStdString(eventOut)));
}

QString QmitkTotalSegmentatorToolInstaller::GetVirtualEnvPath()
{
  return STORAGE_DIR + VENV_NAME;
}
