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
#include "itkExtractChannelFromRgbaImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

// misc
#include <math.h>

namespace itk{

  template< class ReferenceImageType, class OutputImageType >
  ExtractChannelFromRgbaImageFilter< ReferenceImageType, OutputImageType >::ExtractChannelFromRgbaImageFilter():
    m_Channel(RED)
  {

  }

  template< class ReferenceImageType, class OutputImageType >
  ExtractChannelFromRgbaImageFilter< ReferenceImageType, OutputImageType >::~ExtractChannelFromRgbaImageFilter()
  {
  }

  template< class ReferenceImageType, class OutputImageType >
  void ExtractChannelFromRgbaImageFilter< ReferenceImageType, OutputImageType >::GenerateData()
  {

    typename InputImageType::Pointer rgbaImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    typename InputImageType::RegionType region = rgbaImage->GetLargestPossibleRegion();
    outputImage->SetSpacing( m_ReferenceImage->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( m_ReferenceImage->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( m_ReferenceImage->GetDirection() );  // Set the image direction
    outputImage->SetRegions( m_ReferenceImage->GetLargestPossibleRegion());
    outputImage->Allocate();
    outputImage->FillBuffer(0);
    float* outImageBufferPointer = outputImage->GetBufferPointer();

    itk::Image< short, 3 >::Pointer counterImage = itk::Image< short, 3 >::New();
    counterImage->SetSpacing( m_ReferenceImage->GetSpacing() );   // Set the image spacing
    counterImage->SetOrigin( m_ReferenceImage->GetOrigin() );     // Set the image origin
    counterImage->SetDirection( m_ReferenceImage->GetDirection() );  // Set the image direction
    counterImage->SetRegions( m_ReferenceImage->GetLargestPossibleRegion());
    counterImage->Allocate();
    counterImage->FillBuffer(0);
    short* counterImageBufferPointer = counterImage->GetBufferPointer();

    int w = m_ReferenceImage->GetLargestPossibleRegion().GetSize().GetElement(0);
    int h = m_ReferenceImage->GetLargestPossibleRegion().GetSize().GetElement(1);
    int d = m_ReferenceImage->GetLargestPossibleRegion().GetSize().GetElement(2);

    typedef ImageRegionConstIterator< InputImageType > InImageIteratorType;
    InImageIteratorType rgbaIt(rgbaImage, region);
    rgbaIt.GoToBegin();
    while(!rgbaIt.IsAtEnd()){

      InPixelType x = rgbaIt.Get();
      ++rgbaIt;

      itk::Point<float, 3> vertex;
      itk::Index<3> index = rgbaIt.GetIndex();

      rgbaImage->TransformIndexToPhysicalPoint(index, vertex);
      outputImage->TransformPhysicalPointToIndex(vertex, index);

      itk::ContinuousIndex<float, 3> contIndex;
      outputImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

      float frac_x = contIndex[0] - index[0];
      float frac_y = contIndex[1] - index[1];
      float frac_z = contIndex[2] - index[2];
      int px = index[0];
      if (frac_x<0)
      {
        px -= 1;
        frac_x += 1;
      }
      int py = index[1];
      if (frac_y<0)
      {
        py -= 1;
        frac_y += 1;
      }
      int pz = index[2];
      if (frac_z<0)
      {
        pz -= 1;
        frac_z += 1;
      }
      frac_x = 1-frac_x;
      frac_y = 1-frac_y;
      frac_z = 1-frac_z;

      // int coordinates inside image?
      if (px < 0 || px >= w-1)
        continue;
      if (py < 0 || py >= h-1)
        continue;
      if (pz < 0 || pz >= d-1)
        continue;

      OutPixelType out;
      switch (m_Channel)
      {
      case RED:
        out = (float)x.GetRed()/255;
        break;
      case GREEN:
        out = (float)x.GetGreen()/255;
        break;
      case BLUE:
        out = (float)x.GetBlue()/255;
        break;
      case ALPHA:
        out = (float)x.GetAlpha()/255;
      }

      outImageBufferPointer[( px   + w*(py  + h*pz  ))] += out*(  frac_x)*(  frac_y)*(  frac_z);
      outImageBufferPointer[( px   + w*(py+1+ h*pz  ))] += out*(  frac_x)*(1-frac_y)*(  frac_z);
      outImageBufferPointer[( px   + w*(py  + h*pz+h))] += out*(  frac_x)*(  frac_y)*(1-frac_z);
      outImageBufferPointer[( px   + w*(py+1+ h*pz+h))] += out*(  frac_x)*(1-frac_y)*(1-frac_z);
      outImageBufferPointer[( px+1 + w*(py  + h*pz  ))] += out*(1-frac_x)*(  frac_y)*(  frac_z);
      outImageBufferPointer[( px+1 + w*(py  + h*pz+h))] += out*(1-frac_x)*(  frac_y)*(1-frac_z);
      outImageBufferPointer[( px+1 + w*(py+1+ h*pz  ))] += out*(1-frac_x)*(1-frac_y)*(  frac_z);
      outImageBufferPointer[( px+1 + w*(py+1+ h*pz+h))] += out*(1-frac_x)*(1-frac_y)*(1-frac_z);

      counterImageBufferPointer[( px   + w*(py  + h*pz  ))] += 1;
      counterImageBufferPointer[( px   + w*(py+1+ h*pz  ))] += 1;
      counterImageBufferPointer[( px   + w*(py  + h*pz+h))] += 1;
      counterImageBufferPointer[( px   + w*(py+1+ h*pz+h))] += 1;
      counterImageBufferPointer[( px+1 + w*(py  + h*pz  ))] += 1;
      counterImageBufferPointer[( px+1 + w*(py  + h*pz+h))] += 1;
      counterImageBufferPointer[( px+1 + w*(py+1+ h*pz  ))] += 1;
      counterImageBufferPointer[( px+1 + w*(py+1+ h*pz+h))] += 1;

    }

    typedef ImageRegionIterator< OutputImageType > OutImageIteratorType;
    OutImageIteratorType outIt(outputImage, outputImage->GetLargestPossibleRegion());
    outIt.GoToBegin();
    typedef ImageRegionConstIterator< itk::Image< short, 3 > > CountImageIteratorType;
    CountImageIteratorType counterIt(counterImage, counterImage->GetLargestPossibleRegion());
    counterIt.GoToBegin();

    while(!outIt.IsAtEnd() && !counterIt.IsAtEnd()){
      if (counterIt.Value()>0)
        outIt.Set(outIt.Value()/counterIt.Value());
      ++outIt;
      ++counterIt;
    }
  }
}
