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
#include <strstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include "mitkSphericalHarmonicsFunctions.h"
#include "itkPointShell.h"
#include <memory>

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
  m_IsHemisphericalArrangementOfGradientDirections(false)
{
  // At least 1 inputs is necessary for a vector image.
  // For images added one at a time we need at least six
  this->SetNumberOfRequiredInputs( 1 );
}


template< class TReferenceImagePixelType, class TGradientImagePixelType, class TOdfPixelType, int NOrderL, int NrOdfDirections>
typename itk::DiffusionMultiShellQballReconstructionImageFilter< TReferenceImagePixelType,TGradientImagePixelType,TOdfPixelType, NOrderL,NrOdfDirections>::OdfPixelType itk::DiffusionMultiShellQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, NOrderL, NrOdfDirections>
::Normalize( OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
  for(int i=0; i<NrOdfDirections; i++)
  {
    odf[i] = odf[i] < 0 ? 0 : odf[i];
    odf[i] *= QBALL_ANAL_RECON_PI*4/NrOdfDirections;
  }
  TOdfPixelType sum = 0;
  for(int i=0; i<NrOdfDirections; i++)
  {
    sum += odf[i];
  }
  if(sum>0)
    odf /= sum;

  return odf;
}


template<class TReferenceImagePixelType, class TGradientImagePixelType, class TOdfPixelType, int NOrderL, int NrOdfDirections>
vnl_vector<TOdfPixelType>itk::DiffusionMultiShellQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType,NOrderL, NrOdfDirections>
::PreNormalize( vnl_vector<TOdfPixelType> vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
  // Threshold
  double sigma = 0.001;
  double temp;
  int n = vec.size();
  double b0f = (double)b0;
  for(int i=0; i<n; i++)
  {

    if (b0f==0){
      MITK_INFO << b0f;
      b0f = 0.001;
    }

    vec[i] /= b0f;
    //temp = vec[i];

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
    // MITK_INFO <<  temp  << "  <->  " << vec[i];

    vec[i] = log(-log(vec[i]));
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
    itkExceptionMacro( << "At least " << ((L+1)*(L+2))/2 << " gradient directions are required" );
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

}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int NumberOfThreads)
{
  // Get output image pointer
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  // ImageRegionIterator for the output image
  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  // ImageRegionIterator for the BZero (output) image
  ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
  oit2.GoToBegin();



  // define basline-signal indicies vector
  std::vector<unsigned int> baselineIndicies;
  // define gradient-signal indicies vector
  std::vector<unsigned int> gradientIndicies;

  // split for all gradient directions the image in baseline-indicies and gradient-indicies for a fast access
  GradientDirectionContainerType::ConstIterator gdcit;
  for( gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
  {

    if(gdcit.Value().one_norm() <= 0.0) // if vector length at current position is 0, it is a basline signal
    {
      baselineIndicies.push_back(gdcit.Index());
    }else{ // it is a gradient signal of length != 0
      gradientIndicies.push_back(gdcit.Index());
    }
  }

  // if the gradient directiosn aragement is hemispherical, duplicate all gradient directions
  // alone, interested in the value, the direction can be neglected
  if(m_IsHemisphericalArrangementOfGradientDirections){
    int NumbersOfGradientIndicies = gradientIndicies.size();
    for (int i = 0 ; i < NumbersOfGradientIndicies; i++)
      gradientIndicies.push_back(gradientIndicies[i]);
  }


  // Get input gradient image pointer
  typename GradientImagesType::Pointer gradientImagePointer = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );

  //  Const ImageRegionIterator for input gradient image
  typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
  GradientIteratorType git(gradientImagePointer, outputRegionForThread );
  git.GoToBegin();

  typedef typename GradientImagesType::PixelType         GradientVectorType;

  // iterate overall voxels of the gradient image region
  while( ! git.IsAtEnd() )
  {
    GradientVectorType b = git.Get();

    // compute the average bzero signal
    typename NumericTraits<ReferencePixelType>::AccumulateType b0 = NumericTraits<ReferencePixelType>::Zero;
    for(unsigned int i = 0; i < baselineIndicies.size(); ++i)
    {
      b0 += b[baselineIndicies[i]];
    }
    b0 /= this->m_NumberOfBaselineImages;

    // ODF Vector
    OdfPixelType odf(0.0);

    // Create the Signal Vector
    vnl_vector<TO> SignalVector(m_NumberOfGradientDirections);
    if( (b0 != 0) && (b0 >= m_Threshold) )
    {
      for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
      {
        SignalVector[i] = static_cast<TO>(b[gradientIndicies[i]]);
      }

      // apply threashold an generate ln(-ln(E)) signal
      // Replace SignalVector with PreNormalized SignalVector
      SignalVector = PreNormalize(SignalVector, b0);

      // ODF coeffs-vector
      vnl_vector<TO> coeffs(m_NumberCoefficients);

      // approximate ODF coeffs
      coeffs = ( (*m_CoeffReconstructionMatrix) * SignalVector );
      coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));

      odf = ( (*m_SphericalHarmonicBasisMatrix) * coeffs ).data_block();
    }
    // set ODF to ODF-Image
    oit.Set( odf );
    ++oit;

    // set b0(average) to b0-Image
    oit2.Set( b0 );
    ++oit2;

    ++git;

  }

  MITK_INFO << "One Thread finished reconstruction";

}

  template< class T, class TG, class TO, int L, int NODF>
  void DiffusionMultiShellQballReconstructionImageFilter<T, TG, TO, L, NODF>::
      ComputeSphericalHarmonicsBasis(vnl_matrix<double> * QBallReference, vnl_matrix<double> *SHBasisOutput, vnl_matrix<double>* LaplaciaBaltramiOutput, vnl_vector<int>* SHOrderAssociation, vnl_matrix<double>* SHEigenvalues )
  {
    for(unsigned int i=0; i< (*SHBasisOutput).rows(); i++)
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
          if(LaplaciaBaltramiOutput)
            (*LaplaciaBaltramiOutput)(j,j) = k*k*(k + 1)*(k+1);

          // SHEigenvalues with order Accosiation kj
          if(SHEigenvalues)
            (*SHEigenvalues)(j,j) = -k* (k+1);

          // Order Association
          if(SHOrderAssociation)
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

  template< class T, class TG, class TO, int L, int NOdfDirections>
  bool DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NOdfDirections>
      ::CheckHemisphericalArrangementOfGradientDirections()
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
      return false;
    }
    return true;
  }

  template< class T, class TG, class TO, int L, int NOdfDirections>
  void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NOdfDirections>
      ::ComputeReconstructionMatrix()
  {

    typedef std::auto_ptr< vnl_matrix< double> >  MatrixDoublePtr;
    typedef std::auto_ptr< vnl_vector< int > >    VectorIntPtr;
    typedef std::auto_ptr< vnl_matrix_inverse< double > >  InverseMatrixDoublePtr;

    if( m_NumberOfGradientDirections < (((L+1)*(L+2))/2) || m_NumberOfGradientDirections < 6  )
    {
      itkExceptionMacro( << "At least (L+1)(L+2)/2 gradient directions are required" );
    }

    CheckDuplicateDiffusionGradients();


    // check if gradient directions are arrangement as a hemisphere(true) or sphere(false)
    m_IsHemisphericalArrangementOfGradientDirections = CheckHemisphericalArrangementOfGradientDirections();
    if(m_IsHemisphericalArrangementOfGradientDirections) m_NumberOfGradientDirections *= 2;


    MatrixDoublePtr Q(new vnl_matrix<double>(3, m_NumberOfGradientDirections));
    Q->fill(0.0);
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
      if(m_IsHemisphericalArrangementOfGradientDirections)
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

    MatrixDoublePtr SHBasis(new vnl_matrix<double>(m_NumberOfGradientDirections,m_NumberCoefficients));
    SHBasis->fill(0.0);
    VectorIntPtr SHOrderAssociation(new vnl_vector<int>(m_NumberCoefficients));
    SHOrderAssociation->fill(0.0);
    MatrixDoublePtr LaplacianBaltrami(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    LaplacianBaltrami->fill(0.0);
    MatrixDoublePtr FRTMatrix(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    FRTMatrix->fill(0.0);
    MatrixDoublePtr SHEigenvalues(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    SHEigenvalues->fill(0.0);



    // SHBasis-Matrix + LaplacianBaltrami-Matrix + SHOrderAssociationVector
    ComputeSphericalHarmonicsBasis(Q.get() ,SHBasis.get(), LaplacianBaltrami.get(), SHOrderAssociation.get(), SHEigenvalues.get());

    // Compute FunkRadon Transformation Matrix Associated to SHBasis Order lj
    ComputeFunkRadonTransformationMatrix(SHOrderAssociation.get() ,FRTMatrix.get());

    MatrixDoublePtr temp(new vnl_matrix<double>(((SHBasis->transpose()) * (*SHBasis)) + (m_Lambda  * (*LaplacianBaltrami))));

    InverseMatrixDoublePtr pseudo_inv(new vnl_matrix_inverse<double>((*temp)));
    MatrixDoublePtr inverse(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    inverse->fill(0.0);
    (*inverse) = pseudo_inv->inverse();

    // ODF Factor ( missing 1/4PI ?? )
    double factor = (1.0/(16.0*QBALL_ANAL_RECON_PI*QBALL_ANAL_RECON_PI));

    MatrixDoublePtr TransformationMatrix(new vnl_matrix<double>( factor * ((*FRTMatrix) * ((*SHEigenvalues) * ((*inverse) * (SHBasis->transpose()) ) ) )));

    m_CoeffReconstructionMatrix = new vnl_matrix<TO>(m_NumberCoefficients,m_NumberOfGradientDirections);

    // Cast double to float
    for(int i=0; i<m_NumberCoefficients; i++)
      for(unsigned int j=0; j<m_NumberOfGradientDirections; j++)
        (*m_CoeffReconstructionMatrix)(i,j) = (float)(*TransformationMatrix)(i,j);



    // this code goes to the image adapter coeffs->odfs later

    vnl_matrix_fixed<double, 3, NOdfDirections>* U = itk::PointShell<NOdfDirections, vnl_matrix_fixed<double, 3, NOdfDirections> >::DistributePointShell();

    m_SphericalHarmonicBasisMatrix  = new vnl_matrix<TO>(NOdfDirections,m_NumberCoefficients);


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
        }
      }
    }

    m_ReconstructionMatrix = new vnl_matrix<TO>(NOdfDirections,m_NumberOfGradientDirections);
    *m_ReconstructionMatrix = (*m_SphericalHarmonicBasisMatrix) * (*m_CoeffReconstructionMatrix);

  }

  template< class T, class TG, class TO, int L, int NODF>
  template< class VNLType >
  void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
      ::printMatrix( VNLType * mat  )
  {

    std::stringstream stream;

    for(int i = 0 ; i < mat->rows(); i++)
    {
      stream.str("");
      for(int j = 0; j < mat->cols(); j++)
      {
        stream << (*mat)(i,j) << "  ";
      }
      MITK_INFO << stream.str();
    }
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
