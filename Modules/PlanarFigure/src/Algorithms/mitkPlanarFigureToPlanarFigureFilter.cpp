/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureToPlanarFigureFilter.h"

mitk::PlanarFigureToPlanarFigureFilter::PlanarFigureToPlanarFigureFilter() : mitk::PlanarFigureSource()
{
  this->SetNumberOfRequiredInputs(1);
}

mitk::PlanarFigureToPlanarFigureFilter::~PlanarFigureToPlanarFigureFilter()
{
}

void mitk::PlanarFigureToPlanarFigureFilter::SetInput(const InputType *figure)
{
  this->SetInput(0, figure);
}

void mitk::PlanarFigureToPlanarFigureFilter::SetInput(unsigned int idx, const InputType *figure)
{
  if (this->GetInput(idx) != figure)
  {
    this->SetNthInput(idx, const_cast<InputType *>(figure));
    //    this->CreateOutputsForAllInputs();
    this->Modified();
  }
}

const mitk::PlanarFigureToPlanarFigureFilter::InputType *mitk::PlanarFigureToPlanarFigureFilter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const InputType *>(this->ProcessObject::GetInput(0));
}

const mitk::PlanarFigureToPlanarFigureFilter::InputType *mitk::PlanarFigureToPlanarFigureFilter::GetInput(
  unsigned int idx)
{
  if (this->GetNumberOfInputs() < idx)
    return nullptr;

  return static_cast<const InputType *>(this->ProcessObject::GetInput(idx));
}

void mitk::PlanarFigureToPlanarFigureFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfIndexedOutputs(this->GetNumberOfInputs()); // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
  }
}
