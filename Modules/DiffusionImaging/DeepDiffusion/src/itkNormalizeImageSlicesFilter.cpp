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

#ifndef __itkNormalizeImageSlicesFilter_txx
#define __itkNormalizeImageSlicesFilter_txx

#include "itkNormalizeImageSlicesFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

#include <omp.h>
#include <math.h>

namespace itk {

template< class TPixelType >
NormalizeImageSlicesFilter< TPixelType >
::NormalizeImageSlicesFilter()
  : m_Axis(0)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void NormalizeImageSlicesFilter< TPixelType >
::GenerateData()
{
  typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  typename OutputImageType::Pointer outputImage = OutputImageType::New();
  outputImage->SetSpacing( inputImage->GetSpacing() );
  outputImage->SetOrigin( inputImage->GetOrigin() );
  outputImage->SetDirection( inputImage->GetDirection() );
  outputImage->SetRegions( inputImage->GetLargestPossibleRegion() );
  outputImage->Allocate();
  this->SetNthOutput(0, outputImage);

  typename InputImageType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();

  int slices = size[m_Axis];
  int ax = (m_Axis+1)%3;
  int ay = (m_Axis+2)%3;
  int sx = size[ax];
  int sy = size[ay];

#pragma omp parallel for
  for (int i=0; i<slices; i++)
  {
    typename InputImageType::IndexType idx; idx[m_Axis] = i;
    float mean = 0;
    float std = 0;

    for (int x=0; x<sx; x++)
      for (int y=0; y<sy; y++)
      {
        idx[ax] = x;
        idx[ay] = y;
        mean += inputImage->GetPixel(idx);
      }
    mean /= sx*sy;

    for (int x=0; x<sx; x++)
      for (int y=0; y<sy; y++)
      {
        idx[ax] = x;
        idx[ay] = y;
        float diff = mean - inputImage->GetPixel(idx);

        diff *= diff;
        std += diff;
      }
    std = sqrt( std/(sx*sy) );
    if (std<0.0001)
      std = 1;

    for (int x=0; x<sx; x++)
      for (int y=0; y<sy; y++)
      {
        idx[ax] = x;
        idx[ay] = y;
        float val = inputImage->GetPixel(idx);
        val -= mean;
        val /= std;
        outputImage->SetPixel(idx, val);
      }
  }
}

}
#endif
