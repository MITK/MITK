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

#ifndef __MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__
#define __MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__

#include "mitkDiffusionVolumesSource.h"
#include "mitkDiffusionVolumes.h"

template<typename TPixelType>
mitk::DiffusionVolumesSource<TPixelType>::DiffusionVolumesSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type DiffusionVolumes
  typename mitk::DiffusionVolumes<TPixelType>::Pointer output
    = static_cast<typename mitk::DiffusionVolumes<TPixelType>*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

template<typename TPixelType>
mitk::DiffusionVolumesSource<TPixelType>::~DiffusionVolumesSource()
{
}


template<typename TPixelType>
itk::DataObject::Pointer mitk::DiffusionVolumesSource<TPixelType>::MakeOutput( unsigned int /*idx*/ )
{
  return static_cast<itk::DataObject*>(mitk::DiffusionVolumes<TPixelType>::New().GetPointer());
}


template<typename TPixelType>
mitk::DiffusionVolumes<TPixelType>* mitk::DiffusionVolumesSource<TPixelType>::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::DiffusionVolumes<TPixelType>*>
    (this->BaseProcess::GetOutput(0));
}

template<typename TPixelType>
mitk::DiffusionVolumes<TPixelType>* mitk::DiffusionVolumesSource<TPixelType>::GetOutput(unsigned int idx)
{
  return static_cast<mitk::DiffusionVolumes<TPixelType>*>
    (this->ProcessObject::GetOutput(idx));
}

template<typename TPixelType>
void mitk::DiffusionVolumesSource<TPixelType>::SetOutput(mitk::DiffusionVolumes<TPixelType>* output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
}

template<typename TPixelType>
void mitk::DiffusionVolumesSource<TPixelType>::GraftOutput(mitk::DiffusionVolumes<TPixelType>* graft)
{
  this->GraftNthOutput(0, graft);
}

template<typename TPixelType>
void mitk::DiffusionVolumesSource<TPixelType>::GraftNthOutput(unsigned int idx, mitk::DiffusionVolumes<TPixelType> *graft)
{
  if (idx < this->GetNumberOfOutputs())
  {
    mitk::DiffusionVolumes<TPixelType> * output = this->GetOutput(idx);

    if (output && graft)
    {
      // grab a handle to the bulk data of the specified data object
      //      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!

      // copy the region ivars of the specified data object
      output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
      //      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
      //      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!

      // copy the meta-information
      output->CopyInformation( graft );
    }
  }
}

#endif //__MITK_NRRD_DIFFUSION_VOULMES_IO_FACTORY_CPP__