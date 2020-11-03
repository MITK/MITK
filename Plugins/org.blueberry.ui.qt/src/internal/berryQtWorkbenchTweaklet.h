/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTWORKBENCHTWEAKLET_H_
#define BERRYQTWORKBENCHTWEAKLET_H_

#include <berryWorkbenchTweaklet.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

class BERRY_UI_QT QtWorkbenchTweaklet : public QObject, public WorkbenchTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::WorkbenchTweaklet)

public:

  QtWorkbenchTweaklet();

  Display* CreateDisplay() override;

  bool IsRunning() override;

};

} // namespace berry

#endif /*BERRYQTWORKBENCHTWEAKLET_H_*/
