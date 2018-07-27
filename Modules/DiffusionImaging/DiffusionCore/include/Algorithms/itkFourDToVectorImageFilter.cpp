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
#ifndef __itkFourDToVectorImageFilter_cpp
#define __itkFourDToVectorImageFilter_cpp

#include "itkFourDToVectorImageFilter.h"
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
FourDToVectorImageFilter< PixelType>::FourDToVectorImageFilter()
{

}


template< class PixelType >
void FourDToVectorImageFilter< PixelType>
::GenerateData()
{
  Vector<float, 4> spacing4 = m_InputImage->GetSpacing();
  Point<float, 4> origin4 = m_InputImage->GetOrigin();
  Matrix<double, 4, 4> direction4 = m_InputImage->GetDirection();
  ImageRegion<4> imageRegion4 = m_InputImage->GetLargestPossibleRegion();

  Vector<double, 3> spacing3;
  Point<float, 3> origin3;
  Matrix<double, 3, 3> direction3;
  ImageRegion<3> imageRegion3;

  spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
  origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction3[r][c] = direction4[r][c];
  imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
  imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
  imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

  m_OutputImage = OutputImageType::New();
  m_OutputImage->SetSpacing( spacing3 );
  m_OutputImage->SetOrigin( origin3 );
  m_OutputImage->SetDirection( direction3 );
  m_OutputImage->SetRegions( imageRegion3 );
  m_OutputImage->SetVectorLength(imageRegion4.GetSize()[3]);
  m_OutputImage->Allocate();
  typename OutputImageType::PixelType pix; pix.SetSize(imageRegion4.GetSize()[3]); pix.Fill(0.0);
  m_OutputImage->FillBuffer(pix);
  ImageRegionIterator< OutputImageType > oit(m_OutputImage, m_OutputImage->GetLargestPossibleRegion() );

  while( !oit.IsAtEnd() )
  {
    auto idx3 = oit.GetIndex();

    typename OutputImageType::PixelType pix = oit.Get();
    for (unsigned int i=0; i<pix.GetSize(); ++i)
    {
      typename InputImageType::IndexType idx4;
      idx4[0] = idx3[0]; idx4[1] = idx3[1]; idx4[2] = idx3[2]; idx4[3] = i;
      pix[i] = m_InputImage->GetPixel(idx4);
    }
    oit.Set(pix);

    ++oit;
  }
}

}

#endif // __itkFourDToVectorImageFilter_cpp
