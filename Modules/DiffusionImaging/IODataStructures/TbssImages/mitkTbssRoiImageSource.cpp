/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 10345 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __MITK_NRRD_TBSSROI_VOLUMES_IO_FACTORY_CPP__
#define __MITK_NRRD_TBSSROI_VOLUMES_IO_FACTORY_CPP__

#include "mitkTbssRoiImageSource.h"
#include "mitkTbssRoiImage.h"

template<typename TPixelType>
mitk::TbssRoiImageSource<TPixelType>::TbssRoiImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type DiffusionImage
  typename mitk::TbssRoiImage<TPixelType>::Pointer output
    = static_cast<typename mitk::TbssRoiImage<TPixelType>*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}


template<typename TPixelType>
itk::DataObject::Pointer mitk::TbssRoiImageSource<TPixelType>::MakeOutput( unsigned int /*idx*/ )
{
  return static_cast<itk::DataObject*>(mitk::TbssRoiImage<TPixelType>::New().GetPointer());
}

/*
template<typename TPixelType>
mitk::TbssImageSource<TPixelType>::OutputType* mitk::TbssImageSource::GetOutput(unsigned int idx)
{
  return static_cast<OutputType*>
                     (this->ProcessObject::GetOutput(idx));
}

*/

#endif //__MITK_NRRD_TBSS_VOULMES_IO_FACTORY_CPP__
