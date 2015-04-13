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
