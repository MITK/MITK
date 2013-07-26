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
#ifndef _itk_MultiShellDirectionalKurtosisFitImageFilter_cpp_
#define _itk_MultiShellDirectionalKurtosisFitImageFilter_cpp_
#endif

#define _USE_MATH_DEFINES

#include "itkMultiShellRadialAdcKurtosisImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegion.h>
#include "mitkDiffusionFunctionCollection.h"
#include "vnl/vnl_least_squares_function.h"
#include "vnl/algo/vnl_levenberg_marquardt.h"

namespace itk
{


/** baseclass for IVIM fitting algorithms */
struct lestSquaresFunction: public vnl_least_squares_function
{

  void set_measurements(const vnl_vector<double>& x)
  {
    measurements.set_size(x.size());
    measurements.copy_in(x.data_block());
  }

  void set_bvalues(const vnl_vector<double>& x)
  {
    b.set_size(x.size());
    b.copy_in(x.data_block());
  }

  void set_reference_measuremnt(const double & x)
  {
    reference_measurement = x;
  }

  double reference_measurement;
  vnl_vector<double> measurements;
  vnl_vector<double> b;

  int N;

  lestSquaresFunction(unsigned int number_of_measurements) :
    vnl_least_squares_function(2, number_of_measurements, no_gradient)
  {
    N = get_number_of_residuals();
  }

  void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

    const double & D = x[0];
    const double & K = x[1];

    for(int s=0; s<N; s++)
    {
      double approx = std::log(reference_measurement)  - b[s] * D + 1./6.*b[s]*b[s]*D*D*K;
      fx[s] = vnl_math_abs( std::log(measurements[s]) - approx );
    }

  }
};


template <class TInputScalarType, class TOutputScalarType>
MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::MultiShellRadialAdcKurtosisImageFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
  //this->SetNumberOfThreads(1);
}

template <class TInputScalarType, class TOutputScalarType>
void MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::BeforeThreadedGenerateData()
{

  // test whether BvalueMap contains all necessary information
  if(m_B_ValueMap.size() == 0)
  {
    itkWarningMacro(<< "No BValueMap given: create one using GradientDirectionContainer");

    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = m_OriginalGradientDirections->Begin(); gdcit != m_OriginalGradientDirections->End(); ++gdcit)
    {
      double bValueKey = int(((m_B_Value * gdcit.Value().two_norm() * gdcit.Value().two_norm())+7.5)/10)*10;
      m_B_ValueMap[bValueKey].push_back(gdcit.Index());
    }
  }

  //# BValueMap contains no bZero --> itkException
  if(m_B_ValueMap.find(0.0) == m_B_ValueMap.end())
  {
    MITK_INFO << "No ReferenceSignal (BZeroImages) found!";
    itkExceptionMacro(<< "No ReferenceSignal (BZeroImages) found!");
  }

  // [allDirectionsContainer] Gradient DirectionContainer containing all unique directions
  m_allDirectionsIndicies = mitk::gradients::GetAllUniqueDirections(m_B_ValueMap, m_OriginalGradientDirections);
  // [sizeAllDirections] size of GradientContainer cointaining all unique directions
  m_allDirectionsSize = m_allDirectionsIndicies.size();
  m_TargetGradientDirections = mitk::gradients::CreateNormalizedUniqueGradientDirectionContainer(m_B_ValueMap,m_OriginalGradientDirections);

  m_ShellInterpolationMatrixVector.reserve(m_B_ValueMap.size()-1);

  m_bValueVector = vnl_vector<double>(2);

  // for each shell
  BValueMap::const_iterator it = m_B_ValueMap.begin();
  it++; //skip bZeroIndices

  unsigned int shellIndex = 0;

  for(;it != m_B_ValueMap.end();++it)
  {
    //- calculate maxShOrder
    const IndicesVector currentShell = it->second;
    unsigned int SHMaxOrder = 12;
    while( ((SHMaxOrder+1)*(SHMaxOrder+2)/2) > currentShell.size() && ((SHMaxOrder+1)*(SHMaxOrder+2)/2) >= 4 )
    {
      SHMaxOrder -= 2 ;
    }
    //- get TragetSHBasis using allDirectionsContainer
    vnl_matrix<double> sphericalCoordinates;
    sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(m_allDirectionsIndicies, m_OriginalGradientDirections);
    vnl_matrix<double> TargetSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
    //MITK_INFO << "TargetSHBasis " << TargetSHBasis.rows() << " x " << TargetSHBasis.cols();
    //- get ShellSHBasis using currentShellDirections
    sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(currentShell, m_OriginalGradientDirections);
    vnl_matrix<double> ShellSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
    //MITK_INFO << "ShellSHBasis " << ShellSHBasis.rows() << " x " << ShellSHBasis.cols();
    //- calculate interpolationSHBasis [TargetSHBasis * ShellSHBasis^-1]
    vnl_matrix_inverse<double> invShellSHBasis(ShellSHBasis);
    vnl_matrix<double> shellInterpolationMatrix = TargetSHBasis * invShellSHBasis.pinverse();
    //MITK_INFO << "shellInterpolationMatrix " << shellInterpolationMatrix.rows() << " x " << shellInterpolationMatrix.cols();
    //- save interpolationSHBasis
    m_ShellInterpolationMatrixVector.push_back(shellInterpolationMatrix);

    m_bValueVector.put(shellIndex,it->first);

    ++shellIndex;

  }


  m_WeightsVector.reserve(m_B_ValueMap.size()-1);
  BValueMap::const_iterator itt = m_B_ValueMap.begin();
  itt++; // skip ReferenceImages
  //- calculate Weights [Weigthing = shell_size / max_shell_size]
  unsigned int maxShellSize = 0;
  for(;itt != m_B_ValueMap.end(); itt++){
    if(itt->second.size() > maxShellSize)
      maxShellSize = itt->second.size();
  }
  itt = m_B_ValueMap.begin();
  itt++; // skip ReferenceImages
  for(;itt != m_B_ValueMap.end(); itt++){
    m_WeightsVector.push_back(itt->second.size() / (double)maxShellSize);
    MITK_INFO << m_WeightsVector.back();
  }


  // initialize output image
  typename OutputImageType::Pointer outImage = static_cast<OutputImageType * >(ProcessObject::GetOutput(0));
  outImage->SetSpacing( this->GetInput()->GetSpacing() );
  outImage->SetOrigin( this->GetInput()->GetOrigin() );
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( 1+m_allDirectionsSize ); // size of 1(bzeroValue) + AllDirectionsContainer
  outImage->Allocate();

  BValueMap::iterator ittt = m_B_ValueMap.begin();
  ittt++; // skip bZeroImages corresponding to 0-bValue
  m_TargetB_Value = 0;
  while(ittt!=m_B_ValueMap.end())
  {
    m_TargetB_Value += ittt->first;
    ittt++;
  }
  m_TargetB_Value /= (double)(m_B_ValueMap.size()-1);


  MITK_INFO << "Input:" << std::endl << std::endl
            << "    GradientDirections: " << m_OriginalGradientDirections->Size() << std::endl
            << "    Shells: " << (m_B_ValueMap.size() - 1) << std::endl
            << "    ReferenceImages: " << m_B_ValueMap[0.0].size() << std::endl;

  MITK_INFO << "Output:" << std::endl << std::endl
            << "    OutImageVectorLength: " << outImage->GetVectorLength() << std::endl
            << "    TargetDirections: " << m_allDirectionsSize << std::endl
            << "    TargetBValue: " << m_TargetB_Value << std::endl
            << std::endl;

}

template <class TInputScalarType, class TOutputScalarType>
void
MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType /*threadId*/)
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
  const IndicesVector BZeroIndices = m_B_ValueMap[0.0];
  double BZeroAverage = 0.0;

  unsigned int numberOfShells = m_B_ValueMap.size()-1;

  // create empty nxm SignalMatrix containing n->signals/directions (in case of interpolation ~ sizeAllDirections otherwise the size of any shell) for m->shells
  vnl_matrix<double> SignalMatrix(m_allDirectionsSize, numberOfShells);
  // create nx1 targetSignalVector

  vnl_vector<double> SignalVector(m_allDirectionsSize);
  OutputPixelType out;
  InputPixelType b;
  BValueMap::const_iterator shellIterator;
  vnl_matrix<double> lsfCoeffs;
  vnl_vector<double> InterpVector;

  // ** walking over each Voxel
  while(!iit.IsAtEnd())
  {
    b = iit.Get();
    BZeroAverage=0.0;
    for(unsigned int i = 0 ; i < BZeroIndices.size(); i++){
      BZeroAverage += b[BZeroIndices[i]];
    }
    BZeroAverage /= (double)BZeroIndices.size();
    out = oit.Get();
    out.Fill(0.0);
    out.SetElement(0,BZeroAverage);

    shellIterator = m_B_ValueMap.begin();
    shellIterator++; //skip bZeroImages

    unsigned int shellIndex = 0;

    while(shellIterator != m_B_ValueMap.end())
    {
      // reset Data
      SignalVector.fill(0.0);

      // - get the RawSignal
      const IndicesVector & currentShell = shellIterator->second;
      InterpVector.set_size(currentShell.size());

      // - get raw Signal for currente shell
      for(unsigned int i = 0 ; i < currentShell.size(); i++)
        InterpVector.put(i,b[currentShell[i]]);

      //- interpolate the Signal using corresponding interpolationSHBasis
      SignalVector = m_ShellInterpolationMatrixVector.at(shellIndex) * InterpVector;
      SignalMatrix.set_column(shellIndex, SignalVector);

      shellIterator++;
      shellIndex++;
    }

    lsfCoeffs.set_size(m_allDirectionsSize , 2);
    calculateCoeffs(lsfCoeffs,SignalMatrix, m_bValueVector, BZeroAverage);
    calculateSignalFromLsfCoeffs(SignalVector,lsfCoeffs,m_TargetB_Value,BZeroAverage);

    for(unsigned int i = 1 ; i < out.Size(); i ++)
      out.SetElement(i,SignalVector.get(i-1));

    oit.Set(out);
    ++oit;
    ++iit;
  }
}

template <class TInputScalarType, class TOutputScalarType>
void MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::logarithm( vnl_vector<double> & vec)
{
  for(unsigned int i = 0; i < vec.size(); i++){
    if(vec[i] <= 0.0) vec[i] = 0.00001;
    vec[i] = std::log( vec[i] );
  }
}


template <class TInputScalarType, class TOutputScalarType>
void MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::calculateCoeffs(vnl_matrix<double> &lsfCoeffs, const vnl_matrix<double> & SignalMatrix, const vnl_vector<double> & bValueVector, const double & reference_b_value)
{
  vnl_vector<double> initalGuess(2);
  // initialize Least Squres Function
  lestSquaresFunction model(bValueVector.size());
  // initialize Levenberg Marquardt
  vnl_levenberg_marquardt minimizer(model);
  minimizer.set_max_function_evals(50000); // Iterations
  minimizer.set_f_tolerance(1e-10);        // Function tolerance
  minimizer.set_epsilon_function(1e-10);   // Epsilon

  // for each Direction calculate LSF Coeffs ADC & AKC
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); i++)
  {
    // set Signal Vector
    model.set_measurements(SignalMatrix.get_row(i));
    // set BValue Vector e.g.: [1000, 2000, 3000]
    model.set_bvalues(bValueVector);
    // set BZero Value
    model.set_reference_measuremnt(reference_b_value);

    // Start Vector [ADC, AKC]
    initalGuess.put(0, 0.f);
    initalGuess.put(1, 0.f);

    // start Levenberg-Marquardt
    bool status = minimizer.minimize_without_gradient(initalGuess);

    if(!status)
    {
      MITK_INFO<< "______________________________";
      MITK_INFO<< "Minimizer f Error: " << minimizer.get_f_tolerance();
      MITK_INFO<< "Minimizer end Error: " << minimizer.get_end_error();
      MITK_INFO<< "Minimizer Evaluations: " << minimizer.get_num_evaluations();
      MITK_INFO<< "Minimizer Iterations: " << minimizer.get_num_iterations();
      MITK_INFO<< "______________________________";
    }else{

      std::cout << std::scientific << std::setprecision(5)
                << reference_b_value << ";"   // BZero
                << initalGuess[0] << ";"      // ADC
                << initalGuess[1] << ";";     // AKC
      for(unsigned int j = 0; j < SignalMatrix.get_row(i).size(); j++ )
       std::cout << std::scientific << std::setprecision(5)
                 << SignalMatrix.get_row(i)[j] << ";";
      std::cout << std::endl;
    }
    // Set Coeffs for each gradient direction
    lsfCoeffs.set_row(i, initalGuess);
  }
}


template <class TInputScalarType, class TOutputScalarType>
void MultiShellRadialAdcKurtosisImageFilter<TInputScalarType, TOutputScalarType>
::calculateSignalFromLsfCoeffs(vnl_vector<double> & vec, const vnl_matrix<double> & lsfCoeffs, const double &bValue, const double & referenceSignal)
{
  // For each Direction
  for(unsigned int i = 0 ; i < lsfCoeffs.rows();i++){
    const double & D = lsfCoeffs(i,0);
    const double & K = lsfCoeffs(i,1);

    vec[i] = referenceSignal * exp( -bValue * D + 1./6.* bValue * bValue * D * D * K);

    //    std::cout << "OUTVAL = " << i << std::endl;
  }
}


} // end of namespace
