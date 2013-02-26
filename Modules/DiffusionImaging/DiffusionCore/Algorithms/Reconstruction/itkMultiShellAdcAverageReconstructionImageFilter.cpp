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
#include "mitkDiffusionFunctionCollection.h"

namespace itk
{

template <class TInputScalarType, class TOutputScalarType>
MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::MultiShellAdcAverageReconstructionImageFilter():
  m_Interpolation(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template <class TInputScalarType, class TOutputScalarType>
void MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::BeforeThreadedGenerateData()
{

  // test whether BvalueMap contains all necessary information
  if(m_BValueMap.size() == 0)
  {
    itkWarningMacro(<< "No BValueMap given: create one using GradientDirectionContainer");

    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = m_OriginalGradientDirections->Begin(); gdcit != m_OriginalGradientDirections->End(); ++gdcit)
    {
      double bValueKey = int(((m_BValue * gdcit.Value().two_norm() * gdcit.Value().two_norm())+7.5)/10)*10;
      m_BValueMap[bValueKey].push_back(gdcit.Index());
    }
  }

  //# BValueMap contains no bZero --> itkException
  if(m_BValueMap.find(0.0) == m_BValueMap.end())
  {
    MITK_INFO << "No ReferenceSignal (BZeroImages) found!";
    itkExceptionMacro(<< "No ReferenceSignal (BZeroImages) found!");
  }

  // test whether interpolation is necessary
  // - Gradeint directions on different shells are different
  m_Interpolation = mitk::gradients::CheckForDifferingShellDirections(m_BValueMap, m_OriginalGradientDirections.GetPointer());
  // [allDirectionsContainer] Gradient DirectionContainer containing all unique directions
  m_allDirectionsIndicies = mitk::gradients::GetAllUniqueDirections(m_BValueMap, m_OriginalGradientDirections);
  // [sizeAllDirections] size of GradientContainer cointaining all unique directions
  m_allDirectionsSize = m_allDirectionsIndicies.size();

  // if INTERPOLATION necessary
  if(m_Interpolation)
  {
    for(BValueMap::const_iterator it = m_BValueMap.begin();it != m_BValueMap.end(); it++)
    {
      if((*it).first == 0.0) continue;
      // if any #ShellDirection < 15 --> itkException (No interpolation possible)
      if((*it).second.size() < 15){
        MITK_INFO << "Abort: No interpolation possible Shell-" << (*it).first << " has less than 15 directions.";
        itkExceptionMacro(<<"No interpolation possible");
      }
    }

    m_ShellInterpolationMatrixVector.reserve(m_BValueMap.size()-1);
    // for Weightings
    unsigned int maxShellSize = 0;

    // for each shell
    BValueMap::const_iterator it = m_BValueMap.begin();
    it++; //skip bZeroIndices
    for(;it != m_BValueMap.end();it++)
    {
      //- calculate maxShOrder
      const IndicesVector currentShell = (*it).second;
      unsigned int SHMaxOrder = 12;
      while( ((SHMaxOrder+1)*(SHMaxOrder+2)/2) > currentShell.size())
        SHMaxOrder -= 2 ;

      //- save shell size

      if(currentShell.size() > maxShellSize)
        maxShellSize = currentShell.size();

      //- get TragetSHBasis using allDirectionsContainer
      vnl_matrix<double> sphericalCoordinates;
      sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(m_allDirectionsIndicies, m_OriginalGradientDirections);
      vnl_matrix<double> TargetSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
      //- get ShellSHBasis using currentShellDirections
      sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(currentShell, m_OriginalGradientDirections);
      vnl_matrix<double> ShellSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
      //- calculate interpolationSHBasis [TargetSHBasis * ShellSHBasis^-1]
      vnl_matrix_inverse<double> invShellSHBasis(ShellSHBasis);
      vnl_matrix<double> shellInterpolationMatrix = TargetSHBasis * invShellSHBasis.pinverse();
      m_ShellInterpolationMatrixVector.push_back(shellInterpolationMatrix);
      //- save interpolationSHBasis

    }
    m_WeightsVector.reserve(m_BValueMap.size()-1);
    it = m_BValueMap.begin();
    it++; // skip ReferenceImages
    //- calculate Weights [Weigthing = shell_size / max_shell_size]
    for(;it != m_BValueMap.end(); it++)
      m_WeightsVector.push_back(it->second.size() / maxShellSize);
  }


  // calculate average b-Value for target b-Value [bVal_t]
  const IndicesVector BZeroIndices = m_BValueMap.at(0.0);
  const unsigned int numberOfBZeroImages = BZeroIndices.size();
  if(numberOfBZeroImages % (m_BValueMap.size()-1)  == 0)
  {
    MITK_INFO << "referenceImage avareg for each shell";
    m_bZeroIndicesSplitVectors.reserve(m_BValueMap.size()-1);
    const int stepWidth = BZeroIndices.size() / (m_BValueMap.size()-1);
    IndicesVector::const_iterator it1 = BZeroIndices.begin();
    IndicesVector::const_iterator it2 = BZeroIndices.begin() + stepWidth;
    for(int i = 0 ; i < m_BValueMap.size()-1; i++)
    {
      m_bZeroIndicesSplitVectors.push_back(IndicesVector(it1,it2));
      it1 += stepWidth;
      it2 += stepWidth;
    }
  }else
  {
    MITK_INFO << "overall referenceImage avareg";
    m_bZeroIndicesSplitVectors.reserve(1);
    m_bZeroIndicesSplitVectors.push_back(BZeroIndices);
  }


  // initialize output image
  typename OutputImageType::Pointer outImage = OutputImageType::New();
  outImage->SetSpacing( this->GetInput()->GetSpacing() );
  outImage->SetOrigin( this->GetInput()->GetOrigin() );
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( m_allDirectionsSize ); // size of 1(bzeroValue) + AllDirectionsContainer
  outImage->Allocate();


  MITK_INFO << "Input:" << std::endl
            << "GradientDirections: " << m_OriginalGradientDirections->Size() << std::endl
            << "Shells: " << (m_BValueMap.size() - 1) << std::endl
            << "ReferenceImages: " << m_BValueMap.at(0.0).size() << std::endl
            << "Interpolation: " << m_Interpolation;


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

  // calculate target bZero-Value [b0_t]
  const IndicesVector BZeroIndices = m_BValueMap[0.0];
  double BZeroAverage = 0.0;

  // create empty nxm SignalMatrix containing n->signals/directions (in case of interpolation ~ sizeAllDirections otherwise the size of any shell) for m->shells
  vnl_matrix<double> SignalMatrix(m_allDirectionsSize, m_BValueMap.size());
  // create nx1 targetSignalVector
  vnl_vector<double> SignalVector(m_allDirectionsSize);

  // ** walking over each Voxel
  while(!iit.IsAtEnd())
  {
    InputPixelType b = iit.Get();

    for(int i = 0 ; i < BZeroIndices.size(); i++)
      BZeroAverage += b[BZeroIndices[i]];
    BZeroAverage /= BZeroIndices.size();

    OutputPixelType out = oit.Get();
    //out.AllocateElements(m_allDirectionsSize + 1);
    out.Fill(0.0);
    out.SetElement(0,BZeroAverage);

    BValueMap::const_iterator shellIterator = m_BValueMap.begin();
    shellIterator++; //skip bZeroImages
    unsigned int shellIndex = 0;
    bool multipleBValues = m_bZeroIndicesSplitVectors.size() > 1;


    while(shellIterator != m_BValueMap.end())
    {
      // reset Data
      SignalVector.fill(0.0);

      // - get the RawSignal
      const IndicesVector currentShell = shellIterator->second;
      for(int i = 0 ; i < currentShell.size(); i++)
        SignalVector.put(i,b[currentShell[i]]);

      //- interpolate the Signal if necessary using corresponding interpolationSHBasis
      if(m_Interpolation)
        SignalVector *= m_ShellInterpolationMatrixVector.at(shellIndex);

      //- normalization of the raw Signal
      if(multipleBValues)
        S_S0Normalization(SignalVector, BZeroAverage);
      else{
        double shellBZeroAverage = 0;
        for(unsigned int i = 0 ; i < m_bZeroIndicesSplitVectors.at(shellIndex).size(); i++)
          shellBZeroAverage += b[m_bZeroIndicesSplitVectors.at(shellIndex).at(i)];
        S_S0Normalization(SignalVector, shellBZeroAverage/(double)m_bZeroIndicesSplitVectors.at(shellIndex).size());
      }

      calculateAdcFromSignal(SignalVector, shellIterator->first);

      //- weight the signal
      if(m_Interpolation){
        const double shellWeight = m_WeightsVector.at(shellIndex);
        SignalVector *= shellWeight;
      }
      //- save the (interpolated) ShellSignalVector as the ith column in the SignalMatrix
      SignalMatrix.set_column(shellIndex, SignalVector);

      shellIterator++;
      shellIndex++;
    }

    for(int i = 0 ; i < SignalMatrix.rows(); i++)
      SignalVector.put(i,SignalMatrix.get_row(i).sum());

    SignalVector/= SignalMatrix.cols();

    calculateSignalFromAdc(SignalVector,,BZeroAverage)

    for(int i = 1 ; i < out.Size(); i ++)
      out.SetElement(i,SignalVector.get(i-1));


    oit.Set(out);
    ++oit;
    ++iit;
  }






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
  /*
  int vecLength;




  this->SetNumberOfRequiredOutputs (1);
  this->SetNthOutput (0, outImage);
  MITK_INFO << "...done";
  */
}
template <class TInputScalarType, class TOutputScalarType>
void MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::S_S0Normalization( vnl_vector<double> & vec, const double & S0 )
{
  for(unsigned int i = 0; i < vec.size(); i++)
    vec[i] /= S0;
}

template <class TInputScalarType, class TOutputScalarType>
void MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::calculateAdcFromSignal( vnl_vector<double> & vec, const double & bValue)
{
  for(unsigned int i = 0; i < vec.size(); i++)
    vec[i] = log(vec[i])/-bValue;
}

template <class TInputScalarType, class TOutputScalarType>
void MultiShellAdcAverageReconstructionImageFilter<TInputScalarType, TOutputScalarType>
::calculateSignalFromAdc( vnl_vector<double> & vec, const double & bValue, const double & bZero)
{
  for(unsigned int i = 0 ; i < vec.size(); i++)
    vec[i] = bZero * exp(-bValue * vec[i])
}


} // end of namespace
