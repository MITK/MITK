/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDiffusionMultiShellQballReconstructionImageFilter_cpp
#define __itkDiffusionMultiShellQballReconstructionImageFilter_cpp

#include <itkDiffusionMultiShellQballReconstructionImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkArray.h>
#include <vnl/vnl_vector.h>

#include <boost/version.hpp>
#include <stdio.h>
#include <locale>

#define _USE_MATH_DEFINES
#include <math.h>

#if BOOST_VERSION / 100000 > 0
#if BOOST_VERSION / 100 % 1000 > 34
#include <boost/math/special_functions/legendre.hpp>
#endif
#endif

#include "mitkSphericalHarmonicsFunctions.h"
#include "itkPointShell.h"

namespace itk {

#define QBALL_ANAL_RECON_PI       M_PI

template< class T, class TG, class TO, int L, int NODF>
DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::DiffusionMultiShellQballReconstructionImageFilter() :
  m_GradientDirectionContainer(NULL),
  m_NumberOfGradientDirections(0),
  m_NumberOfBaselineImages(1),
  m_Threshold(NumericTraits< ReferencePixelType >::NonpositiveMin()),
  m_BValue(1.0),
  m_Lambda(0.0),
  m_DirectionsDuplicated(false)
{
  // At least 1 inputs is necessary for a vector image.
  // For images added one at a time we need at least six
  this->SetNumberOfRequiredInputs( 1 );
}


template< class TReferenceImagePixelType, class TGradientImagePixelType, class TOdfPixelType, int NOrderL, int NrOdfDirections>
typename itk::DiffusionMultiShellQballReconstructionImageFilter< TReferenceImagePixelType,TGradientImagePixelType,TOdfPixelType, NOrderL,NrOdfDirections>::OdfPixelType itk::DiffusionMultiShellQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, NOrderL, NrOdfDirections>
::Normalize( OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{

}


template<class TReferenceImagePixelType, class TGradientImagePixelType, class TOdfPixelType, int NOrderL, int NrOdfDirections>
vnl_vector<TOdfPixelType>itk::DiffusionMultiShellQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType,NOrderL, NrOdfDirections>
::PreNormalize( vnl_vector<TOdfPixelType> vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
  // Threshold
  double sigma = 0.001;

  int n = vec.size();
  double b0f = (double)b0;
  for(int i=0; i<n; i++)
  {
    if(vec[i] < 0)
    {
      vec[i] = sigma / 2 ;
    }
    if(0 <= vec[i] && vec[i] < sigma )
    {
      vec[i] = sigma / 2 + (vec[i] * vec[i]) / 2 * sigma;
    }
    if( 1 - sigma <= vec[i] && vec[i] < 1 )
    {
      vec[i] = 1-(sigma/2) - (( 1 - vec[i] * vec[i] ) / 2 * sigma );
    }
    if( 1 <= vec[i] )
    {
      vec[i] = 1 - (sigma / 2);
    }

    if (b0f==0)
      b0f = 0.01;
    if(vec[i] >= b0f)
      vec[i] = b0f - 0.001;

    vec[i] = log(-log(vec[i]/b0f));
  }
  return vec;
}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::SetGradientImage( GradientDirectionContainerType *gradientDirection, const GradientImagesType *gradientImage )
{
  this->m_GradientDirectionContainer = gradientDirection;
  this->m_NumberOfBaselineImages = 0;

  for(GradientDirectionContainerType::Iterator it = gradientDirection->Begin(); it != gradientDirection->End(); it++)
  {
    if( it.Value().one_norm() <= 0.0 )
      this->m_NumberOfBaselineImages ++;
    else
      it.Value() = it.Value() / it.Value().two_norm();
  }

  this->m_NumberOfGradientDirections = gradientDirection->Size() - this->m_NumberOfBaselineImages;

  // ensure that the gradient image we received has as many components as
  // the number of gradient directions
  if( gradientImage->GetVectorLength() != this->m_NumberOfBaselineImages + m_NumberOfGradientDirections )
  {
    itkExceptionMacro( << m_NumberOfGradientDirections << " gradients + " << this->m_NumberOfBaselineImages
                       << "baselines = " << m_NumberOfGradientDirections + this->m_NumberOfBaselineImages
                       << " directions specified but image has " << gradientImage->GetVectorLength()
                       << " components.");
  }

  this->ProcessObject::SetNthInput( 0, const_cast< GradientImagesType* >(gradientImage) );
}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::BeforeThreadedGenerateData()
{
  if( m_NumberOfGradientDirections < (((L+1)*(L+2))/2) /* && m_NumberOfGradientDirections < 6 */ )
  {
    itkExceptionMacro( << "At least (L+1)(L+2)/2 gradient directions are required" );
  }
  // Input must be an itk::VectorImage.
  std::string gradientImageClassName(this->ProcessObject::GetInput(0)->GetNameOfClass());

  if ( strcmp(gradientImageClassName.c_str(),"VectorImage") != 0 )
    itkExceptionMacro( << "There is only one Gradient image. I expect that to be a VectorImage. But its of type: " << gradientImageClassName );

  this->ComputeReconstructionMatrix();

  m_BZeroImage = BZeroImageType::New();
  typename GradientImagesType::Pointer img = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );
  m_BZeroImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
  m_BZeroImage->SetOrigin( img->GetOrigin() );     // Set the image origin
  m_BZeroImage->SetDirection( img->GetDirection() );  // Set the image direction
  m_BZeroImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
  m_BZeroImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
  m_BZeroImage->Allocate();

  m_ODFSumImage = BZeroImageType::New();
  m_ODFSumImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
  m_ODFSumImage->SetOrigin( img->GetOrigin() );     // Set the image origin
  m_ODFSumImage->SetDirection( img->GetDirection() );  // Set the image direction
  m_ODFSumImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
  m_ODFSumImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
  m_ODFSumImage->Allocate();

}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int NumberOfThreads)
{
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
  oit2.GoToBegin();

  ImageRegionIterator< BlaImage > oit3(m_ODFSumImage, outputRegionForThread);
  oit3.GoToBegin();

  typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
  typedef typename GradientImagesType::PixelType         GradientVectorType;
  typename GradientImagesType::Pointer gradientImagePointer = NULL;

  gradientImagePointer = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );

  GradientIteratorType git(gradientImagePointer, outputRegionForThread );
  git.GoToBegin();

  std::vector<unsigned int> baselineIndicies;
  std::vector<unsigned int> gradientIndicies;

  GradientDirectionContainerType::ConstIterator gdcit;
  for( gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
  {
    if(gdcit.Value().one_norm() <= 0.0)
    {
      baselineIndicies.push_back(gdcit.Index());
    }else{
      gradientIndicies.push_back(gdcit.Index());
    }
  }

  if(m_DirectionsDuplicated){
    int NumbersOfGradientIndicies = gradientIndicies.size();
    for (int i = 0 ; i < NumbersOfGradientIndicies; i++)
      gradientIndicies.push_back(gradientIndicies[i]);
  }

  while( ! git.IsAtEnd() )
  {
    GradientVectorType b = git.Get();
    typename NumericTraits<ReferencePixelType>::AccumulateType b0 = NumericTraits<ReferencePixelType>::Zero;

    for(unsigned int i = 0; i < baselineIndicies.size(); ++i)
    {
      b0 += b[baselineIndicies[i]];
    }
    b0 /= this->m_NumberOfBaselineImages;

    OdfPixelType odf(0.0);
    vnl_vector<TO> B(m_NumberOfGradientDirections);

    if( (b0 != 0) && (b0 >= m_Threshold) )
    {
      for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
      {
        B[i] = static_cast<TO>(b[gradientIndicies[i]]);
      }

      B = PreNormalize(B, b0);
      vnl_vector<TO> coeffs(m_NumberCoefficients);
      coeffs = ( (*m_CoeffReconstructionMatrix) * B );
      coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));
      odf = ( (*m_SphericalHarmonicBasisMatrix) * coeffs ).data_block();

    }
    float sum = 0.0;
    oit.Set( odf );
    ++oit;
    oit2.Set( b0 );
    for (int k=0; k<odf.Size(); k++)
      sum += (float) odf[k];
    oit3.Set( sum-1 );
    ++oit3;
    ++oit2;
    ++git;
  }


}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T, TG, TO, L, NODF>::
ComputeSphericalHarmonicsBasis(vnl_matrix<double> * QBallReference, vnl_matrix<double> *SHBasisOutput, vnl_matrix<double>* LaplaciaBaltramiOutput, vnl_vector<int>* SHOrderAssociation )
{
  for(unsigned int i=0; i<m_NumberOfGradientDirections; i++)
  {
    for(int k = 0; k <= L; k += 2)
    {
      for(int m =- k; m <= k; m++)
      {
        int j = ( k * k + k + 2 ) / 2 + m - 1;

        // Compute SHBasisFunctions
        double phi = (*QBallReference)(0,i);
        double th = (*QBallReference)(1,i);
        (*SHBasisOutput)(i,j) = mitk::ShericalHarmonicsFunctions::Yj(m,k,th,phi);

        // Laplacian Baltrami Order Association
        (*LaplaciaBaltramiOutput)(j,j) = k*k*(k + 1)*(k+1);

        // Order Association
        (*SHOrderAssociation)[j] = k;

      }
    }
  }
}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ComputeFunkRadonTransformationMatrix(vnl_vector<int>* SHOrderAssociationReference, vnl_matrix<double>* FRTMatrixOutput )
{
  for(int i=0; i<m_NumberCoefficients; i++)
  {
    (*FRTMatrixOutput)(i,i) = 2.0 * QBALL_ANAL_RECON_PI * mitk::ShericalHarmonicsFunctions::legendre0((*SHOrderAssociationReference)[i]);
  }
}


template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ComputeReconstructionMatrix()
{
  if( m_NumberOfGradientDirections < (((L+1)*(L+2))/2) /* && m_NumberOfGradientDirections < 6 */ )
  {
    itkExceptionMacro( << "At least (L+1)(L+2)/2 gradient directions are required" );
  }

  CheckDuplicateDiffusionGradients();

  // check hemisphere or sphere
  {
    // handle acquisition schemes where only half of the spherical
    // shell is sampled by the gradient directions. In this case,
    // each gradient direction is duplicated in negative direction.
    vnl_vector<double> centerMass(3);
    centerMass.fill(0.0);
    int count = 0;
    GradientDirectionContainerType::ConstIterator gdcit1;
    for( gdcit1 = this->m_GradientDirectionContainer->Begin(); gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
    {
      if(gdcit1.Value().one_norm() > 0.0)
      {
        centerMass += gdcit1.Value();
        count ++;
      }
    }
    centerMass /= count;
    if(centerMass.two_norm() > 0.1)
    {
      m_DirectionsDuplicated = true;
      m_NumberOfGradientDirections *= 2;
    }
  }

  vnl_matrix<double> *Q = new vnl_matrix<double>(3, m_NumberOfGradientDirections);

  // Cartesian to spherical coordinates
  {
    int i = 0;
    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
    {
      if(gdcit.Value().one_norm() > 0.0)
      {
        double x = gdcit.Value().get(0);
        double y = gdcit.Value().get(1);
        double z = gdcit.Value().get(2);
        double cart[3];
        mitk::ShericalHarmonicsFunctions::Cart2Sph(x,y,z,cart);
        (*Q)(0,i) = cart[0];
        (*Q)(1,i) = cart[1];
        (*Q)(2,i++) = cart[2];
      }
    }
    if(m_DirectionsDuplicated)
    {
      for( gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
      {
        if(gdcit.Value().one_norm() > 0.0)
        {
          double x = gdcit.Value().get(0);
          double y = gdcit.Value().get(1);
          double z = gdcit.Value().get(2);
          double cart[3];
          mitk::ShericalHarmonicsFunctions::Cart2Sph(x,y,z,cart);
          (*Q)(0,i) = cart[0];
          (*Q)(1,i) = cart[1];
          (*Q)(2,i++) = cart[2];
        }
      }
    }
  }

  int LOrder = L;
  m_NumberCoefficients = (int)(LOrder*LOrder + LOrder + 2.0)/2.0 + LOrder;

  vnl_matrix<double>* SHBasis = new vnl_matrix<double>(m_NumberOfGradientDirections,m_NumberCoefficients);
  vnl_vector<int>* SHOrderAssociation = new vnl_vector<int>(m_NumberCoefficients);
  vnl_matrix<double>* LaplacianBaltrami = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
  vnl_matrix<double>* FRTMatrix = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);

  LaplacianBaltrami->fill(0.0);
  FRTMatrix->fill(0.0);

  // SHBasis-Matrix + LaplacianBaltrami-Matrix + SHOrderAssociationVector
  ComputeSphericalHarmonicsBasis(Q ,SHBasis, LaplacianBaltrami, SHOrderAssociation);

  // Compute FunkRadon Transformation Matrix Associated to SHBasis Order lj
  ComputeFunkRadonTransformationMatrix(SHOrderAssociation ,FRTMatrix);



 // vnl_matrix<double> * temp = new vnl_matrix<double>(((SHBasis->transpose() * (*SHBasis)) + (m_Lamda * LaplacianBaltrami) );
 // vnl_matrix_inverse<double> *inv =

  /// OLD SHIT
  vnl_matrix<double> * SHBasisTranspose = new vnl_matrix<double>(SHBasis->transpose());
  vnl_matrix<double> SHBasisTransposeMultipliedWithSHBasis = (*SHBasisTranspose) * (*SHBasis);
  vnl_matrix<double>* SquaredLaplacianBaltrami = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
  SquaredLaplacianBaltrami->fill(0.0);

  SquaredLaplacianBaltrami->update(*LaplacianBaltrami);
  *SquaredLaplacianBaltrami *= *LaplacianBaltrami;

  vnl_matrix<double> lambdaLMultipliedWithSquaredLaplacianBaltrami(m_NumberCoefficients,m_NumberCoefficients);
  lambdaLMultipliedWithSquaredLaplacianBaltrami.update((*SquaredLaplacianBaltrami));
  lambdaLMultipliedWithSquaredLaplacianBaltrami *= m_Lambda;

  vnl_matrix<double> tmp( SHBasisTransposeMultipliedWithSHBasis + lambdaLMultipliedWithSquaredLaplacianBaltrami );
  vnl_matrix_inverse<double> *pseudoInverse = new vnl_matrix_inverse<double>( tmp );
  vnl_matrix<double>* Inv = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
  Inv->fill(0.0);

  (*Inv) = pseudoInverse->pinverse();

  vnl_matrix<double> temp( (*Inv) *  (*SHBasisTranspose));

  double fac1 = (1.0/(16.0*QBALL_ANAL_RECON_PI*QBALL_ANAL_RECON_PI));

  temp = fac1 * (*FRTMatrix) * (*LaplacianBaltrami) * temp;

  m_CoeffReconstructionMatrix = new vnl_matrix<TO>(m_NumberCoefficients,m_NumberOfGradientDirections);

  MITK_INFO << m_NumberCoefficients <<  " " << m_NumberOfGradientDirections;
  for(int i=0; i<m_NumberCoefficients; i++)
  {
    for(unsigned int j=0; j<m_NumberOfGradientDirections; j++)
    {
      (*m_CoeffReconstructionMatrix)(i,j) = (float) temp(i,j);
    }
  }

  // this code goes to the image adapter coeffs->odfs later

  int NOdfDirections = NODF;
  vnl_matrix_fixed<double, 3, NODF>* U = itk::PointShell<NODF, vnl_matrix_fixed<double, 3, NODF> >::DistributePointShell();

  m_SphericalHarmonicBasisMatrix  = new vnl_matrix<TO>(NOdfDirections,m_NumberCoefficients);
  vnl_matrix<double>* sphericalHarmonicBasisMatrix2 = new vnl_matrix<double>(NOdfDirections,m_NumberCoefficients);

  for(int i=0; i<NOdfDirections; i++)
  {
    double x = (*U)(0,i);
    double y = (*U)(1,i);
    double z = (*U)(2,i);
    double cart[3];
    mitk::ShericalHarmonicsFunctions::Cart2Sph(x,y,z,cart);
    (*U)(0,i) = cart[0];
    (*U)(1,i) = cart[1];
    (*U)(2,i) = cart[2];
  }

  for(int i=0; i<NOdfDirections; i++)
  {
    for(int k=0; k<=LOrder; k+=2)
    {
      for(int m=-k; m<=k; m++)
      {
        int j = (k*k + k + 2)/2 + m - 1;
        double phi = (*U)(0,i);
        double th = (*U)(1,i);
        (*m_SphericalHarmonicBasisMatrix)(i,j) = mitk::ShericalHarmonicsFunctions::Yj(m,k,th,phi);
        (*sphericalHarmonicBasisMatrix2)(i,j) = (*m_SphericalHarmonicBasisMatrix)(i,j);
      }
    }
  }

  m_ReconstructionMatrix = new vnl_matrix<TO>(NOdfDirections,m_NumberOfGradientDirections);
  *m_ReconstructionMatrix = (*m_SphericalHarmonicBasisMatrix) * (*m_CoeffReconstructionMatrix);

}

template< class T, class TG, class TO, int L, int NODF>
bool DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::CheckDuplicateDiffusionGradients()
{

  GradientDirectionContainerType::ConstIterator gdcit1;
  GradientDirectionContainerType::ConstIterator gdcit2;

  for(gdcit1 = this->m_GradientDirectionContainer->Begin(); gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
  {
    for(gdcit2 = this->m_GradientDirectionContainer->Begin(); gdcit2 != this->m_GradientDirectionContainer->End(); ++gdcit2)
    {
      if(gdcit1.Value() == gdcit2.Value() && gdcit1.Index() != gdcit2.Index())
      {
        itkWarningMacro( << "Some of the Diffusion Gradients equal each other. Corresponding image data should be averaged before calling this filter." );
        return true;
      }
    }
  }
  return false;
}


template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::PrintSelf(std::ostream& os, Indent indent) const
{
  std::locale C("C");
  std::locale originalLocale = os.getloc();
  os.imbue(C);

  Superclass::PrintSelf(os,indent);

  os << indent << "OdfReconstructionMatrix: " << m_ReconstructionMatrix << std::endl;
  if ( m_GradientDirectionContainer )
  {
    os << indent << "GradientDirectionContainer: "
       << m_GradientDirectionContainer << std::endl;
  }
  else
  {
    os << indent <<
          "GradientDirectionContainer: (Gradient directions not set)" << std::endl;
  }
  os << indent << "NumberOfGradientDirections: " <<
        m_NumberOfGradientDirections << std::endl;
  os << indent << "NumberOfBaselineImages: " <<
        m_NumberOfBaselineImages << std::endl;
  os << indent << "Threshold for reference B0 image: " << m_Threshold << std::endl;
  os << indent << "BValue: " << m_BValue << std::endl;

  os.imbue( originalLocale );
}




}

#endif // __itkDiffusionMultiShellQballReconstructionImageFilter_cpp
