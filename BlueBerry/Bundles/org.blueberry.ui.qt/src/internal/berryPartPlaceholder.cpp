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

#include "berryPartPlaceholder.h"

#include "berryPartStack.h"

namespace berry
{

const QString PartPlaceholder::WILD_CARD = "*";

PartPlaceholder::PartPlaceholder(const QString& id) :
  LayoutPart(id)
{

}

void PartPlaceholder::CreateControl(QWidget*  /*parent*/)
{
  // do nothing
}

QWidget* PartPlaceholder::GetControl()
{
  return 0;
}

bool PartPlaceholder::HasWildCard()
{
  return this->GetID().indexOf(WILD_CARD) != -1;
}

bool PartPlaceholder::IsPlaceHolder() const
{
  return true;
}

}
