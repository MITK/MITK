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
#ifndef __itkAnalyticalDiffusionQballReconstructionImageFilter_cpp
#define __itkAnalyticalDiffusionQballReconstructionImageFilter_cpp

#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkArray.h>
#include <vnl/vnl_vector.h>
#include <mitkDiffusionFunctionCollection.h>

#include <cstdio>
#include <locale>
#include <fstream>
#include "itkPointShell.h"

namespace itk {

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>::AnalyticalDiffusionQballReconstructionImageFilter() :
  m_GradientDirectionContainer(nullptr),
  m_NumberOfGradientDirections(0),
  m_NumberOfBaselineImages(1),
  m_Threshold(NumericTraits< ReferencePixelType >::NonpositiveMin()),
  m_BValue(-1),
  m_Lambda(0.0),
  m_DirectionsDuplicated(false),
  m_Delta1(0.001),
  m_Delta2(0.001),
  m_UseMrtrixBasis(false)
{
  // At least 1 inputs is necessary for a vector image.
  // For images added one at a time we need at least six
  this->SetNumberOfRequiredInputs( 1 );
}


template<
    class TReferenceImagePixelType,
    class TGradientImagePixelType,
    class TOdfPixelType,
    int ShOrder,
    int NrOdfDirections>
typename itk::AnalyticalDiffusionQballReconstructionImageFilter< TReferenceImagePixelType,TGradientImagePixelType,TOdfPixelType, ShOrder,NrOdfDirections>::OdfPixelType
itk::AnalyticalDiffusionQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, ShOrder, NrOdfDirections>::Normalize( OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
  switch( m_NormalizationMethod )
  {
  case QBAR_STANDARD:
  {
    TOdfPixelType sum = 0;

    for(int i=0; i<NrOdfDirections; i++)
    {
      sum += odf[i];
    }
    if(sum>0)
      odf /= sum;

    return odf;
    break;
  }
  case QBAR_B_ZERO_B_VALUE:
  {
    for(int i=0; i<NrOdfDirections; i++)
    {
      odf[i] = ((TOdfPixelType)log((TOdfPixelType)b0)-odf[i])/m_BValue;
    }
    return odf;
    break;
  }
  case QBAR_B_ZERO:
  {
    odf *= 1.0/b0;
    return odf;
    break;
  }
  case QBAR_NONE:
  {
    return odf;
    break;
  }
  case QBAR_ADC_ONLY:
  {
    for(int i=0; i<NrOdfDirections; i++)
    {
      odf[i] = ((TOdfPixelType)log((TOdfPixelType)b0)-odf[i])/m_BValue;
    }
    return odf;
    break;
  }
  case QBAR_RAW_SIGNAL:
  {
    return odf;
    break;
  }
  case QBAR_SOLID_ANGLE:
  {
    for(int i=0; i<NrOdfDirections; i++)
      odf[i] *= itk::Math::pi*4/NrOdfDirections;

    break;
  }
  case QBAR_NONNEG_SOLID_ANGLE:
  {
    break;
  }
  }

  return odf;
}


template<
    class TReferenceImagePixelType,
    class TGradientImagePixelType,
    class TOdfPixelType,
    int ShOrder,
    int NrOdfDirections>
vnl_vector<TOdfPixelType> itk::AnalyticalDiffusionQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, ShOrder, NrOdfDirections>::PreNormalize( vnl_vector<TOdfPixelType> vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
  switch( m_NormalizationMethod )
  {
  case QBAR_STANDARD:
  {
    int n = vec.size();
    double b0f = (double)b0;
    for(int i=0; i<n; i++)
    {
      vec[i] = vec[i]/b0f;
    }
    return vec;
    break;
  }
  case QBAR_B_ZERO_B_VALUE:
  {
    int n = vec.size();
    for(int i=0; i<n; i++)
    {
      if (vec[i]<=0)
        vec[i] = 0.001;

      vec[i] = log(vec[i]);
    }
    return vec;
    break;
  }
  case QBAR_B_ZERO:
  {
    return vec;
    break;
  }
  case QBAR_NONE:
  {
    return vec;
    break;
  }
  case QBAR_ADC_ONLY:
  {
    int n = vec.size();
    for(int i=0; i<n; i++)
    {
      if (vec[i]<=0)
        vec[i] = 0.001;

      vec[i] = log(vec[i]);
    }
    return vec;
    break;
  }
  case QBAR_RAW_SIGNAL:
  {
    return vec;
    break;
  }
  case QBAR_SOLID_ANGLE:
  case QBAR_NONNEG_SOLID_ANGLE:
  {
    int n = vec.size();
    double b0f = (double)b0;
    for(int i=0; i<n; i++)
    {
      vec[i] = vec[i]/b0f;

      if (vec[i]<0)
        vec[i] = m_Delta1;
      else if (vec[i]<m_Delta1)
        vec[i] = m_Delta1/2 + vec[i]*vec[i]/(2*m_Delta1);
      else if (vec[i]>=1)
        vec[i] = 1-m_Delta2/2;
      else if (vec[i]>=1-m_Delta2)
        vec[i] = 1-m_Delta2/2-(1-vec[i])*(1-vec[i])/(2*m_Delta2);

      vec[i] = log(-log(vec[i]));
    }
    return vec;
    break;
  }
  }

  return vec;
}

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>::BeforeThreadedGenerateData()
{
  // If we have more than 2 inputs, then each input, except the first is a
  // gradient image. The number of gradient images must match the number of
  // gradient directions.
  //const unsigned int numberOfInputs = this->GetNumberOfInputs();

  // There need to be at least 6 gradient directions to be able to compute the
  // tensor basis
  if( m_NumberOfGradientDirections < (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder )
  {
    itkExceptionMacro( << "Not enough gradient directions supplied (" << m_NumberOfGradientDirections << "). At least " << (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder << " needed for SH-order " << ShOrder);
  }

  // Input must be an itk::VectorImage.
  std::string gradientImageClassName(
        this->ProcessObject::GetInput(0)->GetNameOfClass());
  if ( strcmp(gradientImageClassName.c_str(),"VectorImage") != 0 )
  {
    itkExceptionMacro( << "There is only one Gradient image. I expect that to be a VectorImage. " << "But its of type: " << gradientImageClassName );
  }

  this->ComputeReconstructionMatrix();

  typename GradientImagesType::Pointer img = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );

  m_BZeroImage = BZeroImageType::New();
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

  m_CoefficientImage = CoefficientImageType::New();
  m_CoefficientImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
  m_CoefficientImage->SetOrigin( img->GetOrigin() );     // Set the image origin
  m_CoefficientImage->SetDirection( img->GetDirection() );  // Set the image direction
  m_CoefficientImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
  m_CoefficientImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
  m_CoefficientImage->Allocate();

  if(m_NormalizationMethod == QBAR_SOLID_ANGLE || m_NormalizationMethod == QBAR_NONNEG_SOLID_ANGLE)
    m_Lambda = 0.0;
}

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
  oit2.GoToBegin();

  ImageRegionIterator< FloatImageType > oit3(m_ODFSumImage, outputRegionForThread);
  oit3.GoToBegin();

  ImageRegionIterator< CoefficientImageType > oit4(m_CoefficientImage, outputRegionForThread);
  oit4.GoToBegin();

  typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
  typedef typename GradientImagesType::PixelType         GradientVectorType;
  typename GradientImagesType::Pointer gradientImagePointer = nullptr;

  // Would have liked a dynamic_cast here, but seems SGI doesn't like it
  // The enum will ensure that an inappropriate cast is not done
  gradientImagePointer = static_cast< GradientImagesType * >(
                           this->ProcessObject::GetInput(0) );

  GradientIteratorType git(gradientImagePointer, outputRegionForThread );
  git.GoToBegin();

  // Compute the indicies of the baseline images and gradient images
  std::vector<unsigned int> baselineind; // contains the indicies of
  // the baseline images
  std::vector<unsigned int> gradientind; // contains the indicies of
  // the gradient images

  for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
      gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
  {
    float bval = gdcit.Value().two_norm();
    bval = bval*bval*m_BValue;
    if(bval < 100)
      baselineind.push_back(gdcit.Index());
    else
      gradientind.push_back(gdcit.Index());
  }

  if( m_DirectionsDuplicated )
  {
    int gradIndSize = gradientind.size();
    for(int i=0; i<gradIndSize; i++)
      gradientind.push_back(gradientind[i]);
  }

  while( !git.IsAtEnd() )
  {
    GradientVectorType b = git.Get();

    typename NumericTraits<ReferencePixelType>::AccumulateType b0 = NumericTraits<ReferencePixelType>::Zero;

    // Average the baseline image pixels
    for(unsigned int i = 0; i < baselineind.size(); ++i)
    {
      b0 += b[baselineind[i]];
    }
    b0 /= this->m_NumberOfBaselineImages;

    OdfPixelType odf(0.0);

    typename CoefficientImageType::PixelType coeffPixel(0.0);
    vnl_vector<TO> B(m_NumberOfGradientDirections);

    if( (b0 != 0) && (b0 >= m_Threshold) )
    {
      for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
      {
        B[i] = static_cast<TO>(b[gradientind[i]]);
      }

      B = PreNormalize(B, b0);
      if(m_NormalizationMethod == QBAR_SOLID_ANGLE)
      {
        vnl_vector<TO> coeffs(m_NumberCoefficients);
        coeffs = ( m_CoeffReconstructionMatrix * B );
        coeffs[0] += 1.0/(2.0*sqrt(itk::Math::pi));
        odf = ( m_SphericalHarmonicBasisMatrix * coeffs ).data_block();
        coeffPixel = coeffs.data_block();
      }
      else if(m_NormalizationMethod == QBAR_NONNEG_SOLID_ANGLE)
      {
        /** this would be the place to implement a non-negative
              * solver for quadratic programming problem:
              * min .5*|| Bc-s ||^2 subject to -CLPc <= 4*pi*ones
              * (refer to MICCAI 2009 Goh et al. "Estimating ODFs with PDF constraints")
              * .5*|| Bc-s ||^2 == .5*c'B'Bc - x'B's + .5*s's
              */

        itkExceptionMacro( << "Nonnegative Solid Angle not yet implemented");
      }
      else
      {
        vnl_vector<TO> coeffs(m_NumberCoefficients);
        coeffs = ( m_CoeffReconstructionMatrix * B );
        coeffs[0] += 1.0/(2.0*sqrt(itk::Math::pi));
        coeffPixel = coeffs.data_block();
        odf = ( m_ReconstructionMatrix * B ).data_block();
      }
      odf = Normalize(odf, b0);
    }

    oit.Set( odf );
    oit2.Set( b0 );
    float sum = 0;
    for (unsigned int k=0; k<odf.Size(); k++)
      sum += (float) odf[k];
    oit3.Set( sum-1 );
    oit4.Set(coeffPixel);
    ++oit;  // odf image iterator
    ++oit3; // odf sum image iterator
    ++oit2; // b0 image iterator
    ++oit4; // coefficient image iterator
    ++git;  // Gradient  image iterator
  }

  std::cout << "One Thread finished reconstruction" << std::endl;
}

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>
::tofile2(vnl_matrix<float> *pA, std::string fname)
{
  vnl_matrix<float> A = (*pA);
  std::ofstream myfile;
  std::locale C("C");
  std::locale originalLocale = myfile.getloc();
  myfile.imbue(C);

  myfile.open (fname.c_str());
  myfile << "A1=[";
  for(unsigned int i=0; i<A.rows(); i++)
  {
    for(unsigned int j=0; j<A.columns(); j++)
    {
      myfile << A(i,j) << " ";
      if(j==A.columns()-1 && i!=A.rows()-1)
        myfile << ";";
    }
  }
  myfile << "];";
  myfile.close();

  myfile.imbue( originalLocale );
}

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>::ComputeReconstructionMatrix()
{
  m_NumberCoefficients = (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder;
  if( m_NumberOfGradientDirections < m_NumberCoefficients )
  {
    itkExceptionMacro( << "Not enough gradient directions supplied (" << m_NumberOfGradientDirections << "). At least " << (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder << " needed for SH-order " << ShOrder);
  }

  // Gradient preprocessing
  {
    // check for duplicate diffusion gradients
    bool warning = false;
    for(GradientDirectionContainerType::ConstIterator gdcit1 = this->m_GradientDirectionContainer->Begin();
        gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
    {
      for(GradientDirectionContainerType::ConstIterator gdcit2 = this->m_GradientDirectionContainer->Begin();
          gdcit2 != this->m_GradientDirectionContainer->End(); ++gdcit2)
      {
        if(gdcit1.Value() == gdcit2.Value() && gdcit1.Index() != gdcit2.Index())
        {
          itkWarningMacro( << "Some of the Diffusion Gradients equal each other. Corresponding image data should be averaged before calling this filter." );
          warning = true;
          break;
        }
      }
      if (warning)
        break;
    }

    // handle acquisition schemes where only half of the spherical
    // shell is sampled by the gradient directions. In this case,
    // each gradient direction is duplicated in negative direction.
    vnl_vector<double> centerMass(3);
    centerMass.fill(0.0);
    int count = 0;
    for(GradientDirectionContainerType::ConstIterator gdcit1 = this->m_GradientDirectionContainer->Begin();
        gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
    {
      float bval = gdcit1.Value().two_norm();
      bval = bval*bval*m_BValue;
      if(bval > 100)
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

  // Create 3xM matrix Q that contains the gradient vectors in polar coordinates
  vnl_matrix<float> Q(3, m_NumberOfGradientDirections);
  {
    int i = 0;
    for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
    {
      float bval = gdcit.Value().two_norm();
      bval = bval*bval*m_BValue;
      if(bval > 100)
      {
        double x = gdcit.Value().get(0);
        double y = gdcit.Value().get(1);
        double z = gdcit.Value().get(2);
        double cart[3];
        mitk::sh::Cart2Sph(x,y,z,cart);
        Q(0,i) = cart[0];
        Q(1,i) = cart[1];
        Q(2,i++) = cart[2];
      }
    }
    if(m_DirectionsDuplicated)
    {
      for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
      {
        float bval = gdcit.Value().two_norm();
        bval = bval*bval*m_BValue;
        if(bval > 100)
        {
          double x = gdcit.Value().get(0);
          double y = gdcit.Value().get(1);
          double z = gdcit.Value().get(2);
          double cart[3];
          mitk::sh::Cart2Sph(x,y,z,cart);
          Q(0,i) = cart[0];
          Q(1,i) = cart[1];
          Q(2,i++) = cart[2];
        }
      }
    }
  }

  // Calcualte SH basis B
  vnl_matrix<float> L(m_NumberCoefficients,m_NumberCoefficients, 0);
  vnl_vector<int> lj(m_NumberCoefficients);
  vnl_matrix<float> B(m_NumberOfGradientDirections,m_NumberCoefficients);
  for(unsigned int i=0; i<m_NumberOfGradientDirections; i++)
  {
    for(int k=0; k<=ShOrder; k+=2)
    {
      for(int m=-k; m<=k; m++)
      {
        int j = (k*k + k + 2)/2 + m - 1;
        L(j,j) = -k*(k+1);
        lj[j] = k;
        B(i,j) = mitk::sh::Yj(m, k, Q(1,i), Q(0,i));
      }
    }
  }

  vnl_matrix<float> P(m_NumberCoefficients,m_NumberCoefficients, 0);
  for(unsigned int i=0; i<m_NumberCoefficients; i++)
    P(i,i) = mitk::sh::legendre0(lj[i]);

  vnl_matrix<float> B_transpose(B.transpose());
  vnl_matrix_inverse<float>* pseudoInverse = new vnl_matrix_inverse<float>( B_transpose * B + L*L*m_Lambda );
  m_CoeffReconstructionMatrix = pseudoInverse->pinverse() * B_transpose;

  switch(m_NormalizationMethod)
  {
  case QBAR_ADC_ONLY:
  case QBAR_RAW_SIGNAL:
    break;
  case QBAR_STANDARD:
  case QBAR_B_ZERO_B_VALUE:
  case QBAR_B_ZERO:
  case QBAR_NONE:
  {
    m_CoeffReconstructionMatrix = P * m_CoeffReconstructionMatrix;
    break;
  }
  case QBAR_SOLID_ANGLE:
  {
    m_CoeffReconstructionMatrix = (float)(1.0/(8.0*itk::Math::pi)) * P * L * m_CoeffReconstructionMatrix;
    break;
  }
  case QBAR_NONNEG_SOLID_ANGLE:
    break;
  }

  // needed to calculate the ODF values from the SH coefficients
  vnl_matrix_fixed<double, 3, NrOdfDirections>* U = itk::PointShell<NrOdfDirections, vnl_matrix_fixed<double, 3, NrOdfDirections> >::DistributePointShell();

  m_SphericalHarmonicBasisMatrix  = mitk::sh::CalcShBasisForDirections(ShOrder, U->as_matrix());
  m_ReconstructionMatrix = m_SphericalHarmonicBasisMatrix * m_CoeffReconstructionMatrix;
}

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>
::SetGradientImage(const GradientDirectionContainerType *gradientDirection,
                   const GradientImagesType *gradientImage )
{
  // Copy Gradient Direction Container
  this->m_GradientDirectionContainer = GradientDirectionContainerType::New();
  for(GradientDirectionContainerType::ConstIterator it = gradientDirection->Begin();
      it != gradientDirection->End(); it++)
  {
    this->m_GradientDirectionContainer->push_back(it.Value());
  }

  if (m_BValue<0)
    itkExceptionMacro("B-value needs to best before gradient image!");

  unsigned int numImages = gradientDirection->Size();
  this->m_NumberOfBaselineImages = 0;
  for(GradientDirectionContainerType::Iterator it = this->m_GradientDirectionContainer->Begin();
      it != this->m_GradientDirectionContainer->End(); it++)
  {
      float bval = it.Value().two_norm();
      bval = bval*bval*m_BValue;
      if(bval < 100)
      {
          this->m_NumberOfBaselineImages++;
      }
      else // Normalize non-zero gradient directions
      {
          it.Value() = it.Value() / it.Value().two_norm();
      }
  }

  if (this->m_NumberOfBaselineImages==0)
    itkExceptionMacro("No baseline image detected (no b-zero image)");

  this->m_NumberOfGradientDirections = numImages - this->m_NumberOfBaselineImages;

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

template< class T, class TG, class TO, int ShOrder, int NrOdfDirections>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,ShOrder,NrOdfDirections>
::PrintSelf(std::ostream& os, Indent indent) const
{
  std::locale C("C");
  std::locale originalLocale = os.getloc();
  os.imbue(C);

  Superclass::PrintSelf(os,indent);

  os << indent << "OdfReconstructionMatrix: " << m_ReconstructionMatrix << std::endl;
  if ( m_GradientDirectionContainer )
    os << indent << "GradientDirectionContainer: " << m_GradientDirectionContainer << std::endl;
  else
    os << indent << "GradientDirectionContainer: (Gradient directions not set)" << std::endl;

  os << indent << "NumberOfGradientDirections: " <<  m_NumberOfGradientDirections << std::endl;
  os << indent << "NumberOfBaselineImages: " << m_NumberOfBaselineImages << std::endl;
  os << indent << "Threshold for reference B0 image: " << m_Threshold << std::endl;
  os << indent << "BValue: " << m_BValue << std::endl;
  os.imbue( originalLocale );
}

}

#endif // __itkAnalyticalDiffusionQballReconstructionImageFilter_cpp
