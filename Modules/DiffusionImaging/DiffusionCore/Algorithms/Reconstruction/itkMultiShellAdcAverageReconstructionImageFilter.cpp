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
  this->SetNumberOfThreads(1);
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

  //MITK_INFO << m_allDirectionsSize;
  //exit(0);

  m_TargetGradientDirections = mitk::gradients::CreateNormalizedUniqueGradientDirectionContainer(m_BValueMap,m_OriginalGradientDirections);
  //MITK_INFO << m_allDirectionsSize;

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

      //- get TragetSHBasis using allDirectionsContainer
      vnl_matrix<double> sphericalCoordinates;
      sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(m_allDirectionsIndicies, m_OriginalGradientDirections);
      vnl_matrix<double> TargetSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
      MITK_INFO << "TargetSHBasis " << TargetSHBasis.rows() << " x " << TargetSHBasis.cols();
      //- get ShellSHBasis using currentShellDirections
      sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(currentShell, m_OriginalGradientDirections);
      vnl_matrix<double> ShellSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
      MITK_INFO << "ShellSHBasis " << ShellSHBasis.rows() << " x " << ShellSHBasis.cols();
      //- calculate interpolationSHBasis [TargetSHBasis * ShellSHBasis^-1]
      vnl_matrix_inverse<double> invShellSHBasis(ShellSHBasis);
      vnl_matrix<double> shellInterpolationMatrix = TargetSHBasis * invShellSHBasis.pinverse();
      MITK_INFO << "shellInterpolationMatrix " << shellInterpolationMatrix.rows() << " x " << shellInterpolationMatrix.cols();
      m_ShellInterpolationMatrixVector.push_back(shellInterpolationMatrix);



      //- save interpolationSHBasis

    }

  }
  m_WeightsVector.reserve(m_BValueMap.size()-1);
  BValueMap::const_iterator itt = m_BValueMap.begin();
  itt++; // skip ReferenceImages
  //- calculate Weights [Weigthing = shell_size / max_shell_size]
  unsigned int maxShellSize = 0;
  for(;itt != m_BValueMap.end(); itt++){
    if(itt->second.size() > maxShellSize)
      maxShellSize = itt->second.size();
  }
  itt = m_BValueMap.begin();
  itt++; // skip ReferenceImages
  for(;itt != m_BValueMap.end(); itt++){
    m_WeightsVector.push_back(itt->second.size() / (double)maxShellSize);
  }


  // calculate average b-Value for target b-Value [bVal_t]
  // MITK_INFO << "overall referenceImage avareg";



  // initialize output image
  typename OutputImageType::Pointer outImage = static_cast<OutputImageType * >(ProcessObject::GetOutput(0));
  //outImage = OutputImageType::New();
  outImage->SetSpacing( this->GetInput()->GetSpacing() );
  outImage->SetOrigin( this->GetInput()->GetOrigin() );
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( 1+m_allDirectionsSize ); // size of 1(bzeroValue) + AllDirectionsContainer
  outImage->Allocate();

  BValueMap::iterator it = m_BValueMap.begin();
  it++; // skip bZeroImages corresponding to 0-bValue
  m_TargetBValue = 0;
  while(it!=m_BValueMap.end())
  {
    m_TargetBValue += it->first;
    it++;
  }
  m_TargetBValue /= (double)(m_BValueMap.size()-1);


  MITK_INFO << "Input:" << std::endl << std::endl
            << "    GradientDirections: " << m_OriginalGradientDirections->Size() << std::endl
            << "    Shells: " << (m_BValueMap.size() - 1) << std::endl
            << "    ReferenceImages: " << m_BValueMap.at(0.0).size() << std::endl
            << "    Interpolation: " << m_Interpolation << std::endl;

  MITK_INFO << "Output:" << std::endl << std::endl
            << "    OutImageVectorLength: " << outImage->GetVectorLength() << std::endl
            << "    TargetDirections: " << m_allDirectionsSize << std::endl
            << "    TargetBValue: " << m_TargetBValue << std::endl
            << std::endl;

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

  // calculate target bZero-Value [b0_t]
  const IndicesVector BZeroIndices = m_BValueMap[0.0];
  double BZeroAverage = 0.0;

  // create empty nxm SignalMatrix containing n->signals/directions (in case of interpolation ~ sizeAllDirections otherwise the size of any shell) for m->shells
  vnl_matrix<double> SignalMatrix(m_allDirectionsSize, m_BValueMap.size()-1);
  // create nx1 targetSignalVector
  vnl_vector<double> SignalVector(m_allDirectionsSize);


  // ** walking over each Voxel
  while(!iit.IsAtEnd())
  {
    InputPixelType b = iit.Get();

    for(unsigned int i = 0 ; i < BZeroIndices.size(); i++)
      BZeroAverage += b[BZeroIndices[i]];
    BZeroAverage /= (double)BZeroIndices.size();

    OutputPixelType out = oit.Get();
    out.Fill(0.0);
    out.SetElement(0,BZeroAverage);

    BValueMap::const_iterator shellIterator = m_BValueMap.begin();
    shellIterator++; //skip bZeroImages
    unsigned int shellIndex = 0;

    while(shellIterator != m_BValueMap.end())
    {

      // reset Data
      SignalVector.fill(0.0);

      // - get the RawSignal
      const IndicesVector currentShell = shellIterator->second;
      vnl_vector<double> InterpVector(currentShell.size());

      // - get raw Signal for currente shell
      for(unsigned int i = 0 ; i < currentShell.size(); i++)
        InterpVector.put(i,b[currentShell[i]]);
      MITK_INFO <<"RawSignal: "<< InterpVector;

      //- normalization of the raw Signal
      S_S0Normalization(InterpVector, BZeroAverage);
      MITK_INFO <<"Normalized: "<< InterpVector;

      //- interpolate the Signal if necessary using corresponding interpolationSHBasis
      if(m_Interpolation)
        SignalVector = m_ShellInterpolationMatrixVector.at(shellIndex) * InterpVector;
      else
        SignalVector = InterpVector;
      MITK_INFO <<"Interpolated: "<< SignalVector;

      // - ADC calculation for the signalVector
      calculateAdcFromSignal(SignalVector, shellIterator->first);
      MITK_INFO << "ADCVector: " << SignalVector;


      //- weight the signal
      //if(m_Interpolation){
      //  const double shellWeight = m_WeightsVector.at(shellIndex);
      //  SignalVector *= shellWeight;
      //}

      //- save the (interpolated) ShellSignalVector as the ith column in the SignalMatrix
      SignalMatrix.set_column(shellIndex, SignalVector);

      shellIterator++;
      shellIndex++;
      //MITK_INFO << SignalMatrix;
    }
    exit(0);
    // MITK_INFO <<"finalSignalMatrix: " << SignalMatrix;
    // ADC averaging Sum(ADC)/n
    for(unsigned int i = 0 ; i < SignalMatrix.rows(); i++)
      SignalVector.put(i,SignalMatrix.get_row(i).sum());
    SignalVector/= (double)(m_BValueMap.size()-1);
    // MITK_INFO << "AveragedADC: " << SignalVector;
    // recalculate signal from ADC
    calculateSignalFromAdc(SignalVector, m_TargetBValue, BZeroAverage);

    for(unsigned int i = 1 ; i < out.Size(); i ++)
      out.SetElement(i,SignalVector.get(i-1));

    //MITK_INFO << "ADCAveragedSignal: " << out;
    //MITK_INFO << "-------------";
    oit.Set(out);
    ++oit;
    ++iit;
  }


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
::calculateSignalFromAdc(vnl_vector<double> & vec, const double & bValue, const double & referenceSignal)
{
  for(unsigned int i = 0 ; i < vec.size(); i++){
    //MITK_INFO << vec[i];
    //MITK_INFO << bValue;
    //MITK_INFO << referenceSignal;
    vec[i] = referenceSignal * exp((-bValue) * vec[i]);
  }
}

} // end of namespace
