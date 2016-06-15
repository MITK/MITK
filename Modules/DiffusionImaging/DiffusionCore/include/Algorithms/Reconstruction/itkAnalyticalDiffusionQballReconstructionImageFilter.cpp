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

#include <boost/version.hpp>
#include <stdio.h>
#include <locale>
#include <fstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <boost/math/special_functions.hpp>

#include "itkPointShell.h"

using namespace boost::math;

namespace itk {

#define QBALL_ANAL_RECON_PI       M_PI

template< class T, class TG, class TO, int L, int NODF>
AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::AnalyticalDiffusionQballReconstructionImageFilter() :
    m_GradientDirectionContainer(NULL),
    m_NumberOfGradientDirections(0),
    m_NumberOfBaselineImages(1),
    m_Threshold(NumericTraits< ReferencePixelType >::NonpositiveMin()),
    m_BValue(1.0),
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
        int NOrderL,
        int NrOdfDirections>
typename itk::AnalyticalDiffusionQballReconstructionImageFilter<
TReferenceImagePixelType,TGradientImagePixelType,TOdfPixelType,
NOrderL,NrOdfDirections>::OdfPixelType
itk::AnalyticalDiffusionQballReconstructionImageFilter
<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType,
NOrderL, NrOdfDirections>
::Normalize( OdfPixelType odf,
             typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
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
            odf[i] *= QBALL_ANAL_RECON_PI*4/NrOdfDirections;

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
        int NOrderL,
        int NrOdfDirections>
vnl_vector<TOdfPixelType>
itk::AnalyticalDiffusionQballReconstructionImageFilter
<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType,
NOrderL, NrOdfDirections>
::PreNormalize( vnl_vector<TOdfPixelType> vec,
                typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
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

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::BeforeThreadedGenerateData()
{
    // If we have more than 2 inputs, then each input, except the first is a
    // gradient image. The number of gradient images must match the number of
    // gradient directions.
    //const unsigned int numberOfInputs = this->GetNumberOfInputs();

    // There need to be at least 6 gradient directions to be able to compute the
    // tensor basis
    if( m_NumberOfGradientDirections < (L*L + L + 2)/2 + L )
    {
        itkExceptionMacro( << "Not enough gradient directions supplied (" << m_NumberOfGradientDirections << "). At least " << (L*L + L + 2)/2 + L << " needed for SH-order " << L);
    }

    // Input must be an itk::VectorImage.
    std::string gradientImageClassName(
                this->ProcessObject::GetInput(0)->GetNameOfClass());
    if ( strcmp(gradientImageClassName.c_str(),"VectorImage") != 0 )
    {
        itkExceptionMacro( <<
                           "There is only one Gradient image. I expect that to be a VectorImage. "
                           << "But its of type: " << gradientImageClassName );
    }

    this->ComputeReconstructionMatrix();

    typename GradientImagesType::Pointer img = static_cast< GradientImagesType * >(
                this->ProcessObject::GetInput(0) );

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

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       ThreadIdType )
{
    typename OutputImageType::Pointer outputImage =
            static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

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
    typename GradientImagesType::Pointer gradientImagePointer = NULL;

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
        if(gdcit.Value().one_norm() <= 0.0)
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
                coeffs = ( (*m_CoeffReconstructionMatrix) * B );
                coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));
                odf = ( (*m_SphericalHarmonicBasisMatrix) * coeffs ).data_block();
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
                coeffs = ( (*m_CoeffReconstructionMatrix) * B );
                coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));
                coeffPixel = coeffs.data_block();
                odf = ( (*m_ReconstructionMatrix) * B ).data_block();
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

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::tofile2(vnl_matrix<double> *pA, std::string fname)
{
    vnl_matrix<double> A = (*pA);
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

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::Cart2Sph(double x, double y, double z, double *spherical)
{
    double phi, theta, r;
    r = sqrt(x*x+y*y+z*z);

    if( r<mitk::eps )
    {
        theta = M_PI/2;
        phi = M_PI/2;
    }
    else
    {
        theta = acos(z/r);
        phi = atan2(y, x);
    }
    spherical[0] = phi;
    spherical[1] = theta;
    spherical[2] = r;
}

template< class T, class TG, class TO, int L, int NODF>
double AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::Yj(int m, int l, double theta, double phi, bool useMRtrixBasis)
{
    if (!useMRtrixBasis)
    {
        if (m<0)
            return sqrt(2.0)*spherical_harmonic_r(l, -m, theta, phi);
        else if (m==0)
            return spherical_harmonic_r(l, m, theta, phi);
        else
            return pow(-1.0,m)*sqrt(2.0)*spherical_harmonic_i(l, m, theta, phi);
    }
    else
    {
        double plm = legendre_p<double>(l,abs(m),-cos(theta));
        double mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;
        if (m>0)
            return mag*cos(m*phi);
        else if (m==0)
            return mag;
        else
            return mag*sin(-m*phi);
    }

    return 0;
}

template< class T, class TG, class TO, int L, int NODF>
double AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::Legendre0(int l)
{
    if( l%2 != 0 )
    {
        return 0;
    }
    else
    {
        double prod1 = 1.0;
        for(int i=1;i<l;i+=2) prod1 *= i;
        double prod2 = 1.0;
        for(int i=2;i<=l;i+=2) prod2 *= i;
        return pow(-1.0,l/2.0)*(prod1/prod2);
    }
}

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ComputeReconstructionMatrix()
{

    //for(int i=-6;i<7;i++)
    //  std::cout << boost::math::legendre_p<double>(6, i, 0.65657) << std::endl;

    if( m_NumberOfGradientDirections < (L*L + L + 2)/2 + L )
    {
        itkExceptionMacro( << "Not enough gradient directions supplied (" << m_NumberOfGradientDirections << "). At least " << (L*L + L + 2)/2 + L << " needed for SH-order " << L);
    }

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
            if (warning) break;
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

    {
        int i = 0;
        for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
            gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
        {
            if(gdcit.Value().one_norm() > 0.0)
            {
                double x = gdcit.Value().get(0);
                double y = gdcit.Value().get(1);
                double z = gdcit.Value().get(2);
                double cart[3];
                Cart2Sph(x,y,z,cart);
                (*Q)(0,i) = cart[0];
                (*Q)(1,i) = cart[1];
                (*Q)(2,i++) = cart[2];
            }
        }
        if(m_DirectionsDuplicated)
        {
            for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
                gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
            {
                if(gdcit.Value().one_norm() > 0.0)
                {
                    double x = gdcit.Value().get(0);
                    double y = gdcit.Value().get(1);
                    double z = gdcit.Value().get(2);
                    double cart[3];
                    Cart2Sph(x,y,z,cart);
                    (*Q)(0,i) = cart[0];
                    (*Q)(1,i) = cart[1];
                    (*Q)(2,i++) = cart[2];
                }
            }
        }
    }

    int l = L;
    m_NumberCoefficients = (int)(l*l + l + 2.0)/2.0 + l;
    vnl_matrix<double>* B = new vnl_matrix<double>(m_NumberOfGradientDirections,m_NumberCoefficients);
    vnl_matrix<double>* _L = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
    _L->fill(0.0);
    vnl_matrix<double>* LL = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
    LL->fill(0.0);
    vnl_matrix<double>* P = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
    P->fill(0.0);
    vnl_matrix<double>* Inv = new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients);
    P->fill(0.0);
    vnl_vector<int>* lj = new vnl_vector<int>(m_NumberCoefficients);
    m_LP = new vnl_vector<double>(m_NumberCoefficients);

    for(unsigned int i=0; i<m_NumberOfGradientDirections; i++)
    {
        for(int k=0; k<=l; k+=2)
        {
            for(int m=-k; m<=k; m++)
            {
                int j = (k*k + k + 2)/2 + m - 1;
                (*_L)(j,j) = -k*(k+1);
                (*m_LP)(j) = -k*(k+1);
                (*lj)[j] = k;
                double phi = (*Q)(0,i);
                double th = (*Q)(1,i);
                (*B)(i,j) = Yj(m,k,th,phi, m_UseMrtrixBasis);
            }
        }
    }

    //vnl_matrix<double> temp((*_L)*(*_L));
    LL->update(*_L);
    *LL *= *_L;
    //tofile2(LL,"LL");

    for(int i=0; i<m_NumberCoefficients; i++)
    {
        // here we leave out the 2*pi multiplication from Descoteaux
        // this is because we do not want the real integral value
        // but an average value over the equator.

        if(m_NormalizationMethod == QBAR_SOLID_ANGLE || m_NormalizationMethod == QBAR_NONNEG_SOLID_ANGLE)
        {
            (*P)(i,i) = 2.0*QBALL_ANAL_RECON_PI*Legendre0((*lj)[i]);
            (*m_LP)(i) *= (*P)(i,i);
        }
        else
        {
            (*P)(i,i) = Legendre0((*lj)[i]);
        }
    }
    m_B_t = new vnl_matrix<double>(B->transpose());
    //tofile2(&m_B_t,"m_B_t");
    vnl_matrix<double> B_t_B = (*m_B_t) * (*B);
    //tofile2(&B_t_B,"B_t_B");
    vnl_matrix<double> lambdaLL(m_NumberCoefficients,m_NumberCoefficients);
    lambdaLL.update((*LL));
    lambdaLL *= m_Lambda;
    //tofile2(&lambdaLL,"lLL");

    vnl_matrix<double> tmp( B_t_B + lambdaLL);
    vnl_matrix_inverse<double> *pseudoInverse
            = new vnl_matrix_inverse<double>( tmp );

    (*Inv) = pseudoInverse->pinverse();
    //tofile2(Inv,"Inv");
    vnl_matrix<double> temp((*Inv) * (*m_B_t));
    double fac1 = (1.0/(16.0*QBALL_ANAL_RECON_PI*QBALL_ANAL_RECON_PI));
    switch(m_NormalizationMethod)
    {
    case QBAR_ADC_ONLY:
    case QBAR_RAW_SIGNAL:
        break;
    case QBAR_STANDARD:
    case QBAR_B_ZERO_B_VALUE:
    case QBAR_B_ZERO:
    case QBAR_NONE:
        temp = (*P) * temp;
        break;
    case QBAR_SOLID_ANGLE:
        temp = fac1 * (*P) * (*_L) * temp;
        break;
    case QBAR_NONNEG_SOLID_ANGLE:
        break;
    }

    //tofile2(&temp,"A");

    m_CoeffReconstructionMatrix = new vnl_matrix<TO>(m_NumberCoefficients,m_NumberOfGradientDirections);
    for(int i=0; i<m_NumberCoefficients; i++)
    {
        for(unsigned int j=0; j<m_NumberOfGradientDirections; j++)
        {
            (*m_CoeffReconstructionMatrix)(i,j) = (float) temp(i,j);
        }
    }

    // this code goes to the image adapter coeffs->odfs later

    int NOdfDirections = NODF;
    vnl_matrix_fixed<double, 3, NODF>* U =
            itk::PointShell<NODF, vnl_matrix_fixed<double, 3, NODF> >::DistributePointShell();

    m_SphericalHarmonicBasisMatrix  = new vnl_matrix<TO>(NOdfDirections,m_NumberCoefficients);
    vnl_matrix<double>* sphericalHarmonicBasisMatrix2
            = new vnl_matrix<double>(NOdfDirections,m_NumberCoefficients);
    for(int i=0; i<NOdfDirections; i++)
    {
        double x = (*U)(0,i);
        double y = (*U)(1,i);
        double z = (*U)(2,i);
        double cart[3];
        Cart2Sph(x,y,z,cart);
        (*U)(0,i) = cart[0];
        (*U)(1,i) = cart[1];
        (*U)(2,i) = cart[2];
    }

    for(int i=0; i<NOdfDirections; i++)
    {
        for(int k=0; k<=l; k+=2)
        {
            for(int m=-k; m<=k; m++)
            {
                int j = (k*k + k + 2)/2 + m - 1;
                double phi = (*U)(0,i);
                double th = (*U)(1,i);
                (*m_SphericalHarmonicBasisMatrix)(i,j) = Yj(m,k,th,phi,m_UseMrtrixBasis);
                (*sphericalHarmonicBasisMatrix2)(i,j) = (*m_SphericalHarmonicBasisMatrix)(i,j);
            }
        }
    }

    m_ReconstructionMatrix = new vnl_matrix<TO>(NOdfDirections,m_NumberOfGradientDirections);
    *m_ReconstructionMatrix = (*m_SphericalHarmonicBasisMatrix) * (*m_CoeffReconstructionMatrix);

}

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
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


    unsigned int numImages = gradientDirection->Size();
    this->m_NumberOfBaselineImages = 0;
    for(GradientDirectionContainerType::Iterator it = this->m_GradientDirectionContainer->Begin();
        it != this->m_GradientDirectionContainer->End(); it++)
    {
        if(it.Value().one_norm() <= 0.0)
        {
            this->m_NumberOfBaselineImages++;
        }
        else // Normalize non-zero gradient directions
        {
            it.Value() = it.Value() / it.Value().two_norm();
        }
    }

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

template< class T, class TG, class TO, int L, int NODF>
void AnalyticalDiffusionQballReconstructionImageFilter<T,TG,TO,L,NODF>
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
