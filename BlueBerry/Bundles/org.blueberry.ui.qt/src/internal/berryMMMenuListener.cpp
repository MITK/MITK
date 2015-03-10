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


#include "berryMMMenuListener.h"

#include <berryMenuManager.h>

namespace berry {

MMMenuListener::MMMenuListener(berry::MenuManager* mm)
  : QObject(), mm(mm)
{}

void MMMenuListener::HandleAboutToShow()
{
  if (mm->removeAllWhenShown)
  {
    mm->RemoveAll();
  }
  mm->Update(false, false);
}

}
