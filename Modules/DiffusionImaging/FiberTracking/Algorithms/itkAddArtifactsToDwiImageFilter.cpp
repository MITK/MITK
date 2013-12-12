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
#include <itkCropImageFilter.h>

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
    , m_AddGibbsRinging(false)
    , m_Spikes(0)
    , m_SpikeAmplitude(1)
    , m_Wrap(1.0)
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
    m_StartTime = clock();
    m_StatusText = "Starting simulation\n";

    typename DiffusionImageType::Pointer inputImage  = static_cast< DiffusionImageType * >( this->ProcessObject::GetInput(0) );
    itk::ImageRegion<3> inputRegion = inputImage->GetLargestPossibleRegion();

    typename itk::ImageDuplicator<DiffusionImageType>::Pointer duplicator = itk::ImageDuplicator<DiffusionImageType>::New();
    duplicator->SetInputImage( inputImage );
    duplicator->Update();
    typename DiffusionImageType::Pointer outputImage = duplicator->GetOutput();

    // is input slize size even?
    int xMax=inputRegion.GetSize(0); int yMax=inputRegion.GetSize(1);
    if ( xMax%2 == 1 )
        xMax += 1;
    if ( yMax%2 == 1 )
        yMax += 1;

    // create slice object
    typename SliceType::Pointer slice = SliceType::New();
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, xMax);
    sliceRegion.SetSize(1, yMax);
    slice->SetLargestPossibleRegion( sliceRegion );
    slice->SetBufferedRegion( sliceRegion );
    slice->SetRequestedRegion( sliceRegion );
    slice->Allocate();
    slice->FillBuffer(0.0);

    ImageRegion<2> upsampledSliceRegion;
    if (m_AddGibbsRinging)
    {
        upsampledSliceRegion.SetSize(0, xMax*2);
        upsampledSliceRegion.SetSize(1, yMax*2);
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

    if (m_Spikes>0 || m_FrequencyMap.IsNotNull() || m_kOffset>0.0 || m_AddGibbsRinging || m_SimulateEddyCurrents || m_Wrap<1.0)
    {
        ImageRegion<3> croppedRegion = inputRegion; croppedRegion.SetSize(1, croppedRegion.GetSize(1)*m_Wrap);
        itk::Point<double,3> shiftedOrigin = inputImage->GetOrigin(); shiftedOrigin[1] += (inputRegion.GetSize(1)-croppedRegion.GetSize(1))*inputImage->GetSpacing()[1]/2;

        outputImage = DiffusionImageType::New();
        outputImage->SetSpacing( inputImage->GetSpacing() );
        outputImage->SetOrigin( shiftedOrigin );
        outputImage->SetDirection( inputImage->GetDirection() );
        outputImage->SetLargestPossibleRegion( croppedRegion );
        outputImage->SetBufferedRegion( croppedRegion );
        outputImage->SetRequestedRegion( croppedRegion );
        outputImage->SetVectorLength( inputImage->GetVectorLength() );
        outputImage->Allocate();
        typename DiffusionImageType::PixelType temp;
        temp.SetSize(inputImage->GetVectorLength());
        temp.Fill(0.0);
        outputImage->FillBuffer(temp);

        int tempY=croppedRegion.GetSize(1);
        if ( tempY%2 == 1 )
            tempY += 1;
        croppedRegion.SetSize(1, tempY);

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
        if (m_AddGibbsRinging)
            MITK_INFO << "Gibbs ringing enabled";
        else
            MITK_INFO << "Gibbs ringing disabled";

        if (m_SimulateEddyCurrents)
            MITK_INFO << "Simulating eddy currents";

        std::vector< int > spikeVolume;
        for (int i=0; i<m_Spikes; i++)
            spikeVolume.push_back(rand()%inputImage->GetVectorLength());
        std::sort (spikeVolume.begin(), spikeVolume.end());
        std::reverse (spikeVolume.begin(), spikeVolume.end());

        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        unsigned long lastTick = 0;
        boost::progress_display disp(inputImage->GetVectorLength()*inputRegion.GetSize(2));
        for (unsigned int g=0; g<inputImage->GetVectorLength(); g++)
        {
            std::vector< int > spikeSlice;
            while (!spikeVolume.empty() && spikeVolume.back()==g)
            {
                spikeSlice.push_back(rand()%inputImage->GetLargestPossibleRegion().GetSize(2));
                spikeVolume.pop_back();
            }
            std::sort (spikeSlice.begin(), spikeSlice.end());
            std::reverse (spikeSlice.begin(), spikeSlice.end());

            for (unsigned int z=0; z<inputRegion.GetSize(2); z++)
            {
                if (this->GetAbortGenerateData())
                {
                    m_StatusText += "\nSimulation aborted\n";
                    return;
                }

                std::vector< SliceType::Pointer > compartmentSlices;
                // extract slice from channel g
                for (unsigned int y=0; y<sliceRegion.GetSize(1); y++)
                    for (unsigned int x=0; x<sliceRegion.GetSize(0); x++)
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

                if (m_AddGibbsRinging)
                {
                    itk::ResampleImageFilter<SliceType, SliceType>::Pointer resampler = itk::ResampleImageFilter<SliceType, SliceType>::New();
                    resampler->SetInput(slice);
                    resampler->SetOutputParametersFromImage(slice);
                    resampler->SetSize(upsampledSliceRegion.GetSize());
                    resampler->SetOutputSpacing(slice->GetSpacing()/2);
                    resampler->Update();
                    typename SliceType::Pointer upslice = resampler->GetOutput();
                    compartmentSlices.push_back(upslice);
                }
                else
                    compartmentSlices.push_back(slice);

                // fourier transform slice
                typename ComplexSliceType::Pointer fSlice;

                itk::Size<2> outSize; outSize.SetElement(0, xMax); outSize.SetElement(1, croppedRegion.GetSize()[1]);
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
                int numSpikes = 0;
                while (!spikeSlice.empty() && spikeSlice.back()==z)
                {
                    numSpikes++;
                    spikeSlice.pop_back();
                }
                idft->SetSpikes(numSpikes);
                idft->SetSpikeAmplitude(m_SpikeAmplitude);
                idft->Update();
                fSlice = idft->GetOutput();

                // inverse fourier transform slice
                typename SliceType::Pointer newSlice;
                typename itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
                dft->SetInput(fSlice);
                dft->Update();
                newSlice = dft->GetOutput();

                // put slice back into channel g
                for (unsigned int y=0; y<outputImage->GetLargestPossibleRegion().GetSize(1); y++)
                    for (unsigned int x=0; x<outputImage->GetLargestPossibleRegion().GetSize(0); x++)
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
                unsigned long newTick = 50*disp.count()/disp.expected_count();
                for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                    m_StatusText += "*";
                lastTick = newTick;
            }
        }
        m_StatusText += "\n\n";
    }

    if (m_NoiseModel!=NULL)
    {
        m_StatusText += "Adding noise\n";
        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        unsigned long lastTick = 0;

        ImageRegionIterator<DiffusionImageType> it1 (outputImage, outputImage->GetLargestPossibleRegion());
        boost::progress_display disp(outputImage->GetLargestPossibleRegion().GetNumberOfPixels());
        while(!it1.IsAtEnd())
        {
            if (this->GetAbortGenerateData())
            {
                m_StatusText += "\nSimulation aborted\n";
                return;
            }

            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;

            typename DiffusionImageType::PixelType signal = it1.Get();
            m_NoiseModel->AddNoise(signal);
            it1.Set(signal);

            ++it1;
        }
        m_StatusText += "\n\n";
    }

    this->SetNthOutput(0, outputImage);
    m_StatusText += "Finished simulation\n";
    m_StatusText += "Simulation time: "+GetTime();
}

template< class TPixelType >
std::string AddArtifactsToDwiImageFilter< TPixelType >::GetTime()
{
    unsigned long total = (double)(clock() - m_StartTime)/CLOCKS_PER_SEC;
    unsigned long hours = total/3600;
    unsigned long minutes = (total%3600)/60;
    unsigned long seconds = total%60;
    std::string out = "";
    out.append(boost::lexical_cast<std::string>(hours));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(minutes));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(seconds));
    return out;
}

}
#endif
