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

#ifndef __MITK_DIFFUSIONIMAGE_SOURCE_CPP__
#define __MITK_DIFFUSIONIMAGE_SOURCE_CPP__

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

template<typename TPixelType>
typename mitk::DiffusionImageSource<TPixelType>::OutputType*
mitk::DiffusionImageSource<TPixelType>::GetOutput(void)
{
  return itkDynamicCastInDebugMode<OutputType*>( this->GetPrimaryOutput() );
}

template<typename TPixelType>
const typename mitk::DiffusionImageSource<TPixelType>::OutputType*
mitk::DiffusionImageSource<TPixelType>::GetOutput(void) const
{
  return itkDynamicCastInDebugMode<const OutputType*>( this->GetPrimaryOutput() );
}

template<typename TPixelType>
typename mitk::DiffusionImageSource<TPixelType>::OutputType*
mitk::DiffusionImageSource<TPixelType>::GetOutput(DataObjectPointerArraySizeType idx)
{
  OutputType* out = dynamic_cast<OutputType*>( this->ProcessObject::GetOutput(idx) );
  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
  }
  return out;
}

template<typename TPixelType>
const typename mitk::DiffusionImageSource<TPixelType>::OutputType*
mitk::DiffusionImageSource<TPixelType>::GetOutput(DataObjectPointerArraySizeType idx) const
{
  const OutputType* out = dynamic_cast<const OutputType*>( this->ProcessObject::GetOutput(idx) );
  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
  }
  return out;
}

#endif //__MITK_DIFFUSIONIMAGE_SOURCE_CPP__
