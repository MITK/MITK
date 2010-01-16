/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "berryPartPlaceholder.h"

#include "berryPartStack.h"

namespace berry
{

const std::string PartPlaceholder::WILD_CARD = "*"; //$NON-NLS-1$

PartPlaceholder::PartPlaceholder(const std::string& id) :
  StackablePart(id)
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
  return this->GetId().find_first_of(WILD_CARD) != std::string::npos;
}

bool PartPlaceholder::IsPlaceHolder() const
{
  return true;
}

}
