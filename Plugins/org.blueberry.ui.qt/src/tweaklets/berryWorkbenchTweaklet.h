/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYWORKBENCHTWEAKLET_H_
#define BERRYWORKBENCHTWEAKLET_H_

#include "internal/berryTweaklets.h"

#include "berryShell.h"
#include "berryDisplay.h"

namespace berry
{

class WorkbenchWindow;

struct BERRY_UI_QT WorkbenchTweaklet
{

  static Tweaklets::TweakKey<WorkbenchTweaklet> KEY;

  virtual Display* CreateDisplay() = 0;

  virtual bool IsRunning() = 0;

};

}

Q_DECLARE_INTERFACE(berry::WorkbenchTweaklet, "org.blueberry.WorkbenchTweaklet")

#endif /* BERRYWORKBENCHTWEAKLET_H_ */
