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
#include <itkNumericTraits.h>

namespace itk {


template< class TInPixelType >
DwiNormilzationFilter< TInPixelType>::DwiNormilzationFilter()
    :m_B0Index(-1)
    ,m_ScalingFactor(1000)
    ,m_UseGlobalReference(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType >
void DwiNormilzationFilter< TInPixelType>::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    m_B0Index = -1;
    for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
    {
        GradientDirectionType g = m_GradientDirections->GetElement(i);
        if (g.magnitude()<0.001)
            m_B0Index = i;
    }
    if (m_B0Index==-1)
        itkExceptionMacro(<< "DwiNormilzationFilter: No b-Zero indecies found!");

    if (m_UseGlobalReference)
        MITK_INFO << "Using global reference value: " << m_Reference;
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
        typename OutputImageType::PixelType outPix;
        outPix.SetSize(inputImagePointer->GetVectorLength());

        double S0 = pix[m_B0Index];
        if (m_UseGlobalReference)
            S0 = m_Reference;

        if (S0>0.1 && pix[m_B0Index]>0)
        {
            for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
            {
                double val = (double)pix[i];

                if (val!=val || val<0)
                    val = 0;
                else
                {
                    val /= S0;
                    val *= (double)m_ScalingFactor;
                }
                outPix[i] = (TInPixelType)val;
            }
            oit.Set(outPix);
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
