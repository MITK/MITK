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


#include "mitkImageToPlanarFigureFilter.h"

mitk::ImageToPlanarFigureFilter::ImageToPlanarFigureFilter()
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);
}

mitk::ImageToPlanarFigureFilter::~ImageToPlanarFigureFilter()
{
}

/**
*
*/
void mitk::ImageToPlanarFigureFilter::SetInput(const mitk::ImageToPlanarFigureFilter::InputImageType *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::ImageToPlanarFigureFilter::InputImageType * >( input ) );
}


/**
* Connect one of the operands for pixel-wise addition
*/
void mitk::ImageToPlanarFigureFilter::SetInput( unsigned int index, const mitk::ImageToPlanarFigureFilter::InputImageType * image ) 
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index, 
    const_cast< mitk::ImageToPlanarFigureFilter::InputImageType *>( image ) );
}



/**
*
*/
const mitk::ImageToPlanarFigureFilter::InputImageType *mitk::ImageToPlanarFigureFilter::GetInput(void) 
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::ImageToPlanarFigureFilter::InputImageType * >
    (this->ProcessObject::GetInput(0) );
}

/**
*
*/
const mitk::ImageToPlanarFigureFilter::InputImageType *mitk::ImageToPlanarFigureFilter::GetInput(unsigned int idx)
{
  return static_cast< const mitk::ImageToPlanarFigureFilter::InputImageType * >
    (this->ProcessObject::GetInput(idx));
}



//-----------------------------------------------------------------------
//
void mitk::ImageToPlanarFigureFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  for (unsigned int idx = 0; idx < this->GetNumberOfInputs(); ++idx)
  {
    if (this->GetInput(idx))
    {
      mitk::ImageToPlanarFigureFilter::InputImagePointer input =
        const_cast< mitk::ImageToPlanarFigureFilter::InputImageType * > ( this->GetInput(idx) );

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

void mitk::ImageToPlanarFigureFilter::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
