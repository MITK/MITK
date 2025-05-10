/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentAnythingToolInstaller_h
#define QmitkSegmentAnythingToolInstaller_h

#include <QApplication>
#include <QmitkSetupVirtualEnvUtil.h>
#include <QStringList>

class QmitkSAMToolInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  static const QString VENV_NAME;
  static const QString SAM_VERSION; // currently, unused
  static const QStringList PACKAGES;
  static const QString STORAGE_DIR;
  QmitkSAMToolInstaller();
  QString GetVirtualEnvPath() override;
  static void PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *);
};

#endif
