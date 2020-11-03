/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSetImageSource.h"
#include "mitkLabelSetImage.h"

mitk::LabelSetImageSource::LabelSetImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type LabelSetImage
  mitk::LabelSetImage::Pointer output = static_cast<mitk::LabelSetImage *>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

mitk::LabelSetImageSource::~LabelSetImageSource()
{
}

itk::DataObject::Pointer mitk::LabelSetImageSource::MakeOutput(DataObjectPointerArraySizeType /*idx*/)
{
  return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::LabelSetImageSource::MakeOutput(const DataObjectIdentifierType &name)
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if (this->IsIndexedOutputName(name))
  {
    return this->MakeOutput(this->MakeIndexFromOutputName(name));
  }
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

mitkBaseDataSourceGetOutputDefinitions(mitk::LabelSetImageSource)
