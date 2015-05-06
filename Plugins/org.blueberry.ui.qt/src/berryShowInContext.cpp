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

#include "berryShowInContext.h"

#include "berryISelection.h"

namespace berry {

ShowInContext::ShowInContext(const Object::Pointer& input, const SmartPointer<const ISelection>& selection)
  : input(input)
  , selection(selection)
{
}

Object::Pointer ShowInContext::GetInput() const
{
  return input;
}

SmartPointer<const ISelection> ShowInContext::GetSelection() const
{
  return selection;
}

void ShowInContext::SetInput(const Object::Pointer& input)
{
  this->input = input;
}

void ShowInContext::SetSelection(const SmartPointer<const ISelection>& selection)
{
  this->selection = selection;
}

}
