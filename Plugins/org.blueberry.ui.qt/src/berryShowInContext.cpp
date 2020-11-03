/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
