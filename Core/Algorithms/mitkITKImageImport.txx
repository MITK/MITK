/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkITKImageImport_txx
#define __mitkITKImageImport_txx
#include "mitkITKImageImport.h"

template <class TInputImage>
mitk::ITKImageImport<TInputImage>::ITKImageImport()
{
}

template <class TInputImage>
mitk::ITKImageImport<TInputImage>::~ITKImageImport()
{
}

template <class TInputImage>
typename mitk::ITKImageImport<TInputImage>::InputImageType *
mitk::ITKImageImport<TInputImage>::GetInput(void)
{
  return static_cast<TInputImage*>(
    this->ProcessObject::GetInput(0));
}

template <class TInputImage>
void mitk::ITKImageImport<TInputImage>::SetInput(const InputImageType* input)
{
  this->ProcessObject::SetNthInput(0, const_cast<TInputImage*>(input) );
}

template <class TInputImage>
void mitk::ITKImageImport<TInputImage>::GenerateOutputInformation()
{
	InputImageConstPointer input  = this->GetInput();
	mitk::Image::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateOutputInformation()");

	output->InitializeByItk(input.GetPointer());
}

template <class TInputImage>
void mitk::ITKImageImport<TInputImage>::GenerateData()
{
	InputImageConstPointer input  = this->GetInput();
	mitk::Image::Pointer output = this->GetOutput();

  output->SetImportVolume((void*)input->GetBufferPointer(), 0, 0, mitk::Image::ReferenceMemory); 
}

template <class TInputImage>
void mitk::ITKImageImport<TInputImage>::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  // Input is an image, cast away the constness so we can set
  // the requested region.
  InputImagePointer input =
    const_cast< TInputImage * > ( this->GetInput() );

  // Use the function object RegionCopier to copy the output region
  // to the input.  The default region copier has default implementations
  // to handle the cases where the input and output are the same
  // dimension, the input a higher dimension than the output, and the
  // input a lower dimension than the output.
  InputImageRegionType inputRegion;
  OutputToInputRegionCopierType regionCopier;
  regionCopier(inputRegion, this->GetOutput()->GetRequestedRegion());
  input->SetRequestedRegion( inputRegion );
}

template <class TInputImage>
void mitk::ITKImageImport<TInputImage>::SetNthOutput(unsigned int idx, itk::DataObject *output)
{
  if((output == NULL) && (idx == 0))
  {
    // we are disconnected from our output:
    // copy buffer of input to output, because we
    // cannot guarantee that the input (to which our
    // output is refering) will stay alive.
  	InputImageConstPointer input = this->GetInput();
  	mitk::Image::Pointer currentOutput = this->GetOutput();
  	if(input.IsNotNull() && currentOutput.IsNotNull())
  	  currentOutput->SetVolume(input->GetBufferPointer());
  }
  Superclass::SetNthOutput(idx, output);
}

template <typename ItkOutputImageType> 
mitk::Image::Pointer mitk::ImportItkImage(const itk::SmartPointer<ItkOutputImageType>& itkimage, bool update)
{
  mitk::ITKImageImport<ItkOutputImageType>::Pointer importer = mitk::ITKImageImport<ItkOutputImageType>::New();
  importer->SetInput(itkimage);
  if(update)
    importer->Update();
  return importer->GetOutput();
}

template <typename ItkOutputImageType> 
mitk::Image::Pointer mitk::ImportItkImage(const ItkOutputImageType* itkimage, bool update)
{
  mitk::ITKImageImport<ItkOutputImageType>::Pointer importer = mitk::ITKImageImport<ItkOutputImageType>::New();
  importer->SetInput(itkimage);
  if(update)
    importer->Update();
  return importer->GetOutput();
}

#endif //__mitkITKImageImport_txx
