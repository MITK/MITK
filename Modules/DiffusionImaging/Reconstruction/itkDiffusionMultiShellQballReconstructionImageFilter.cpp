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
            veci[i] = sigma / 2 + (vec[i] * vec[i]) / 2 * sigma;
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
        {
            this->m_NumberOfBaselineImages ++;
        }else{
            it.Value() = it.Value() / it.Value().two_norm();
        }
    }

}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::BeforeThreadedGenerateData()
{
    if( m_NumberOfGradientDirections < 6 )
    {
        itkExceptionMacro( << "At least 6 gradient directions are required" );
    }
}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int NumberOfThreads)
{

}

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::ComputeReconstructionMatrix()
{


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

template< class T, class TG, class TO, int L, int NODF>
void DiffusionMultiShellQballReconstructionImageFilter<T,TG,TO,L,NODF>
::tofile2(vnl_matrix<double> *pA, std::string fname)
{
    vnl_matrix<double> A = (*pA);
    ofstream myfile;
    std::locale C("C");
    std::locale originalLocale = myfile.getloc();
    myfile.imbue(C);

    myfile.open (fname.c_str());
    myfile << "A1=[";
    for(int i=0; i<A.rows(); i++)
    {
        for(int j=0; j<A.columns(); j++)
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


}

#endif // __itkDiffusionMultiShellQballReconstructionImageFilter_cpp
