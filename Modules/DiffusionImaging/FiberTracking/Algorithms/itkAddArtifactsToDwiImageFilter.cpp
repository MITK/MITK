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

#ifndef __itkAddArtifactsToDwiImageFilter_txx
#define __itkAddArtifactsToDwiImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkAddArtifactsToDwiImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <boost/progress.hpp>
#include <itkImageDuplicator.h>
#include <itkResampleDwiImageFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< class TPixelType >
AddArtifactsToDwiImageFilter< TPixelType >
::AddArtifactsToDwiImageFilter()
    : m_NoiseModel(NULL)
    , m_FrequencyMap(NULL)
    , m_kOffset(0)
    , m_tLine(1)
    , m_EddyGradientStrength(0.001)
    , m_SimulateEddyCurrents(false)
    , m_TE(100)
    , m_Upsampling(1)
{
    this->SetNumberOfRequiredInputs( 1 );
}


template< class TPixelType >
AddArtifactsToDwiImageFilter< TPixelType >::ComplexSliceType::Pointer AddArtifactsToDwiImageFilter< TPixelType >::RearrangeSlice(ComplexSliceType::Pointer slice)
{
    ImageRegion<2> region = slice->GetLargestPossibleRegion();
    typename ComplexSliceType::Pointer rearrangedSlice = ComplexSliceType::New();
    rearrangedSlice->SetLargestPossibleRegion( region );
    rearrangedSlice->SetBufferedRegion( region );
    rearrangedSlice->SetRequestedRegion( region );
    rearrangedSlice->Allocate();

    int xHalf = region.GetSize(0)/2;
    int yHalf = region.GetSize(1)/2;

    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            typename SliceType::IndexType idx;
            idx[0]=x; idx[1]=y;
            vcl_complex< SliceType::PixelType > pix = slice->GetPixel(idx);

            if( idx[0] <  xHalf )
                idx[0] = idx[0] + xHalf;
            else
                idx[0] = idx[0] - xHalf;

            if( idx[1] <  yHalf )
                idx[1] = idx[1] + yHalf;
            else
                idx[1] = idx[1] - yHalf;

            rearrangedSlice->SetPixel(idx, pix);
        }

    return rearrangedSlice;
}

template< class TPixelType >
void AddArtifactsToDwiImageFilter< TPixelType >
::GenerateData()
{
    typename DiffusionImageType::Pointer inputImage  = static_cast< DiffusionImageType * >( this->ProcessObject::GetInput(0) );
    itk::ImageRegion<3> inputRegion = inputImage->GetLargestPossibleRegion();

    typename itk::ImageDuplicator<DiffusionImageType>::Pointer duplicator = itk::ImageDuplicator<DiffusionImageType>::New();
    duplicator->SetInputImage( inputImage );
    duplicator->Update();
    typename DiffusionImageType::Pointer outputImage = duplicator->GetOutput();

    // is input slize size even?
    int x=inputRegion.GetSize(0); int y=inputRegion.GetSize(1);
    if ( x%2 == 1 )
        x += 1;
    if ( y%2 == 1 )
        y += 1;

    // create slice object
    typename SliceType::Pointer slice = SliceType::New();
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, x);
    sliceRegion.SetSize(1, y);
    slice->SetLargestPossibleRegion( sliceRegion );
    slice->SetBufferedRegion( sliceRegion );
    slice->SetRequestedRegion( sliceRegion );
    slice->Allocate();
    slice->FillBuffer(0.0);

    ImageRegion<2> upsampledSliceRegion;
    if (m_Upsampling>1.00001)
    {
        upsampledSliceRegion.SetSize(0, x*m_Upsampling);
        upsampledSliceRegion.SetSize(1, y*m_Upsampling);
    }

    // frequency map slice
    typename SliceType::Pointer fMap = NULL;
    if (m_FrequencyMap.IsNotNull())
    {
        fMap = SliceType::New();
        fMap->SetLargestPossibleRegion( sliceRegion );
        fMap->SetBufferedRegion( sliceRegion );
        fMap->SetRequestedRegion( sliceRegion );
        fMap->Allocate();
        fMap->FillBuffer(0.0);
    }

    if ( m_FrequencyMap.IsNotNull() || m_kOffset>0.0 || m_Upsampling>1.00001 || m_SimulateEddyCurrents)
    {
        MatrixType transform = inputImage->GetDirection();
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                    transform[i][j] *= inputImage->GetSpacing()[j];

        MITK_INFO << "Adjusting complex signal";
        MITK_INFO << "line readout time: " << m_tLine;
        MITK_INFO << "line offset: " << m_kOffset;
        if (m_FrequencyMap.IsNotNull())
            MITK_INFO << "frequency map is set";
        else
            MITK_INFO << "no frequency map set";
        if (m_Upsampling>1.00001)
            MITK_INFO << "Gibbs ringing enabled";
        else
            MITK_INFO << "Gibbs ringing disabled";

        if (m_SimulateEddyCurrents)
            MITK_INFO << "Simulating eddy currents";

        boost::progress_display disp(inputImage->GetVectorLength()*inputRegion.GetSize(2));
        for (int g=0; g<inputImage->GetVectorLength(); g++)
            for (int z=0; z<inputRegion.GetSize(2); z++)
            {
                std::vector< SliceType::Pointer > compartmentSlices;
                // extract slice from channel g
                for (int y=0; y<sliceRegion.GetSize(1); y++)
                    for (int x=0; x<sliceRegion.GetSize(0); x++)
                    {
                        typename SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;
                        typename DiffusionImageType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        SliceType::PixelType pix2D = (SliceType::PixelType)inputImage->GetPixel(index3D)[g];
                        slice->SetPixel(index2D, pix2D);

                        if (fMap.IsNotNull())
                            fMap->SetPixel(index2D, m_FrequencyMap->GetPixel(index3D));
                    }

                if (m_Upsampling>1.00001)
                {
                    itk::ResampleImageFilter<SliceType, SliceType>::Pointer resampler = itk::ResampleImageFilter<SliceType, SliceType>::New();
                    resampler->SetInput(slice);
                    resampler->SetOutputParametersFromImage(slice);
                    resampler->SetSize(upsampledSliceRegion.GetSize());
                    resampler->SetOutputSpacing(slice->GetSpacing()/m_Upsampling);
                    resampler->Update();
                    typename SliceType::Pointer upslice = resampler->GetOutput();
                    compartmentSlices.push_back(upslice);
                }
                else
                    compartmentSlices.push_back(slice);

                // fourier transform slice
                typename ComplexSliceType::Pointer fSlice;

                itk::Size<2> outSize; outSize.SetElement(0, x); outSize.SetElement(1, y);
                typename itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
                idft->SetCompartmentImages(compartmentSlices);
                idft->SetkOffset(m_kOffset);
                idft->SettLine(m_tLine);
                idft->SetSimulateRelaxation(false);
                idft->SetFrequencyMap(fMap);
                idft->SetDiffusionGradientDirection(m_GradientList.at(g));
                idft->SetSimulateEddyCurrents(m_SimulateEddyCurrents);
                idft->SetEddyGradientMagnitude(m_EddyGradientStrength);
                idft->SetTE(m_TE);
                idft->SetZ((double)z-(double)inputRegion.GetSize(2)/2.0);
                idft->SetDirectionMatrix(transform);
                idft->SetOutSize(outSize);
                idft->Update();
                fSlice = idft->GetOutput();

                // inverse fourier transform slice
                typename SliceType::Pointer newSlice;
                typename itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
                dft->SetInput(fSlice);
                dft->Update();
                newSlice = dft->GetOutput();

                // put slice back into channel g
                for (int y=0; y<inputRegion.GetSize(1); y++)
                    for (int x=0; x<inputRegion.GetSize(0); x++)
                    {
                        typename DiffusionImageType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        typename DiffusionImageType::PixelType pix3D = outputImage->GetPixel(index3D);
                        typename SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;

                        double signal = newSlice->GetPixel(index2D);
                        if (signal>0)
                            signal = floor(signal+0.5);
                        else
                            signal = ceil(signal-0.5);

                        pix3D[g] = signal;
                        outputImage->SetPixel(index3D, pix3D);
                    }

                ++disp;
            }
    }

    if (m_NoiseModel!=NULL)
    {
        ImageRegionIterator<DiffusionImageType> it1 (outputImage, inputRegion);
        boost::progress_display disp2(inputRegion.GetNumberOfPixels());
        while(!it1.IsAtEnd())
        {
            ++disp2;

            typename DiffusionImageType::PixelType signal = it1.Get();
            m_NoiseModel->AddNoise(signal);
            it1.Set(signal);

            ++it1;
        }
    }

    this->SetNthOutput(0, outputImage);
}

}
#endif
