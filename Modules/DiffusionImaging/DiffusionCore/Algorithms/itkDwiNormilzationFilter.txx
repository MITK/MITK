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
    ,m_NewMax(1000)
    ,m_UseGlobalMax(false)
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

//    // cleanup
//    itk::ImageRegionIterator<InputImageType> inIt(inputImagePointer, inputImagePointer->GetLargestPossibleRegion());
//    while ( !inIt.IsAtEnd() )
//    {
//        typename InputImageType::PixelType pix = inIt.Get();
//        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
//        {
//            if (pix[i]<0)
//                pix[i] = 0;
//            else if(pix[i]>pix[m_B0Index])
//                pix[i] = pix[m_B0Index];
//        }
//        inIt.Set(pix);
//        ++inIt;
//    }

//    if (m_MaskImage.IsNull())
//    {
//        m_MaskImage = UcharImageType::New();
//        m_MaskImage->SetRegions(inputImagePointer->GetLargestPossibleRegion());
//        m_MaskImage->SetOrigin(inputImagePointer->GetOrigin());
//        m_MaskImage->SetSpacing(inputImagePointer->GetSpacing());
//        m_MaskImage->SetDirection(inputImagePointer->GetDirection());
//        m_MaskImage->Allocate();
//        m_MaskImage->FillBuffer(1);
//    }

//    BinImageType::Pointer outputBinMaskImage = BinImageType::New();
//    outputBinMaskImage->SetRegions(m_MaskImage->GetLargestPossibleRegion());
//    outputBinMaskImage->SetOrigin(m_MaskImage->GetOrigin());
//    outputBinMaskImage->SetSpacing(m_MaskImage->GetSpacing());
//    outputBinMaskImage->SetDirection(m_MaskImage->GetDirection());
//    outputBinMaskImage->Allocate();

//    typename TInPixelImageType::Pointer b0Image = TInPixelImageType::New();
//    b0Image->SetRegions(m_MaskImage->GetLargestPossibleRegion());
//    b0Image->SetOrigin(m_MaskImage->GetOrigin());
//    b0Image->SetSpacing(m_MaskImage->GetSpacing());
//    b0Image->SetDirection(m_MaskImage->GetDirection());
//    b0Image->Allocate();
//    b0Image->FillBuffer(0);

//    itk::ImageRegionIterator<InputImageType> it(inputImagePointer, inputImagePointer->GetLargestPossibleRegion());
//    while( !it.IsAtEnd() )
//    {
//        if (it.Get()[m_B0Index]>0)
//            b0Image->SetPixel(it.GetIndex(), it.Get()[m_B0Index]);
//        if( m_MaskImage->GetPixel(it.GetIndex()) > 0)
//            outputBinMaskImage->SetPixel(it.GetIndex(), 1);
//        else
//            outputBinMaskImage->SetPixel(it.GetIndex(), 0);

//        ++it;
//    }

//    typename StatisticsFilterType::Pointer statistics = StatisticsFilterType::New();
//    statistics->SetLabelInput( outputBinMaskImage );
//    statistics->SetInput( b0Image );
//    statistics->Update();

//    typename HistogramType::Pointer histogram = HistogramType::New();
//    typename HistogramType::SizeType sz;
//    typename HistogramType::MeasurementVectorType lowerBound;
//    typename HistogramType::MeasurementVectorType upperBound;
//    sz.SetSize(1);
//    lowerBound.SetSize(1);
//    upperBound.SetSize(1);
//    histogram->SetMeasurementVectorSize(1);

//    // Numver of Bins
//    sz[0] = 256;

//    lowerBound.Fill(statistics->GetMinimum(1));
//    upperBound.Fill(statistics->GetMaximum(1));

//    histogram->Initialize(sz, lowerBound, upperBound);
//    histogram->SetToZero();

//    typename HistogramType::MeasurementVectorType measurement;
//    measurement.SetSize(1); measurement[0] = 0;

//    itk::ImageRegionIterator<TInPixelImageType> b0It(b0Image, b0Image->GetLargestPossibleRegion());
//    while ( !b0It.IsAtEnd() )
//    {
//        if (m_MaskImage->GetPixel(b0It.GetIndex()) > 0)
//        {
//            TInPixelType value = b0It.Value();
//            measurement[0] = value;
//            histogram->IncreaseFrequencyOfMeasurement(measurement, 1);
//        }
//        ++b0It;
//    }

//    // Find bin with max frequency
//    double maxFreq =0 ;
//    unsigned int index = 0;
//    for (unsigned int ii =0 ; ii < sz[0];++ii)
//    {
//        if( maxFreq < histogram->GetFrequency(ii))
//        {
//            maxFreq = histogram->GetFrequency(ii);
//            index = ii;
//        }
//    }

//    typename StatisticsFilterType::Pointer statisticsImageFilter = StatisticsFilterType::New();
//    statisticsImageFilter->SetInput(b0Image);
//    statisticsImageFilter->SetLabelInput(outputBinMaskImage);
//    statisticsImageFilter->Update();
//    double variance = statisticsImageFilter->GetVariance(1);

//    typename ShiftScaleImageFilterType::Pointer shiftScaleImageFilter = ShiftScaleImageFilterType::New();
//      MITK_INFO << "******************* " << histogram->GetBinMax(0,index);
//      m_GlobalMax = histogram->GetBinMax(0,index);
//      m_UseGlobalMax = true;
//    MITK_INFO << "******************* " << maxFreq;
//    shiftScaleImageFilter->SetShift(  0 );//-histogram->GetBinMax(0,index));
//    shiftScaleImageFilter->SetScale( ( (double) 1 )  / std::sqrt(variance));

//    for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
//    {
//        typename TInPixelImageType::Pointer channel = TInPixelImageType::New();
//        channel->SetSpacing( this->GetInput()->GetSpacing() );
//        channel->SetOrigin( this->GetInput()->GetOrigin() );
//        channel->SetDirection( this->GetInput()->GetDirection() );
//        channel->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion() );
//        channel->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
//        channel->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
//        channel->Allocate();

//        ImageRegionIterator<TInPixelImageType> it(channel, channel->GetLargestPossibleRegion());
//        while(!it.IsAtEnd())
//        {
//            typename InputImageType::PixelType pix = inputImagePointer->GetPixel(it.GetIndex());
//            it.Set(pix[i]);
//            ++it;
//        }

//        shiftScaleImageFilter->SetInput( channel );
//        shiftScaleImageFilter->Update();
//        DoubleImageType::Pointer normalized = shiftScaleImageFilter->GetOutput();

//        ImageRegionIterator<DoubleImageType> it2(normalized, normalized->GetLargestPossibleRegion());
//        while(!it2.IsAtEnd())
//        {
//            typename InputImageType::PixelType pix = inputImagePointer->GetPixel(it2.GetIndex());
//            pix[i] = m_NewMax*it2.Get();
//            inputImagePointer->SetPixel(it2.GetIndex(), pix);
//            ++it2;
//        }
//    }

}

template< class TInPixelType >
void DwiNormilzationFilter< TInPixelType>::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
    MITK_INFO << "8";
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
