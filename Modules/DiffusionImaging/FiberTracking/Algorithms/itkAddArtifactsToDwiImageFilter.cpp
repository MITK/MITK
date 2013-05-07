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

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< class TPixelType >
AddArtifactsToDwiImageFilter< TPixelType >
::AddArtifactsToDwiImageFilter()
    : m_NoiseModel(NULL)
    , m_kOffset(0)
    , m_tLine(1)
{
    this->SetNumberOfRequiredInputs( 1 );
}


template< class TPixelType >
typename AddArtifactsToDwiImageFilter< TPixelType >::ComplexSliceType::Pointer AddArtifactsToDwiImageFilter< TPixelType >::RearrangeSlice(typename ComplexSliceType::Pointer slice)
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

    typename DiffusionImageType::Pointer outputImage = DiffusionImageType::New();
    outputImage->SetSpacing( inputImage->GetSpacing() );
    outputImage->SetOrigin( inputImage->GetOrigin() );
    outputImage->SetDirection( inputImage->GetDirection() );
    outputImage->SetLargestPossibleRegion( inputRegion );
    outputImage->SetBufferedRegion( inputRegion );
    outputImage->SetRequestedRegion( inputRegion );
    outputImage->SetVectorLength( inputImage->GetVectorLength() );
    outputImage->Allocate();

    unsigned int upsampling = 1;
    if (m_RingingModel!=NULL)
    {
        upsampling = m_RingingModel->GetKspaceCropping();
    }

    // create slice object
    typename SliceType::Pointer slice = SliceType::New();
    ImageRegion<2> region;
    region.SetSize(0, inputRegion.GetSize(0));
    region.SetSize(1, inputRegion.GetSize(1));
    slice->SetLargestPossibleRegion( region );
    slice->SetBufferedRegion( region );
    slice->SetRequestedRegion( region );
    slice->Allocate();

    // frequency map slice
    typename SliceType::Pointer fMap = NULL;
    if (m_FrequencyMap.IsNotNull())
    {
        fMap = SliceType::New();
        ImageRegion<2> region;
        region.SetSize(0, inputRegion.GetSize(0));
        region.SetSize(1, inputRegion.GetSize(1));
        fMap->SetLargestPossibleRegion( region );
        fMap->SetBufferedRegion( region );
        fMap->SetRequestedRegion( region );
        fMap->Allocate();
    }

    boost::progress_display disp(inputImage->GetVectorLength()*inputRegion.GetSize(2));
    for (int g=0; g<inputImage->GetVectorLength(); g++)
        for (int z=0; z<inputRegion.GetSize(2); z++)
        {
            ++disp;
            // extract slice from channel g
            for (int y=0; y<inputRegion.GetSize(1); y++)
                for (int x=0; x<inputRegion.GetSize(0); x++)
                {
                    typename SliceType::IndexType index2D;
                    index2D[0]=x; index2D[1]=y;
                    typename DiffusionImageType::IndexType index3D;
                    index3D[0]=x; index3D[1]=y; index3D[2]=z;

                    TPixelType pix2D = inputImage->GetPixel(index3D)[g];
                    slice->SetPixel(index2D, pix2D);

                    if (fMap.IsNotNull())
                        fMap->SetPixel(index2D, m_FrequencyMap->GetPixel(index3D));
                }

            // fourier transform slice
            typename ComplexSliceType::Pointer fSlice;
            typename itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
            idft->SetInput(slice);
            idft->SetkOffset(m_kOffset);
            idft->SettLine(m_tLine);
            idft->SetFrequencyMap(fMap);
            idft->Update();

            fSlice = idft->GetOutput();

            if (m_RingingModel!=NULL)
            {
                fSlice = RearrangeSlice(fSlice);
                fSlice = m_RingingModel->AddArtifact(fSlice);
            }

            // inverse fourier transform slice
            typename SliceType::Pointer newSlice;
            typename itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
            dft->SetInput(fSlice);
            dft->Update();
            newSlice = dft->GetOutput();

            // put slice back into channel g
            for (int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
                for (int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
                {
                    typename DiffusionImageType::IndexType index3D;
                    index3D[0]=x; index3D[1]=y; index3D[2]=z;
                    typename DiffusionImageType::PixelType pix3D = outputImage->GetPixel(index3D);
                    typename SliceType::IndexType index2D;
                    index2D[0]=x; index2D[1]=y;

                    pix3D[g] = newSlice->GetPixel(index2D);
                    outputImage->SetPixel(index3D, pix3D);
                }
        }

    if (m_NoiseModel!=NULL)
    {
        ImageRegionIterator<DiffusionImageType> it1 (outputImage, outputImage->GetLargestPossibleRegion());
        boost::progress_display disp2(outputImage->GetLargestPossibleRegion().GetNumberOfPixels());
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
