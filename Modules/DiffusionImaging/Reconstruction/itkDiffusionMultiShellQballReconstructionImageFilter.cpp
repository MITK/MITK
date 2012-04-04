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
    m_IsHemisphericalArrangementOfGradientDirections(false),
    m_IsArithmeticProgession(false),
    m_ReconstructionType(Standard1Shell)
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
void itk::DiffusionMultiShellQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType,NOrderL, NrOdfDirections>
::PreNormalize( vnl_vector<TOdfPixelType> & vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
{
    // Threshold
    double sigma = 0.001;

    double b0f = (double)b0;
    for(int i = 0; i < vec.size(); i++)
    {

        if (b0f==0){
            MITK_INFO << b0f;
            b0f = 0.001;
        }

        vec[i] /= b0f;

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
    }

}


template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::DoubleLogarithm(vnl_vector<TO> & vec)
{
    for(int i = 0; i < vec.size(); i++)
    {
        vec[i] = log(-log(vec[i]));
    }
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

    m_BZeroImage = BZeroImageType::New();
    typename GradientImagesType::Pointer img = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );
    m_BZeroImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
    m_BZeroImage->SetOrigin( img->GetOrigin() );     // Set the image origin
    m_BZeroImage->SetDirection( img->GetDirection() );  // Set the image direction
    m_BZeroImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
    m_BZeroImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
    m_BZeroImage->Allocate();

    // if no GradientIndexMap is set take all Gradients to GradientIndicies-Vector and add this to GradientIndexMap[ALL]
    // add All Bzero
    if(m_GradientIndexMap.size() == 0){

        // split for all gradient directions the image in baseline-indicies and gradient-indicies for a fast access
        GradientDirectionContainerType::ConstIterator gdcit;
        for( gdcit = this->m_GradientDirectionContainer->Begin(); gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
        {
            if(gdcit.Value().one_norm() <= 0.0) // if vector length at current position is 0, it is a basline signal
            {
                m_GradientIndexMap[0].push_back(gdcit.Index());
            }else{ // it is a gradient signal of length != 0
                m_GradientIndexMap[1].push_back(gdcit.Index());
            }
        }
    }

    if(m_GradientIndexMap.size() == 4){

        GradientIndexMapIteraotr it = m_GradientIndexMap.begin();
        it++;
        int b1 = (*it).first;
        it++;
        int b2 = (*it).first;
        it++;
        int b3 = (*it).first;


        if(b2 - b1 == b1 && b3 - b2 == b1 )
        {
            m_ReconstructionType = Analytical3Shells;
        }else
        {
            m_ReconstructionType = NumericalNShells;
        }
    }else if(m_GradientIndexMap.size() > 2)
    {
        m_ReconstructionType = NumericalNShells;
    }else{
        m_ReconstructionType = Standard1Shell;
    }



    switch(m_ReconstructionType)
    {
    case Analytical3Shells:
    {
        GradientIndexMapIteraotr it = m_GradientIndexMap.begin();
        it++;
        this->ComputeReconstructionMatrix((*it).second);
        break;
    }
    case NumericalNShells:
        this->ComputeReconstructionMatrix(m_GradientIndexMap[1]);
        break;
    case Standard1Shell:
        this->ComputeReconstructionMatrix(m_GradientIndexMap[1]);
        break;
    }


}



template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::StandardOneShellReconstruction(const OutputImageRegionType& outputRegionForThread)
{
    // Get output image pointer
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    // ImageRegionIterator for the output image
    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    // ImageRegionIterator for the BZero (output) image
    ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
    oit2.GoToBegin();

    IndiciesVector BZeroIndicies = m_GradientIndexMap[0];
    IndiciesVector SignalIndicies = m_GradientIndexMap[1];

    // if the gradient directiosn aragement is hemispherical, duplicate all gradient directions
    // alone, interested in the value, the direction can be neglected
    if(m_IsHemisphericalArrangementOfGradientDirections){
        int NumbersOfGradientIndicies = SignalIndicies.size();
        for (int i = 0 ; i < NumbersOfGradientIndicies; i++)
            SignalIndicies.push_back(SignalIndicies[i]);
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
        for(unsigned int i = 0; i < BZeroIndicies.size(); ++i)
        {
            b0 += b[BZeroIndicies[i]];
        }
        b0 /= this->m_NumberOfBaselineImages;

        // ODF Vector
        OdfPixelType odf(0.0);

        // Create the Signal Vector
        vnl_vector<TO> SignalVector(m_NumberOfGradientDirections);
        if( (b0 != 0) && (b0 >= m_Threshold) )
        {

            for( unsigned int i = 0; i< SignalIndicies.size(); i++ )
            {
                SignalVector[i] = static_cast<TO>(b[SignalIndicies[i]]);
            }

            // apply threashold an generate ln(-ln(E)) signal
            // Replace SignalVector with PreNormalized SignalVector
            PreNormalize(SignalVector, b0);
            DoubleLogarithm(SignalVector);

            // ODF coeffs-vector
            vnl_vector<TO> coeffs(m_NumberCoefficients);

            // approximate ODF coeffs
            coeffs = ( (*m_CoeffReconstructionMatrix) * SignalVector );
            coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));

            odf = ( (*m_ODFSphericalHarmonicBasisMatrix) * coeffs ).data_block();
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
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::AnalyticalThreeShellReconstruction(const OutputImageRegionType& outputRegionForThread)
{

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
    oit2.GoToBegin();

    IndiciesVector BZeroIndicies = m_GradientIndexMap[0];

    GradientIndexMapIteraotr it = m_GradientIndexMap.begin();
    it++;
    IndiciesVector Shell1Indiecies = (*it).second;
    it++;
    IndiciesVector Shell2Indiecies = (*it).second;
    it++;
    IndiciesVector Shell3Indiecies = (*it).second;

    assert(Shell1Indiecies.size() == Shell2Indiecies.size() && Shell1Indiecies.size() == Shell3Indiecies.size());

    if(m_IsHemisphericalArrangementOfGradientDirections){
        int NumbersOfGradientIndicies = Shell1Indiecies.size();
        for (int i = 0 ; i < NumbersOfGradientIndicies; i++){
            Shell1Indiecies.push_back(Shell1Indiecies[i]);
            Shell2Indiecies.push_back(Shell2Indiecies[i]);
            Shell3Indiecies.push_back(Shell3Indiecies[i]);
        }
    }

    // Get input gradient image pointer
    typename GradientImagesType::Pointer gradientImagePointer = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );

    //  Const ImageRegionIterator for input gradient image
    typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
    GradientIteratorType git(gradientImagePointer, outputRegionForThread );
    git.GoToBegin();

    typedef typename GradientImagesType::PixelType         GradientVectorType;


    vnl_matrix<TO> SHBasis(m_SHBasisMatrix->rows(),m_SHBasisMatrix->cols());
    SHBasis.fill(0.0);

    for(int i=0; i<SHBasis.rows(); i++)
        for(unsigned int j=0; j<SHBasis.cols(); j++)
            SHBasis(i,j) = (TO)((*m_SHBasisMatrix)(i,j));


    vnl_matrix<TO> ReconstructionMatrix(m_SignalReonstructionMatrix->rows(),m_SignalReonstructionMatrix->cols());
    ReconstructionMatrix.fill(0.0);

    for(int i=0; i < ReconstructionMatrix.rows(); i++)
        for(unsigned int j=0; j < ReconstructionMatrix.cols(); j++)
            ReconstructionMatrix(i,j) = (TO)(*m_SignalReonstructionMatrix)(i,j);

    // iterate overall voxels of the gradient image region
    while( ! git.IsAtEnd() )
    {
        GradientVectorType b = git.Get();

        // compute the average bzero signal
        typename NumericTraits<ReferencePixelType>::AccumulateType b0 = NumericTraits<ReferencePixelType>::Zero;
        for(unsigned int i = 0; i < BZeroIndicies.size(); ++i)
        {
            b0 += b[BZeroIndicies[i]];
        }
        b0 /= this->m_NumberOfBaselineImages;

        // ODF Vector
        OdfPixelType odf(0.0);

        // Create the Signal Vector
        //vnl_vector<TO> SignalVector(m_NumberOfGradientDirections);
        if( (b0 != 0) && (b0 >= m_Threshold) )
        {

            vnl_vector< TO > Shell1OriginalSignal(Shell1Indiecies.size());
            Shell1OriginalSignal.fill(0.0);
            vnl_vector< TO > Shell2OriginalSignal(Shell1Indiecies.size());
            Shell2OriginalSignal.fill(0.0);
            vnl_vector< TO > Shell3OriginalSignal(Shell1Indiecies.size());
            Shell3OriginalSignal.fill(0.0);

            for(int i = 0 ; i < Shell2Indiecies.size(); i++)
            {
                Shell1OriginalSignal[i] = static_cast<TO>(b[Shell1Indiecies[i]]);
                Shell2OriginalSignal[i] = static_cast<TO>(b[Shell2Indiecies[i]]);
                Shell3OriginalSignal[i] = static_cast<TO>(b[Shell3Indiecies[i]]);
            }

            vnl_vector< TO > Shell2Coefficients(Shell1Indiecies.size());
            Shell2Coefficients.fill(0.0);
            vnl_vector< TO > Shell3Coefficients(Shell1Indiecies.size());
            Shell3Coefficients.fill(0.0);



            Shell2Coefficients = ReconstructionMatrix * Shell2OriginalSignal;
            Shell3Coefficients = ReconstructionMatrix * Shell3OriginalSignal;


            vnl_vector< TO > SHApproximatedSignal2(Shell1Indiecies.size());
            SHApproximatedSignal2.fill(0.0);
            vnl_vector< TO > SHApproximatedSignal3(Shell1Indiecies.size());
            SHApproximatedSignal3.fill(0.0);
            // MITK_INFO << "Shell2Coefficients : " << Shell2Coefficients.size();
            // MITK_INFO << "ShellCoeffs : " << m_SHBasisMatrix->cols() << " " << m_SHBasisMatrix->rows();



            SHApproximatedSignal2 = SHBasis * Shell2Coefficients;
            SHApproximatedSignal3 = SHBasis * Shell3Coefficients;

            PreNormalize(Shell1OriginalSignal,b0);
            PreNormalize(SHApproximatedSignal2,b0);
            PreNormalize(SHApproximatedSignal3,b0);


            vnl_vector<TO> AlphaValues(Shell1Indiecies.size());
            vnl_vector<TO> BetaValues(Shell1Indiecies.size());
            vnl_vector<TO> LAValues(Shell1Indiecies.size());



            for( unsigned int i = 0; i< Shell1Indiecies.size(); i++ )
            {
                float E1 = Shell1OriginalSignal[i];
                float E2 = SHApproximatedSignal2[i];
                float E3 = SHApproximatedSignal3[i];

               /* if(!(0 < E3))
                {
                    //MITK_INFO << "0 < E3 ";
                    E3 = 0.0001;
                }
                if(!(E3 < E2))
                {
                    ///MITK_INFO << "E3 < E2 ";
                    E3 = (E3 + E2) / 2;
                }
                if(!(E2 < E1))
                {
                    // MITK_INFO << "E2 < E1 ";
                    E2 = (E2 + E1) / 2;
                }
                if(!(E1 < 1))
                {
                    // MITK_INFO << "E1 < 1 ";
                    E1 = 1 - 0.0001;
                }

                if(!(E1 * E1 < E2))
                {
                    // MITK_INFO << "E1 < 1 ";
                    E1 = E2 - 0.5;
                }
                if(!(E2 * E2 < E1 * E3))
                {
                    // MITK_INFO << "E1 < 1 ";
                    E2 = E2 - 0.5;
                }
*/



                float A = (E3 -E1*E2) / (2*(E2-E1*E1)) ;
                float B = sqrt( A * A - ((E1*E3 - E2 * E2) / (E2-E1*E1)) );
                //MITK_INFO << A << "   " << B;

                AlphaValues[i] = A + B;
                BetaValues[i] = A - B;
                LAValues[i] = 0.5 + ((E1 - A)/(2*B));

                if(A != A || B != B || LAValues[i] != LAValues[i])
                {
                //    MITK_INFO << A << " " << B << " " << LAValues[i];
                }

            }

            DoubleLogarithm(AlphaValues);
            DoubleLogarithm(BetaValues);

            vnl_vector<TO> SignalVector(Shell1Indiecies.size());
            for( unsigned int i = 0; i< AlphaValues.size(); i++ )
            {
                SignalVector = LAValues[i] * AlphaValues + LAValues[i] * BetaValues;
            }

            vnl_vector<TO> coeffs(m_NumberCoefficients);

            coeffs = ( (*m_CoeffReconstructionMatrix) * SignalVector );
            coeffs[0] += 1.0/(2.0*sqrt(QBALL_ANAL_RECON_PI));

            odf = ( (*m_ODFSphericalHarmonicBasisMatrix) * coeffs ).data_block();

        }
        // set ODF to ODF-Image
        oit.Set( odf );
        ++oit;
        // MITK_INFO << odf;
        // set b0(average) to b0-Image
        oit2.Set( b0 );
        ++oit2;

        ++git;

    }


    MITK_INFO << "THREAD FINISHED";


}


template< class T, class TG, class TO, int L, int NODF>
vnl_vector<TO> DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::AnalyticalThreeShellParameterEstimation(const IndiciesVector * shell1Indicies,const IndiciesVector * shell2Indicies ,const IndiciesVector * shell3Indicies, vnl_vector<TO>)
{



}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int NumberOfThreads)
{
    switch(m_ReconstructionType)
    {
    case Standard1Shell:
        StandardOneShellReconstruction(outputRegionForThread);
        break;
    case Analytical3Shells:
        AnalyticalThreeShellReconstruction(outputRegionForThread);
        break;
    case NumericalNShells:
        break;
    }



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
::ComputeReconstructionMatrix(std::vector< unsigned int > gradientIndiciesVector)
{

    typedef std::auto_ptr< vnl_matrix< double> >  MatrixDoublePtr;
    typedef std::auto_ptr< vnl_vector< int > >    VectorIntPtr;
    typedef std::auto_ptr< vnl_matrix_inverse< double > >  InverseMatrixDoublePtr;

    int numberOfGradientDirections = gradientIndiciesVector.size();

    MITK_INFO << numberOfGradientDirections << "NUMBEROFGRADIENTS";

    if( numberOfGradientDirections < (((L+1)*(L+2))/2) || numberOfGradientDirections < 6  )
    {
        itkExceptionMacro( << "At least (L+1)(L+2)/2 gradient directions are required" );
    }

    CheckDuplicateDiffusionGradients();


    // check if gradient directions are arrangement as a hemisphere(true) or sphere(false)
    m_IsHemisphericalArrangementOfGradientDirections = CheckHemisphericalArrangementOfGradientDirections();
    if(m_IsHemisphericalArrangementOfGradientDirections) numberOfGradientDirections *= 2;


    MatrixDoublePtr Q(new vnl_matrix<double>(3, numberOfGradientDirections));
    Q->fill(0.0);
    // Cartesian to spherical coordinates
    {
        int j = 0;

        for(int i = 0; i < gradientIndiciesVector.size(); i++)
        {

            double x = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(0);
            double y = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(1);
            double z = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(2);
            double cart[3];
            mitk::ShericalHarmonicsFunctions::Cart2Sph(x,y,z,cart);
            (*Q)(0,j) = cart[0];
            (*Q)(1,j) = cart[1];
            (*Q)(2,j++) = cart[2];

        }
        if(m_IsHemisphericalArrangementOfGradientDirections)
        {
            for(int i = 0; i < gradientIndiciesVector.size(); i++)
            {
                double x = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(0);
                double y = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(1);
                double z = this->m_GradientDirectionContainer->ElementAt(gradientIndiciesVector[i]).get(2);
                double cart[3];
                mitk::ShericalHarmonicsFunctions::Cart2Sph(x,y,z,cart);
                (*Q)(0,j) = cart[0];
                (*Q)(1,j) = cart[1];
                (*Q)(2,j++) = cart[2];
            }
        }
    }

    int LOrder = L;
    m_NumberCoefficients = (int)(LOrder*LOrder + LOrder + 2.0)/2.0 + LOrder;

    m_SHBasisMatrix = new vnl_matrix<double>(numberOfGradientDirections,m_NumberCoefficients);
    m_SHBasisMatrix->fill(0.0);
    VectorIntPtr SHOrderAssociation(new vnl_vector<int>(m_NumberCoefficients));
    SHOrderAssociation->fill(0.0);
    MatrixDoublePtr LaplacianBaltrami(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    LaplacianBaltrami->fill(0.0);
    MatrixDoublePtr FRTMatrix(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    FRTMatrix->fill(0.0);
    MatrixDoublePtr SHEigenvalues(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    SHEigenvalues->fill(0.0);



    // SHBasis-Matrix + LaplacianBaltrami-Matrix + SHOrderAssociationVector
    ComputeSphericalHarmonicsBasis(Q.get() ,m_SHBasisMatrix, LaplacianBaltrami.get(), SHOrderAssociation.get(), SHEigenvalues.get());

    // Compute FunkRadon Transformation Matrix Associated to SHBasis Order lj
    ComputeFunkRadonTransformationMatrix(SHOrderAssociation.get() ,FRTMatrix.get());

    MatrixDoublePtr temp(new vnl_matrix<double>(((m_SHBasisMatrix->transpose()) * (*m_SHBasisMatrix)) + (m_Lambda  * (*LaplacianBaltrami))));

    InverseMatrixDoublePtr pseudo_inv(new vnl_matrix_inverse<double>((*temp)));
    MatrixDoublePtr inverse(new vnl_matrix<double>(m_NumberCoefficients,m_NumberCoefficients));
    inverse->fill(0.0);
    (*inverse) = pseudo_inv->inverse();

    // ODF Factor ( missing 1/4PI ?? )
    double factor = (1.0/(16.0*QBALL_ANAL_RECON_PI*QBALL_ANAL_RECON_PI));

    m_SignalReonstructionMatrix = new vnl_matrix<double>();
    m_SignalReonstructionMatrix->fill(0.0);
    (*m_SignalReonstructionMatrix) = (*inverse) * (m_SHBasisMatrix->transpose());

    MatrixDoublePtr TransformationMatrix(new vnl_matrix<double>( factor * ((*FRTMatrix) * ((*SHEigenvalues) * (*m_SignalReonstructionMatrix))) ) );

    m_CoeffReconstructionMatrix = new vnl_matrix<TO>(m_NumberCoefficients,numberOfGradientDirections);


    // Cast double to float
    for(int i=0; i<m_NumberCoefficients; i++)
        for(unsigned int j=0; j<numberOfGradientDirections; j++)
            (*m_CoeffReconstructionMatrix)(i,j) = (float)(*TransformationMatrix)(i,j);



    // this code goes to the image adapter coeffs->odfs later

    vnl_matrix_fixed<double, 3, NOdfDirections>* U = itk::PointShell<NOdfDirections, vnl_matrix_fixed<double, 3, NOdfDirections> >::DistributePointShell();

    m_ODFSphericalHarmonicBasisMatrix  = new vnl_matrix<TO>(NOdfDirections,m_NumberCoefficients);


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
                (*m_ODFSphericalHarmonicBasisMatrix)(i,j) = mitk::ShericalHarmonicsFunctions::Yj(m,k,th,phi);
            }
        }
    }

    m_ReconstructionMatrix = new vnl_matrix<TO>((*m_ODFSphericalHarmonicBasisMatrix) * (*m_CoeffReconstructionMatrix));

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

    }


    MITK_INFO << stream.str();

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
