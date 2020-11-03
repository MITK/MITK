/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSetSource.h"

mitk::ContourModelSetSource::ContourModelSetSource()
{
  // Create the output.
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredInputs(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}

mitk::ContourModelSetSource::~ContourModelSetSource()
{
}

itk::DataObject::Pointer mitk::ContourModelSetSource::MakeOutput(DataObjectPointerArraySizeType /*idx*/)
{
  return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::ContourModelSetSource::MakeOutput(const DataObjectIdentifierType &name)
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if (this->IsIndexedOutputName(name))
  {
    return this->MakeOutput(this->MakeIndexFromOutputName(name));
  }
  return OutputType::New().GetPointer();
}

mitkBaseDataSourceGetOutputDefinitions(mitk::ContourModelSetSource)
