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
#ifndef __itkFlipPeaksFilter_cpp
#define __itkFlipPeaksFilter_cpp

#include "itkFlipPeaksFilter.h"
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
FlipPeaksFilter< PixelType>::FlipPeaksFilter()
{
  this->SetNumberOfRequiredInputs(1);
}


template< class PixelType >
void FlipPeaksFilter< PixelType>
::BeforeThreadedGenerateData()
{
  typename InputImageType::Pointer input = static_cast< InputImageType* >( this->ProcessObject::GetInput(0) );

//  typename OutputImageType::Pointer output = OutputImageType::New();
//  output->SetSpacing( input->GetSpacing() );
//  output->SetOrigin( input->GetOrigin() );
//  output->SetDirection( input->GetDirection() );
//  output->SetRegions( input->GetLargestPossibleRegion() );
//  output->Allocate();
//  output->FillBuffer(0.0);
}

template< class PixelType >
void FlipPeaksFilter< PixelType>
::AfterThreadedGenerateData()
{

}

template< class PixelType >
void FlipPeaksFilter< PixelType>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  typename InputImageType::Pointer input_image = static_cast< InputImageType* >( this->ProcessObject::GetInput(0) );
  typename OutputImageType::Pointer output_image = static_cast< OutputImageType* >( this->ProcessObject::GetOutput(0) );

  ImageRegionIterator< InputImageType > iit(input_image, outputRegionForThread );
  ImageRegionIterator< OutputImageType > oit(output_image, outputRegionForThread );

  while( !iit.IsAtEnd() )
  {
    typename InputImageType::IndexType idx = iit.GetIndex();

    oit.Set(iit.Get());
    if (m_FlipX && idx[3]%3==0)
      oit.Set(-1*iit.Get());
    if (m_FlipY && idx[3]%3==1)
      oit.Set(-1*iit.Get());
    if (m_FlipZ && idx[3]%3==2)
      oit.Set(-1*iit.Get());

    ++iit;
    ++oit;
  }
}

}

#endif // __itkFlipPeaksFilter_cpp
