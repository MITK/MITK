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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_MultiShellAdcAverageReconstructionImageFilter_cpp_
#define _itk_MultiShellAdcAverageReconstructionImageFilter_cpp_
#endif

#define _USE_MATH_DEFINES

#include "itkMultiShellAdcAverageReconstructionImageFilter.h"
#include <math.h>
#include <time.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegion.h>

namespace itk
{

template <class TInputScalarType, class TOutputScalarType>
MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::MultiShellAdcAverageReconstructionImageFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

template <class TInputScalarType, class TOutputScalarType>
void
MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, int /*threadId*/)
{

  // Get input gradient image pointer
  typename InputImageType::Pointer inputImage = static_cast< InputImageType * >(ProcessObject::GetInput(0));
  // ImageRegionIterator for the input image
  ImageRegionConstIterator< InputImageType > iit(inputImage, outputRegionForThread);
  iit.GoToBegin();

  // Get output gradient image pointer
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(ProcessObject::GetOutput(0));
  // ImageRegionIterator for the output image
  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  const int numShells = m_BValueMap.size()-1;
  BValueMap::iterator it = m_BValueMap.begin();
  std::vector<double> adcVec = new Vector<double>(numShells);

  // initialize output image
  typename OutputImageType::Pointer outImage = OutputImageType::New();
  outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
  outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( vecLength ); // Set the vector length
  outImage->Allocate();


  this->SetNumberOfRequiredOutputs (1);
  this->SetNthOutput (0, outImage);
  MITK_INFO << "...done";
}



} // end of namespace
