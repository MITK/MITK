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
void MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::BeforeThreadedGenerateData()
{
  // test whether interpolation is necessary
  // - Gradeint directions on different shells are different
  //# if interpolation is neccesary and any #ShellDirection < 15 --> itkException (No interpolation possible)

  // test whether BvalueMap contains all necessary information
  //# BValueMapSize == 0 --> itkWarning (take BValueMap from inputImage)
  //# BValueMap contains no bZero --> itkException

  // if INTERPOLATION necessary
  // [allDirectionsContainer] Gradient DirectionContainer containing all unique directions
  // [sizeAllDirections] size of GradientContainer cointaining all unique directions
  /* for each shell
   * - calculate maxShOrder
   * - calculate Weights [Weigthing = shell_size / max_shell_size]
   * - get TragetSHBasis using allDirectionsContainer
   * - get ShellSHBasis using currentShellDirections
   * - calculate interpolationSHBasis [TargetSHBasis * ShellSHBasis^-1]
   * - save interpolationSHBasis
   */

  // calculate average b-Value for target b-Value [bVal_t]
  // calculate target bZero-Value [b0_t]

}

template <class TInputScalarType, class TOutputScalarType>
void
MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, int /*threadId*/)
{

  // Get input gradient image pointer
  typename InputImageType::Pointer inputImage = static_cast< InputImageType * >(ProcessObject::GetInput(0));
  // ImageRegionIterator for the input image
  ImageRegionIterator< InputImageType > iit(inputImage, outputRegionForThread);
  iit.GoToBegin();

  // Get output gradient image pointer
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(ProcessObject::GetOutput(0));
  // ImageRegionIterator for the output image
  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  const int numShells = m_BValueMap.size()-1;
  BValueMap::iterator it = m_BValueMap.begin();
  //std::vector<double> adcVec = new Vector<double>(numShells);


  // create empty nxm SignalMatrix containing n->signals/directions (in case of interpolation ~ sizeAllDirections otherwise the size of any shell) for m->shells
  // create nx1 targetSignalVector
  // ** walking over each Voxel
  /* for each shell in this Voxel
   * - get the RawSignal
   * - interpolate the Signal if necessary using corresponding interpolationSHBasis
   * - save the (interpolated) ShellSignalVector as the ith column in the SignalMatrix
   */

  // normalize the signals in SignalMatrix [bZeroAverage????] [S/S0 = E]

  /* for each row in the SignalMatrix
   * - calculate for each rowentry the ADC [ln(E)/-b = ADC]
   * - weight the ith ADC entry with the corresponding shellWeighting
   * - average the Signal over the row [ADC_t]
   * - calculate target Signal using target b-Value [S_t = b0_t * e^(-bVal_t * ADC_t)
   * - Save S_t in targetSignalVector
   */

  // outImageIterator set S_t

  // **

  int vecLength;

  // initialize output image
  typename OutputImageType::Pointer outImage = OutputImageType::New();
  outImage->SetSpacing( this->GetInput()->GetSpacing() );
  outImage->SetOrigin( this->GetInput()->GetOrigin() );
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( vecLength ); // size of 1(bzeroValue) + AllDirectionsContainer
  outImage->Allocate();


  this->SetNumberOfRequiredOutputs (1);
  this->SetNthOutput (0, outImage);
  MITK_INFO << "...done";
}



} // end of namespace
