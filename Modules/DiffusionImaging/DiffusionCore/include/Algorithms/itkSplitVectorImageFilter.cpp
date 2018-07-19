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
#ifndef __itkSplitVectorImageFilter_cpp
#define __itkSplitVectorImageFilter_cpp

#include "itkSplitVectorImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <vnl/vnl_vector.h>
#include <itkOrientationDistributionFunction.h>
#include <itkContinuousIndex.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

#include <boost/math/special_functions.hpp>
#include <boost/progress.hpp>


using namespace boost::math;

namespace itk {

template< class PixelType >
SplitVectorImageFilter< PixelType>::SplitVectorImageFilter()
{

}


template< class PixelType >
void SplitVectorImageFilter< PixelType>
::GenerateData()
{
  m_OutputImages.clear();
  for (unsigned int i=0; i<m_InputImage->GetVectorLength(); ++i)
  {
    typename OutputImageType::Pointer m_OutputImage = OutputImageType::New();
    m_OutputImage->SetSpacing( m_InputImage->GetSpacing() );
    m_OutputImage->SetOrigin( m_InputImage->GetOrigin() );
    m_OutputImage->SetDirection( m_InputImage->GetDirection() );
    m_OutputImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
    m_OutputImage->Allocate();
    m_OutputImage->FillBuffer(0);
    m_OutputImages.push_back(m_OutputImage);
  }

  ImageRegionIterator< InputImageType > iit(m_InputImage, m_InputImage->GetLargestPossibleRegion() );

  while( !iit.IsAtEnd() )
  {
    auto idx = iit.GetIndex();
    typename InputImageType::PixelType pix = iit.Get();
    for (unsigned int i=0; i<pix.GetSize(); ++i)
      m_OutputImages.at(i)->SetPixel(idx, pix[i]);
    ++iit;
  }
}

}

#endif // __itkSplitVectorImageFilter_cpp
