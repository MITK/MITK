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

const std::string PartPlaceholder::WILD_CARD = "*"; //$NON-NLS-1$

PartPlaceholder::PartPlaceholder(const std::string& id) :
  LayoutPart(id)
{

}

void PartPlaceholder::CreateControl(void*  /*parent*/)
{
  // do nothing
}

void* PartPlaceholder::GetControl()
{
  return 0;
}

bool PartPlaceholder::HasWildCard()
{
  return this->GetID().find_first_of(WILD_CARD) != std::string::npos;
}

bool PartPlaceholder::IsPlaceHolder() const
{
  return true;
}

}
