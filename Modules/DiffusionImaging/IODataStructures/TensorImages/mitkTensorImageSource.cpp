/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

mitk::TensorImageSource::DataObjectPointer mitk::TensorImageSource::MakeOutput( unsigned int  /*idx*/ )
{
  return static_cast<itk::DataObject*>(OutputImageType::New().GetPointer());
}
