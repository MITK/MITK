/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
