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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkResampleDwiImageFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ResampleDwiImageFilter_txx_
#define _itk_ResampleDwiImageFilter_txx_
#endif

#define _USE_MATH_DEFINES

#include "itkResampleDwiImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegion.h>
#include <itkResampleImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

namespace itk
{

template <class TScalarType>
ResampleDwiImageFilter<TScalarType>
::ResampleDwiImageFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template <class TScalarType>
void
ResampleDwiImageFilter<TScalarType>
::GenerateData()
{
    // initialize output image
    itk::Vector< double, 3 > spacing = this->GetInput()->GetSpacing();
    spacing[0] /= m_SamplingFactor[0];
    spacing[1] /= m_SamplingFactor[1];
    spacing[2] /= m_SamplingFactor[2];
    ImageRegion<3> region = this->GetInput()->GetLargestPossibleRegion();
    region.SetSize(0, region.GetSize(0)*m_SamplingFactor[0]);
    region.SetSize(1, region.GetSize(1)*m_SamplingFactor[1]);
    region.SetSize(2, region.GetSize(2)*m_SamplingFactor[2]);

    typename DwiImageType::Pointer outImage = DwiImageType::New();
    outImage->SetSpacing( spacing );
    outImage->SetOrigin( this->GetInput()->GetOrigin() );
    outImage->SetDirection( this->GetInput()->GetDirection() );
    outImage->SetLargestPossibleRegion( region );
    outImage->SetBufferedRegion( region );
    outImage->SetRequestedRegion( region );
    outImage->SetVectorLength( this->GetInput()->GetVectorLength() );
    outImage->Allocate();

    typename itk::NearestNeighborInterpolateImageFunction<DwiChannelType>::Pointer interp = itk::NearestNeighborInterpolateImageFunction<DwiChannelType>::New();
//    typename itk::BSplineInterpolateImageFunction<DwiChannelType>::Pointer interp = itk::BSplineInterpolateImageFunction<DwiChannelType>::New();

    typename itk::ResampleImageFilter<DwiChannelType, DwiChannelType>::Pointer resampler = itk::ResampleImageFilter<DwiChannelType, DwiChannelType>::New();
    resampler->SetOutputParametersFromImage(outImage);
    resampler->SetInterpolator(interp);

    for (int i=0; i<this->GetInput()->GetVectorLength(); i++)
    {
        typename DwiChannelType::Pointer channel = DwiChannelType::New();
        channel->SetSpacing( this->GetInput()->GetSpacing() );
        channel->SetOrigin( this->GetInput()->GetOrigin() );
        channel->SetDirection( this->GetInput()->GetDirection() );
        channel->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion() );
        channel->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
        channel->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
        channel->Allocate();

        ImageRegionIterator<DwiChannelType> it(channel, channel->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            typename DwiImageType::PixelType pix = this->GetInput()->GetPixel(it.GetIndex());
            it.Set(pix[i]);
            ++it;
        }

        resampler->SetInput(channel);
        resampler->Update();
        channel = resampler->GetOutput();

        ImageRegionIterator<DwiImageType> it2(outImage, outImage->GetLargestPossibleRegion());
        while(!it2.IsAtEnd())
        {
            typename DwiImageType::PixelType pix = it2.Get();
            pix[i] = channel->GetPixel(it2.GetIndex());
            it2.Set(pix);
            ++it2;
        }
    }

    this->SetNthOutput(0, outImage);
}



} // end of namespace
