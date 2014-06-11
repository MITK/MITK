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

#ifndef __itkDwiNormilzationFilter_txx
#define __itkDwiNormilzationFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"

namespace itk {


template< class TInPixelType >
DwiNormilzationFilter< TInPixelType>::DwiNormilzationFilter()
    :m_B0Index(-1)
    ,m_NewMax(1000)
    ,m_UseGlobalMax(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType >
void DwiNormilzationFilter< TInPixelType>::BeforeThreadedGenerateData()
{
    m_B0Index = -1;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
    {
        GradientDirectionType g = m_GradientDirections->GetElement(i);
        if (g.magnitude()<0.001)
            m_B0Index = i;
    }
    if (m_B0Index==-1)
        itkExceptionMacro(<< "DwiNormilzationFilter: No b-Zero indecies found!");

    m_GlobalMax = 0;
    if (m_UseGlobalMax)
    {
        typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
        typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

        InputIteratorType git( inputImagePointer, inputImagePointer->GetLargestPossibleRegion() );
        git.GoToBegin();
        while( !git.IsAtEnd() )
        {
            if (git.Get()[m_B0Index]>m_GlobalMax)
                m_GlobalMax = git.Get()[m_B0Index];
            ++git;
        }
    }
}

template< class TInPixelType >
void DwiNormilzationFilter< TInPixelType>::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::PixelType nullPix;
    nullPix.SetSize(inputImagePointer->GetVectorLength());
    nullPix.Fill(0);

    InputIteratorType git( inputImagePointer, outputRegionForThread );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
        typename InputImageType::PixelType pix = git.Get();

        double S0 = pix[m_B0Index];
        if (m_UseGlobalMax)
            S0 = m_GlobalMax;

        if (S0>0.1)
        {
            for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
            {
                double val = (double)pix[i];

                if (val!=val || val<0)
                    val = 0;
                else
                {
                    val /= S0;
                    val *= (double)m_NewMax;
                }
                pix[i] = (TInPixelType)val;
            }
            oit.Set(pix);
        }
        else
            oit.Set(nullPix);

        ++oit;
        ++git;
    }

    std::cout << "One Thread finished calculation" << std::endl;
}

template< class TInPixelType >
void
DwiNormilzationFilter< TInPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
    Superclass::PrintSelf(os,indent);
}

}

#endif // __itkDwiNormilzationFilter_txx
