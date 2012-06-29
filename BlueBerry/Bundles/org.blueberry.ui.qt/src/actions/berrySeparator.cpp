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

QAction* Separator::Fill(QMenu* menu, QAction* before)
{
  if (before)
  {
    return menu->insertSeparator(before);
  }
  return menu->addSeparator();
}

QAction* Separator::Fill(QToolBar* toolbar, QAction* before)
{
  if (before)
  {
    return toolbar->insertSeparator(before);
  }
  return toolbar->addSeparator();
}

bool Separator::IsSeparator() const
{
  return true;
}

}
