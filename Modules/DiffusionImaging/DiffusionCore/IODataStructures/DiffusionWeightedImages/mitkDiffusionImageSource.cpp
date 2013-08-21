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

#ifndef __MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__
#define __MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__

#include "mitkDiffusionImageSource.h"
#include "mitkDiffusionImage.h"

template<typename TPixelType>
mitk::DiffusionImageSource<TPixelType>::DiffusionImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type DiffusionImage
  typename mitk::DiffusionImage<TPixelType>::Pointer output
    = static_cast<typename mitk::DiffusionImage<TPixelType>*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

template<typename TPixelType>
mitk::DiffusionImageSource<TPixelType>::~DiffusionImageSource()
{
}

template<typename TPixelType>
itk::DataObject::Pointer mitk::DiffusionImageSource<TPixelType>::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}


template<typename TPixelType>
itk::DataObject::Pointer mitk::DiffusionImageSource<TPixelType>::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}


//template<typename TPixelType>
//mitk::DiffusionImage<TPixelType>* mitk::DiffusionImageSource<TPixelType>::GetOutput()
//{
//  if (this->GetNumberOfOutputs() < 1)
//  {
//    return 0;
//  }
//
//  return static_cast<mitk::DiffusionImage<TPixelType>*>
//    (this->BaseProcess::GetOutput());
//}
//
//
//template<typename TPixelType>
//void mitk::DiffusionImageSource<TPixelType>::GraftOutput(mitk::DiffusionImage<TPixelType>* graft)
//{
//  this->GraftNthOutput(0, graft);
//}
//
//template<typename TPixelType>
//void mitk::DiffusionImageSource<TPixelType>::GraftNthOutput(unsigned int idx, mitk::DiffusionImage<TPixelType> *graft)
//{
//  if (idx < this->GetNumberOfOutputs())
//  {
//    mitk::DiffusionImage<TPixelType> * output = this->GetOutput(idx);
//
//    if (output && graft)
//    {
//      // grab a handle to the bulk data of the specified data object
//      //      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!
//
//      // copy the region ivars of the specified data object
//      output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
//      //      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
//      //      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!
//
//      // copy the meta-information
//      output->CopyInformation( graft );
//    }
//  }
//}

#endif //__MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__
