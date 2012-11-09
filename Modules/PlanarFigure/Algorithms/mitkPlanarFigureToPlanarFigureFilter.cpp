/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkPlanarFigureToPlanarFigureFilter.h"

mitk::PlanarFigureToPlanarFigureFilter::PlanarFigureToPlanarFigureFilter()
: mitk::PlanarFigureSource()
{
  this->SetNumberOfRequiredInputs(1);
}


mitk::PlanarFigureToPlanarFigureFilter::~PlanarFigureToPlanarFigureFilter()
{
}


void mitk::PlanarFigureToPlanarFigureFilter::SetInput( const InputType* figure )
{
  this->SetInput( 0, figure );
}

void mitk::PlanarFigureToPlanarFigureFilter::SetInput( unsigned int idx, const InputType* figure )
{
  if ( this->GetInput(idx) != figure )
  {
    this->SetNthInput( idx, const_cast<InputType*>( figure ) );
    this->CreateOutputsForAllInputs();
    this->Modified();
  }
}

const mitk::PlanarFigureToPlanarFigureFilter::InputType* mitk::PlanarFigureToPlanarFigureFilter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const InputType*>(this->ProcessObject::GetInput(0));
}

const mitk::PlanarFigureToPlanarFigureFilter::InputType* mitk::PlanarFigureToPlanarFigureFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < idx)
    return NULL;

  return static_cast<const InputType*>(this->ProcessObject::GetInput(idx));
}

void mitk::PlanarFigureToPlanarFigureFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
  }
}
