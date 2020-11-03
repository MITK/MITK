/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetToPointSetFilter.h"

mitk::PointSetToPointSetFilter::PointSetToPointSetFilter()
{
  // Modify superclass default values, may be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);
}

mitk::PointSetToPointSetFilter::~PointSetToPointSetFilter()
{
}

void mitk::PointSetToPointSetFilter::SetInput(const mitk::PointSetToPointSetFilter::InputType *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<mitk::PointSetToPointSetFilter::InputType *>(input));
}

void mitk::PointSetToPointSetFilter::SetInput(const unsigned int &idx,
                                              const mitk::PointSetToPointSetFilter::InputType *input)
{
  if (idx + 1 > this->GetNumberOfInputs())
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(idx, const_cast<mitk::PointSetToPointSetFilter::InputType *>(input));
}

const mitk::PointSetToPointSetFilter::InputType *mitk::PointSetToPointSetFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  // Process object is not const-correct so the const_cast is required here
  return static_cast<const mitk::PointSetToPointSetFilter::InputType *>(this->ProcessObject::GetInput(0));
}

const mitk::PointSetToPointSetFilter::InputType *mitk::PointSetToPointSetFilter::GetInput(const unsigned int &idx)
{
  if (idx > this->GetNumberOfInputs() - 1)
    return nullptr;
  // Process object is not const-correct so the const_cast is required here
  return static_cast<const mitk::PointSetToPointSetFilter::InputType *>(this->ProcessObject::GetInput(idx));
}

void mitk::PointSetToPointSetFilter::operator=(const mitk::PointSetToPointSetFilter::Self &)
{
}
