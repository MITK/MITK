
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
  return QApplication::instance() != 0;
}

}  // namespace berry
