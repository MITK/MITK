
/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtWorkbenchTweaklet.h"

#include <QApplication>

#include "berryQtDisplay.h"

namespace berry {

QtWorkbenchTweaklet::QtWorkbenchTweaklet()
{

}

Display* QtWorkbenchTweaklet::CreateDisplay()
{
  return new QtDisplay();
}

bool QtWorkbenchTweaklet::IsRunning()
{
  return QApplication::instance() != nullptr;
}

}  // namespace berry
