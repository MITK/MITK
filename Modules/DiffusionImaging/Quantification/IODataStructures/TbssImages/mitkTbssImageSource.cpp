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

#ifndef __MITK_NRRD_TBSS_VOULMES_IO_FACTORY_CPP__
#define __MITK_NRRD_TBSS_VOULMES_IO_FACTORY_CPP__

#include "mitkTbssImageSource.h"
#include "mitkTbssImage.h"


mitk::TbssImageSource::TbssImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type DiffusionImage
  mitk::TbssImage::Pointer output
    = static_cast<mitk::TbssImage*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}


itk::DataObject::Pointer mitk::TbssImageSource::MakeOutput( unsigned int /*idx*/ )
{
  return static_cast<itk::DataObject*>(mitk::TbssImage::New().GetPointer());
}



mitk::TbssImageSource::OutputType* mitk::TbssImageSource::GetOutput(unsigned int idx)
{
  return static_cast<OutputType*>
                     (this->ProcessObject::GetOutput(idx));
}



#endif //__MITK_NRRD_TBSS_VOULMES_IO_FACTORY_CPP__
