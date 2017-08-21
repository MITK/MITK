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
    : m_B0Index(-1)
    , m_NewMean(1000)
    , m_NewStdev(500)
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

    if (m_MaskImage.IsNull())
    {
      // initialize mask image
      m_MaskImage = UcharImageType::New();
      m_MaskImage->SetSpacing( inputImagePointer->GetSpacing() );
      m_MaskImage->SetOrigin( inputImagePointer->GetOrigin() );
      m_MaskImage->SetDirection( inputImagePointer->GetDirection() );
      m_MaskImage->SetRegions( inputImagePointer->GetLargestPossibleRegion() );
      m_MaskImage->Allocate();
      m_MaskImage->FillBuffer(1);
    }
    else
      std::cout << "DwiNormilzationFilter: using mask image" << std::endl;

    InputIteratorType git( inputImagePointer, inputImagePointer->GetLargestPossibleRegion() );
    MaskIteratorType mit( m_MaskImage, m_MaskImage->GetLargestPossibleRegion() );
    git.GoToBegin();
    mit.GoToBegin();

    int n = 0;
    m_Mean = 0;
    m_Stdev = 0;
    while( !git.IsAtEnd() )
    {
      if (mit.Get()>0)
      {
        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
        {
          if ((int)i==m_B0Index)
            continue;
          m_Mean += git.Get()[i];
          n++;
        }
      }
      ++git;
      ++mit;
    }
    m_Mean /= n;

    git.GoToBegin();
    mit.GoToBegin();

    while( !git.IsAtEnd() )
    {
      if (mit.Get()>0)
      {
        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
        {
          if ((int)i==m_B0Index)
            continue;
          double diff = (double)(git.Get()[i]) - m_Mean;
          m_Stdev += diff*diff;
        }
      }
      ++git;
      ++mit;
    }

    m_Stdev = std::sqrt(m_Stdev/(n-1));

    MITK_INFO << "Mean: " << m_Mean;
    MITK_INFO << "Stdev: " << m_Stdev;
}

template< class TInPixelType >
void DwiNormilzationFilter< TInPixelType>::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    MaskIteratorType mit( m_MaskImage, outputRegionForThread );
    mit.GoToBegin();
    InputIteratorType git( inputImagePointer, outputRegionForThread );
    git.GoToBegin();

    while( !git.IsAtEnd() )
    {
        typename InputImageType::PixelType pix = git.Get();
        typename OutputImageType::PixelType outPix;
        outPix.SetSize(inputImagePointer->GetVectorLength());

        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
        {
            double val = (double)pix[i] - m_Mean;
            val /= m_Stdev;
            val *= m_NewStdev;
            val += m_NewMean;
            if (val<0)
            {
              val = 0;
              MITK_INFO << "Negative value.";
            }
            if (val>32767)
            {
              val = 32767;
              MITK_INFO << "Range overflow. Value is too large for datatype short.";
            }
            outPix[i] = (TInPixelType)val;
        }

        oit.Set(outPix);

        ++mit;
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
