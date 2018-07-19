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
#ifndef __itkVectorImageToFourDImageFilter_cpp
#define __itkVectorImageToFourDImageFilter_cpp

#include "itkVectorImageToFourDImageFilter.h"
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
VectorImageToFourDImageFilter< PixelType>::VectorImageToFourDImageFilter()
{

}


template< class PixelType >
void VectorImageToFourDImageFilter< PixelType>
::GenerateData()
{
  Vector<float, 4> spacing4;
  Point<float, 4> origin4;
  Matrix<double, 4, 4> direction4; direction4.SetIdentity();
  ImageRegion<4> imageRegion4;

  Vector<double, 3> spacing3 = m_InputImage->GetSpacing();
  Point<float, 3> origin3 = m_InputImage->GetOrigin();
  Matrix<double, 3, 3> direction3 = m_InputImage->GetDirection();
  ImageRegion<3> imageRegion3 = m_InputImage->GetLargestPossibleRegion();

  spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
  origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin4[3] = 0;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction4[r][c] = direction3[r][c];

  imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
  imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
  imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
  imageRegion4.SetSize(3, m_InputImage->GetVectorLength());

  m_OutputImage = OutputImageType::New();
  m_OutputImage->SetSpacing( spacing4 );
  m_OutputImage->SetOrigin( origin4 );
  m_OutputImage->SetDirection( direction4 );
  m_OutputImage->SetRegions( imageRegion4 );
  m_OutputImage->Allocate();
  m_OutputImage->FillBuffer(0);
  ImageRegionIterator< InputImageType > iit(m_InputImage, m_InputImage->GetLargestPossibleRegion() );

  while( !iit.IsAtEnd() )
  {
    auto idx3 = iit.GetIndex();

    typename InputImageType::PixelType pix = iit.Get();

    for (unsigned int i=0; i<pix.GetSize(); ++i)
    {
      typename OutputImageType::IndexType idx4;
      idx4[0] = idx3[0]; idx4[1] = idx3[1]; idx4[2] = idx3[2]; idx4[3] = i;
      m_OutputImage->SetPixel(idx4, pix[i]);
    }

    ++iit;
  }
}

}

#endif // __itkVectorImageToFourDImageFilter_cpp
