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

#ifndef __itkRemoveDwiChannelFilter_txx
#define __itkRemoveDwiChannelFilter_txx

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
RemoveDwiChannelFilter< TInPixelType>::RemoveDwiChannelFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType >
void RemoveDwiChannelFilter< TInPixelType>::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    if ( inputImagePointer->GetVectorLength()-m_ChannelIndices.size()<=0 )
        itkExceptionMacro("No channels remaining!");

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    outputImage->SetSpacing( inputImagePointer->GetSpacing() );
    outputImage->SetOrigin( inputImagePointer->GetOrigin() );
    outputImage->SetDirection( inputImagePointer->GetDirection() );
    outputImage->SetLargestPossibleRegion( inputImagePointer->GetLargestPossibleRegion() );
    outputImage->SetBufferedRegion( inputImagePointer->GetLargestPossibleRegion() );
    outputImage->SetRequestedRegion( inputImagePointer->GetLargestPossibleRegion() );
    outputImage->Allocate();
    outputImage->SetVectorLength( inputImagePointer->GetVectorLength()-m_ChannelIndices.size() );
    typename OutputImageType::PixelType nullPix;
    nullPix.SetSize(outputImage->GetVectorLength());
    nullPix.Fill(0);
    outputImage->FillBuffer(nullPix);
    this->SetNthOutput(0, outputImage);

    m_NewDirections = DirectionContainerType::New();

    int chIdx = 0;
    for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
    {
        bool use = true;
        for (unsigned int j=0; j<m_ChannelIndices.size(); j++)
            if (m_ChannelIndices.at(j)==i)
            {
                use = false;
                break;
            }
        if (use)
        {
            m_NewDirections->InsertElement(chIdx, m_Directions->GetElement(i));
            ++chIdx;
            MITK_INFO << "Using channel " << i;
        }
    }
}

template< class TInPixelType >
void RemoveDwiChannelFilter< TInPixelType>::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType id )
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    InputIteratorType git( inputImagePointer, outputRegionForThread );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
        int chIdx = 0;
        typename OutputImageType::PixelType pix = oit.Get();
        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
        {
            bool use = true;
            for (unsigned int j=0; j<m_ChannelIndices.size(); j++)
                if (m_ChannelIndices.at(j)==i)
                {
                    use = false;
                    break;
                }
            if (use)
            {
                pix[chIdx] = git.Get()[i];
                ++chIdx;
            }
        }
        oit.Set(pix);
        ++oit;
        ++git;
    }

    std::cout << "One Thread finished calculation" << std::endl;
}

template< class TInPixelType >
void
RemoveDwiChannelFilter< TInPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
    Superclass::PrintSelf(os,indent);
}

}

#endif // __itkRemoveDwiChannelFilter_txx
