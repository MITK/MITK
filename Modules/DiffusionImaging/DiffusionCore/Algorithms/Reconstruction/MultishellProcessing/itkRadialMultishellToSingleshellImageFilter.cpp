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
#ifndef _itk_RadialMultishellToSingleshellImageFilter_cpp_
#define _itk_RadialMultishellToSingleshellImageFilter_cpp_
#endif

#define _USE_MATH_DEFINES

#include "itkRadialMultishellToSingleshellImageFilter.h"
#include "mitkDiffusionFunctionCollection.h"


namespace itk
{


template <class TInputScalarType, class TOutputScalarType>
RadialMultishellToSingleshellImageFilter<TInputScalarType, TOutputScalarType>
::RadialMultishellToSingleshellImageFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
  //this->SetNumberOfThreads(1);
}

template <class TInputScalarType, class TOutputScalarType>
void RadialMultishellToSingleshellImageFilter<TInputScalarType, TOutputScalarType>
::BeforeThreadedGenerateData()
{
  // test whether BvalueMap contains all necessary information
  if(m_BValueMap.size() == 0)
  {
    itkWarningMacro(<< "No BValueMap given: create one using GradientDirectionContainer");

    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = m_OriginalGradientDirections->Begin(); gdcit != m_OriginalGradientDirections->End(); ++gdcit)
    {
      double bValueKey = int(((m_OriginalBValue * gdcit.Value().two_norm() * gdcit.Value().two_norm())+7.5)/10)*10;
      m_BValueMap[bValueKey].push_back(gdcit.Index());
    }
  }

  //# BValueMap contains no bZero --> itkException
  if(m_BValueMap.find(0.0) == m_BValueMap.end())
  {
    MITK_INFO << "No ReferenceSignal (BZeroImages) found!";
    itkExceptionMacro(<< "No ReferenceSignal (BZeroImages) found!");
  }

  // [allDirectionsContainer] Gradient DirectionContainer containing all unique directions
  m_TargetDirectionsIndicies = mitk::gradients::GetAllUniqueDirections(m_BValueMap, m_OriginalGradientDirections);
  // [sizeAllDirections] size of GradientContainer cointaining all unique directions
  m_NumberTargetDirections = m_TargetDirectionsIndicies.size();
  m_TargetGradientDirections = mitk::gradients::CreateNormalizedUniqueGradientDirectionContainer(m_BValueMap,m_OriginalGradientDirections);

  m_ShellInterpolationMatrixVector.reserve(m_BValueMap.size()-1);

  // for each shell
  BValueMap::const_iterator it = m_BValueMap.begin();
  it++; //skip bZeroIndices

  unsigned int shellIndex = 0;
  for(;it != m_BValueMap.end();++it)
  {
    //- calculate maxShOrder
    const IndicesVector currentShell = it->second;
    unsigned int SHMaxOrder = 12;
    while( ((SHMaxOrder+1)*(SHMaxOrder+2)/2) > currentShell.size() && ((SHMaxOrder+1)*(SHMaxOrder+2)/2) >= 4 )
          SHMaxOrder -= 2 ;

    //- get TragetSHBasis using allDirectionsContainer
    vnl_matrix<double> sphericalCoordinates;
    sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(m_TargetDirectionsIndicies, m_OriginalGradientDirections);
    vnl_matrix<double> TargetSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
    //- get ShellSHBasis using currentShellDirections
    sphericalCoordinates = mitk::gradients::ComputeSphericalFromCartesian(currentShell, m_OriginalGradientDirections);
    vnl_matrix<double> ShellSHBasis = mitk::gradients::ComputeSphericalHarmonicsBasis(sphericalCoordinates, SHMaxOrder);
    //- calculate interpolationSHBasis [TargetSHBasis * ShellSHBasis^-1]
    vnl_matrix_inverse<double> invShellSHBasis(ShellSHBasis);
    vnl_matrix<double> shellInterpolationMatrix = TargetSHBasis * invShellSHBasis.pinverse();
    //- save interpolationSHBasis
    m_ShellInterpolationMatrixVector.push_back(shellInterpolationMatrix);

    ++shellIndex;
  }

  // initialize output image
  typename OutputImageType::Pointer outImage = static_cast<OutputImageType * >(ProcessObject::GetOutput(0));
  outImage->SetSpacing( this->GetInput()->GetSpacing() );
  outImage->SetOrigin( this->GetInput()->GetOrigin() );
  outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  outImage->SetVectorLength( 1+m_NumberTargetDirections ); // size of 1(bzeroValue) + AllDirectionsContainer
  outImage->Allocate();

  m_ErrorImage = ErrorImageType::New();
  m_ErrorImage->SetSpacing( this->GetInput()->GetSpacing() );
  m_ErrorImage->SetOrigin( this->GetInput()->GetOrigin() );
  m_ErrorImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction using bZeroDirection+AllDirectionsContainer
  m_ErrorImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
  m_ErrorImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
  m_ErrorImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
  m_ErrorImage->Allocate();

  MITK_INFO << "Input:" << std::endl << std::endl
            << "    GradientDirections: " << m_OriginalGradientDirections->Size() << std::endl
            << "    Shells: " << (m_BValueMap.size() - 1) << std::endl
            << "    ReferenceImages: " << m_BValueMap[0.0].size() << std::endl;

  MITK_INFO << "Output:" << std::endl << std::endl
            << "    OutImageVectorLength: " << outImage->GetVectorLength() << std::endl
            << "    TargetDirections: " << m_NumberTargetDirections << std::endl
            << std::endl;

}

template <class TInputScalarType, class TOutputScalarType>
void
RadialMultishellToSingleshellImageFilter<TInputScalarType, TOutputScalarType>
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

  // ImageRegionIterator for the output image
  ImageRegionIterator< ErrorImageType > eit(m_ErrorImage, outputRegionForThread);
  eit.GoToBegin();

  // calculate target bZero-Value [b0_t]
  const IndicesVector IndicesS0 = m_BValueMap[0.0];
  double AverageS0 = 0.0;

  unsigned int numberOfShells = m_BValueMap.size()-1;

  // create empty nxm SignalMatrix containing n->signals/directions (in case of interpolation ~ sizeAllDirections otherwise the size of any shell) for m->shells
  vnl_matrix<double> SignalMatrix(m_NumberTargetDirections, numberOfShells);
  // create nx1 targetSignalVector

  vnl_vector<double> SignalVector(m_NumberTargetDirections);
  OutputPixelType out;
  InputPixelType b;
  BValueMap::const_iterator shellIterator;
  vnl_matrix<double> NewSignalMatrix (m_NumberTargetDirections, 2);
  vnl_vector<double> InterpVector;
  unsigned int shellIndex = 0;
  // ** walking over each Voxel
  while(!iit.IsAtEnd())
  {
    b = iit.Get();
    AverageS0=0.0;
    for(unsigned int i = 0 ; i < IndicesS0.size(); i++){
      AverageS0 += b[IndicesS0[i]];
    }
    AverageS0 /= (double)IndicesS0.size();
    out = oit.Get();
    out.SetElement(0,AverageS0);

    shellIterator = m_BValueMap.begin();
    shellIterator++; //skip bZeroImages

    shellIndex = 0;
    // for each shell
    while(shellIterator != m_BValueMap.end())
    {
      const IndicesVector & currentShell = shellIterator->second;
      InterpVector.set_size(currentShell.size());

      // get the raw signal for currente shell (signal for eacht direction)
      for(unsigned int i = 0 ; i < currentShell.size(); i++)
        InterpVector.put(i,b[currentShell[i]]);

      // interpolate the signal using corresponding interpolationSHBasis
      SignalVector = m_ShellInterpolationMatrixVector.at(shellIndex) * InterpVector;

      // save interpolated signal column
      SignalMatrix.set_column(shellIndex, SignalVector);

      shellIterator++;
      shellIndex++;
    }

    // apply voxel wise signal manipulation functor
    (*m_Functor)(NewSignalMatrix, /*const &*/SignalMatrix,/*const &*/ AverageS0);
    SignalVector = NewSignalMatrix.get_column(0);

    for(unsigned int i = 1 ; i < out.Size(); i ++)
      out.SetElement(i,SignalVector.get(i-1));

    eit.Set(NewSignalMatrix.get_column(1).mean());

    oit.Set(out);
    ++eit;
    ++oit;
    ++iit;
  }
}

} // end of namespace
