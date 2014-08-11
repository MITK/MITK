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


#include "mitkImageToImageFilter.h"

mitk::ImageToImageFilter::ImageToImageFilter()
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);
}

mitk::ImageToImageFilter::~ImageToImageFilter()
{
}

/**
*
*/
void mitk::ImageToImageFilter::SetInput(const mitk::ImageToImageFilter::InputImageType *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0,
    const_cast< mitk::ImageToImageFilter::InputImageType * >( input ) );
}


/**
* Connect one of the operands for pixel-wise addition
*/
void mitk::ImageToImageFilter::SetInput( unsigned int index, const mitk::ImageToImageFilter::InputImageType * image )
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index,
    const_cast< mitk::ImageToImageFilter::InputImageType *>( image ) );
}



/**
*
*/
mitk::ImageToImageFilter::InputImageType *mitk::ImageToImageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::ImageToImageFilter::InputImageType * >
    (this->ProcessObject::GetInput(0) );
}

/**
*
*/
mitk::ImageToImageFilter::InputImageType *mitk::ImageToImageFilter::GetInput(unsigned int idx)
{
  return static_cast< mitk::ImageToImageFilter::InputImageType * >
    (this->ProcessObject::GetInput(idx));
}

const mitk::ImageToImageFilter::InputImageType *mitk::ImageToImageFilter::GetInput(void) const
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::ImageToImageFilter::InputImageType * >
    (this->ProcessObject::GetInput(0) );
}

/**
*
*/
const mitk::ImageToImageFilter::InputImageType *mitk::ImageToImageFilter::GetInput(unsigned int idx) const
{
  return static_cast< const mitk::ImageToImageFilter::InputImageType * >
    (this->ProcessObject::GetInput(idx));
}

//-----------------------------------------------------------------------
//
void mitk::ImageToImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  for (unsigned int idx = 0; idx < this->GetNumberOfInputs(); ++idx)
  {
    if (this->GetInput(idx))
    {
      mitk::ImageToImageFilter::InputImagePointer input =
        const_cast< mitk::ImageToImageFilter::InputImageType * > ( this->GetInput(idx) );

      // Use the function object RegionCopier to copy the output region
      // to the input.  The default region copier has default implementations
      // to handle the cases where the input and output are the same
      // dimension, the input a higher dimension than the output, and the
      // input a lower dimension than the output.
      InputImageRegionType inputRegion;
      //      this->CallCopyRegion(inputRegion, this->GetOutput()->GetRequestedRegion()); @FIXME  ??
      //      input->SetRequestedRegion( inputRegion ); @FIXME ??
      input->SetRequestedRegion( this->GetOutput() ); // ersatz. @FIXME ??
    }
  }
}

void mitk::ImageToImageFilter::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
