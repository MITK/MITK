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


#include "mitkTensorImageSource.h"


mitk::TensorImageSource::TensorImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  OutputImageType::Pointer output
    = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer());
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

itk::DataObject::Pointer mitk::TensorImageSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return OutputImageType::New().GetPointer();
}


itk::DataObject::Pointer mitk::TensorImageSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(OutputImageType::New().GetPointer());
}
