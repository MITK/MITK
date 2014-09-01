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
    : m_UseConstantRandSeed(false)
{
    this->SetNumberOfRequiredInputs( 1 );

    m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();
}

template< class TPixelType >
void AddArtifactsToDwiImageFilter< TPixelType >
::GenerateData()
{
    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        m_RandGen->SetSeed(0);
    else
        m_RandGen->SetSeed();

    m_StartTime = clock();
    m_StatusText = "Starting simulation\n";

    typename InputImageType::Pointer inputImage  = static_cast< InputImageType* >( this->ProcessObject::GetInput(0) );
    itk::ImageRegion<3> inputRegion = inputImage->GetLargestPossibleRegion();

    typename itk::ImageDuplicator<InputImageType>::Pointer duplicator = itk::ImageDuplicator<InputImageType>::New();
    duplicator->SetInputImage( inputImage );
    duplicator->Update();
    typename InputImageType::Pointer outputImage = duplicator->GetOutput();

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
    if ( m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
    {
        upsampledSliceRegion.SetSize(0, xMax*2);
        upsampledSliceRegion.SetSize(1, yMax*2);
    }

    // frequency map slice
    typename SliceType::Pointer fMapSlice = NULL;
    if ( m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
    {
        fMapSlice = SliceType::New();
        fMapSlice->SetLargestPossibleRegion( sliceRegion );
        fMapSlice->SetBufferedRegion( sliceRegion );
        fMapSlice->SetRequestedRegion( sliceRegion );
        fMapSlice->Allocate();
        fMapSlice->FillBuffer(0.0);
    }

    m_Parameters.m_SignalGen.m_SignalScale = 1;
    m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;

    if ( m_Parameters.m_SignalGen.m_Spikes>0 ||  m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull() ||  m_Parameters.m_SignalGen.m_KspaceLineOffset>0.0 ||  m_Parameters.m_SignalGen.m_DoAddGibbsRinging ||  m_Parameters.m_SignalGen.m_EddyStrength>0 ||  m_Parameters.m_SignalGen.m_CroppingFactor<1.0)
    {
        ImageRegion<3> croppedRegion = inputRegion; croppedRegion.SetSize(1, croppedRegion.GetSize(1)* m_Parameters.m_SignalGen.m_CroppingFactor);
        itk::Point<double,3> shiftedOrigin = inputImage->GetOrigin(); shiftedOrigin[1] += (inputRegion.GetSize(1)-croppedRegion.GetSize(1))*inputImage->GetSpacing()[1]/2;

        outputImage = InputImageType::New();
        outputImage->SetSpacing( inputImage->GetSpacing() );
        outputImage->SetOrigin( shiftedOrigin );
        outputImage->SetDirection( inputImage->GetDirection() );
        outputImage->SetLargestPossibleRegion( croppedRegion );
        outputImage->SetBufferedRegion( croppedRegion );
        outputImage->SetRequestedRegion( croppedRegion );
        outputImage->SetVectorLength( inputImage->GetVectorLength() );
        outputImage->Allocate();
        typename InputImageType::PixelType temp;
        temp.SetSize(inputImage->GetVectorLength());
        temp.Fill(0.0);
        outputImage->FillBuffer(temp);

        int tempY=croppedRegion.GetSize(1);
        tempY += tempY%2;
        croppedRegion.SetSize(1, tempY);

        m_StatusText += this->GetTime()+" > Adjusting complex signal\n";
        if ( m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
            m_StatusText += "Simulating distortions\n";
        if ( m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
            m_StatusText += "Simulating ringing artifacts\n";
        if ( m_Parameters.m_SignalGen.m_EddyStrength>0)
            m_StatusText += "Simulating eddy currents\n";
        if ( m_Parameters.m_SignalGen.m_Spikes>0)
            m_StatusText += "Simulating spikes\n";
        if ( m_Parameters.m_SignalGen.m_CroppingFactor<1.0)
            m_StatusText += "Simulating aliasing artifacts\n";
        if ( m_Parameters.m_SignalGen.m_KspaceLineOffset>0)
            m_StatusText += "Simulating ghosts\n";

        std::vector< unsigned int > spikeVolume;
        for (unsigned int i=0; i< m_Parameters.m_SignalGen.m_Spikes; i++)
            spikeVolume.push_back(m_RandGen->GetIntegerVariate()%inputImage->GetVectorLength());
        std::sort (spikeVolume.begin(), spikeVolume.end());
        std::reverse (spikeVolume.begin(), spikeVolume.end());
        FiberfoxParameters<double> doubleParam =  m_Parameters.CopyParameters<double>();

        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        unsigned long lastTick = 0;
        boost::progress_display disp(inputImage->GetVectorLength()*inputRegion.GetSize(2));
        for (unsigned int g=0; g<inputImage->GetVectorLength(); g++)
        {
            std::vector< unsigned int > spikeSlice;
            while (!spikeVolume.empty() && spikeVolume.back()==g)
            {
                spikeSlice.push_back(m_RandGen->GetIntegerVariate()%inputImage->GetLargestPossibleRegion().GetSize(2));
                spikeVolume.pop_back();
            }
            std::sort (spikeSlice.begin(), spikeSlice.end());
            std::reverse (spikeSlice.begin(), spikeSlice.end());

            for (unsigned int z=0; z<inputRegion.GetSize(2); z++)
            {
                if (this->GetAbortGenerateData())
                {
                    m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
                    return;
                }

                std::vector< SliceType::Pointer > compartmentSlices;
                // extract slice from channel g
                for (unsigned int y=0; y<inputRegion.GetSize(1); y++)
                    for (unsigned int x=0; x<inputRegion.GetSize(0); x++)
                    {
                        typename SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;
                        typename InputImageType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        SliceType::PixelType pix2D = (SliceType::PixelType)inputImage->GetPixel(index3D)[g];
                        slice->SetPixel(index2D, pix2D);
                        if (fMapSlice.IsNotNull())
                            fMapSlice->SetPixel(index2D,  m_Parameters.m_SignalGen.m_FrequencyMap->GetPixel(index3D));
                    }

                if ( m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
                {
                    itk::ResampleImageFilter<SliceType, SliceType>::Pointer resampler = itk::ResampleImageFilter<SliceType, SliceType>::New();
                    resampler->SetInput(slice);
                    resampler->SetOutputParametersFromImage(slice);
                    resampler->SetSize(upsampledSliceRegion.GetSize());
                    resampler->SetOutputSpacing(slice->GetSpacing()/2);
                    itk::NearestNeighborInterpolateImageFunction<SliceType, double>::Pointer nn_interpolator
                            = itk::NearestNeighborInterpolateImageFunction<SliceType, double>::New();
                    resampler->SetInterpolator(nn_interpolator);
                    resampler->Update();
                    typename SliceType::Pointer upslice = resampler->GetOutput();
                    compartmentSlices.push_back(upslice);

                    if (fMapSlice.IsNotNull())
                    {
                        itk::ResampleImageFilter<SliceType, SliceType>::Pointer resampler = itk::ResampleImageFilter<SliceType, SliceType>::New();
                        resampler->SetInput(fMapSlice);
                        resampler->SetOutputParametersFromImage(fMapSlice);
                        resampler->SetSize(upsampledSliceRegion.GetSize());
                        resampler->SetOutputSpacing(fMapSlice->GetSpacing()/2);
                        itk::NearestNeighborInterpolateImageFunction<SliceType, double>::Pointer nn_interpolator
                                = itk::NearestNeighborInterpolateImageFunction<SliceType, double>::New();
                        resampler->SetInterpolator(nn_interpolator);
                        resampler->Update();
                        fMapSlice = resampler->GetOutput();
                    }
                }
                else
                    compartmentSlices.push_back(slice);

                // fourier transform slice
                typename ComplexSliceType::Pointer fSlice;

                itk::Size<2> outSize; outSize.SetElement(0, xMax); outSize.SetElement(1, croppedRegion.GetSize()[1]);
                typename itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
                idft->SetUseConstantRandSeed(m_UseConstantRandSeed);
                idft->SetParameters(doubleParam);
                idft->SetCompartmentImages(compartmentSlices);
                idft->SetFrequencyMapSlice(fMapSlice);
                idft->SetDiffusionGradientDirection( m_Parameters.m_SignalGen.GetGradientDirection(g));
                idft->SetZ((double)z-(double)inputRegion.GetSize(2)/2.0);
                idft->SetOutSize(outSize);
                int numSpikes = 0;
                while (!spikeSlice.empty() && spikeSlice.back()==z)
                {
                    numSpikes++;
                    spikeSlice.pop_back();
                }
                idft->SetSpikesPerSlice(numSpikes);
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
                        typename InputImageType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        typename InputImageType::PixelType pix3D = outputImage->GetPixel(index3D);
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

    if ( m_Parameters.m_NoiseModel!=NULL)
    {
        m_StatusText += this->GetTime()+" > Adding noise\n";
        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        unsigned long lastTick = 0;

        ImageRegionIterator<InputImageType> it1 (outputImage, outputImage->GetLargestPossibleRegion());
        boost::progress_display disp(outputImage->GetLargestPossibleRegion().GetNumberOfPixels());
        while(!it1.IsAtEnd())
        {
            if (this->GetAbortGenerateData())
            {
                m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
                return;
            }

            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;

            typename InputImageType::PixelType signal = it1.Get();
            m_Parameters.m_NoiseModel->AddNoise(signal);
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
