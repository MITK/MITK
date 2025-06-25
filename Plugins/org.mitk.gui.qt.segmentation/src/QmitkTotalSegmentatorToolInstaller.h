/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorToolInstaller_h
#define QmitkTotalSegmentatorToolInstaller_h

#include <QmitkSetupVirtualEnvUtil.h>

class QmitkTotalSegmentatorToolInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  static const QString VENV_NAME;
  static const QString TOTALSEGMENTATOR_VERSION;
  static const QStringList PACKAGES;
  static const QString STORAGE_DIR;
  QmitkTotalSegmentatorToolInstaller();
  QString GetVirtualEnvPath() override;
  static void PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *);
};

#endif
