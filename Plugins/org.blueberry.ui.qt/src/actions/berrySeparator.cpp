/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berrySeparator.h"

#include <QMenu>
#include <QToolBar>

namespace berry {

Separator::Separator()
{
}

Separator::Separator(const QString& groupName)
  : AbstractGroupMarker(groupName)
{
}

void Separator::Fill(QMenu* menu, QAction* before)
{
  if (before)
  {
    menu->insertSeparator(before);
  }
  else
  {
    menu->addSeparator();
  }
}

void Separator::Fill(QToolBar* toolbar, QAction* before)
{
  if (before)
  {
    toolbar->insertSeparator(before);
  }
  else
  {
    toolbar->addSeparator();
  }
}

bool Separator::IsSeparator() const
{
  return true;
}

}
