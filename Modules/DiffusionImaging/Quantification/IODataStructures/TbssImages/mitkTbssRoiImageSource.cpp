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

#ifndef __MITK_NRRD_TBSSROI_VOLUMES_IO_FACTORY_CPP__
#define __MITK_NRRD_TBSSROI_VOLUMES_IO_FACTORY_CPP__

#include "mitkTbssRoiImageSource.h"
#include "mitkTbssRoiImage.h"


mitk::TbssRoiImageSource::TbssRoiImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type DiffusionImage
  mitk::TbssRoiImage::Pointer output
    = static_cast<mitk::TbssRoiImage*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}


itk::DataObject::Pointer mitk::TbssRoiImageSource::MakeOutput( unsigned int /*idx*/ )
{
  return static_cast<itk::DataObject*>(mitk::TbssRoiImage::New().GetPointer());
}



mitk::TbssRoiImageSource::OutputType* mitk::TbssRoiImageSource::GetOutput(unsigned int idx)
{
  return static_cast<OutputType*>
                     (this->ProcessObject::GetOutput(idx));
}



#endif //__MITK_NRRD_TBSS_VOULMES_IO_FACTORY_CPP__
