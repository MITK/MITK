/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  return nullptr;
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
