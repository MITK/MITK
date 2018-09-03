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
#include "itkTractsToRgbaImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

// misc
#include <math.h>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

namespace itk{

template< class OutputImageType >
TractsToRgbaImageFilter< OutputImageType >::TractsToRgbaImageFilter()
  : m_UpsamplingFactor(1)
  , m_UseImageGeometry(false)
  , m_InputImage(nullptr)
{

}

template< class OutputImageType >
TractsToRgbaImageFilter< OutputImageType >::~TractsToRgbaImageFilter()
{
}

template< class OutputImageType >
void TractsToRgbaImageFilter< OutputImageType >::GenerateData()
{
  if(&typeid(OutPixelType) != &typeid(itk::RGBAPixel<unsigned char>))
    return;

  // generate upsampled image
  mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();
  typename OutputImageType::Pointer outImage = this->GetOutput();

  // calculate new image parameters
  itk::Vector<double,3> newSpacing;
  mitk::Point3D newOrigin;
  itk::Matrix<double, 3, 3> newDirection;
  ImageRegion<3> upsampledRegion;
  if (m_UseImageGeometry && !m_InputImage.IsNull())
  {
    newSpacing = m_InputImage->GetSpacing()/m_UpsamplingFactor;
    upsampledRegion = m_InputImage->GetLargestPossibleRegion();
    newOrigin = m_InputImage->GetOrigin();
    typename OutputImageType::RegionType::SizeType size = upsampledRegion.GetSize();
    size[0] *= m_UpsamplingFactor;
    size[1] *= m_UpsamplingFactor;
    size[2] *= m_UpsamplingFactor;
    upsampledRegion.SetSize(size);
    newDirection = m_InputImage->GetDirection();
  }
  else
  {
    newSpacing = geometry->GetSpacing()/m_UpsamplingFactor;
    newOrigin = geometry->GetOrigin();
    mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();
    newOrigin[0] += bounds.GetElement(0);
    newOrigin[1] += bounds.GetElement(2);
    newOrigin[2] += bounds.GetElement(4);

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        newDirection[j][i] = geometry->GetMatrixColumn(i)[j];
    upsampledRegion.SetSize(0, geometry->GetExtent(0)*m_UpsamplingFactor);
    upsampledRegion.SetSize(1, geometry->GetExtent(1)*m_UpsamplingFactor);
    upsampledRegion.SetSize(2, geometry->GetExtent(2)*m_UpsamplingFactor);
  }
  typename OutputImageType::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();

  // apply new image parameters
  outImage->SetSpacing( newSpacing );
  outImage->SetOrigin( newOrigin );
  outImage->SetDirection( newDirection );
  outImage->SetRegions( upsampledRegion );
  outImage->Allocate();
  outImage->FillBuffer(0.0);

  itk::Image< itk::RGBAPixel<double>, 3 >::Pointer double_out = itk::Image< itk::RGBAPixel<double>, 3 >::New();
  double_out->SetSpacing( newSpacing );
  double_out->SetOrigin( newOrigin );
  double_out->SetDirection( newDirection );
  double_out->SetRegions( upsampledRegion );
  double_out->Allocate();
  double_out->FillBuffer(0.0);

  vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
  int numFibers = m_FiberBundle->GetNumFibers();

  float scale = 100 * pow((float)m_UpsamplingFactor,3);
  boost::progress_display disp(numFibers);
  for( int i=0; i<numFibers; ++i )
  {
    ++disp;
    vtkCell* cell = fiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    for( int j=0; j<numPoints-1; ++j)
    {
      Point<float, 3> startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
      Index<3> startIndex;
      ContinuousIndex<float, 3> startIndexCont;
      double_out->TransformPhysicalPointToIndex(startVertex, startIndex);
      double_out->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

      Point<float, 3> endVertex = mitk::imv::GetItkPoint(points->GetPoint(j + 1));
      Index<3> endIndex;
      ContinuousIndex<float, 3> endIndexCont;
      double_out->TransformPhysicalPointToIndex(endVertex, endIndex);
      double_out->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

      Vector<float, 3> dir;
      dir[0] = fabs(endVertex[0]-startVertex[0]);
      dir[1] = fabs(endVertex[1]-startVertex[1]);
      dir[2] = fabs(endVertex[2]-startVertex[2]);
      dir.Normalize();

      std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(newSpacing, startIndex, endIndex, startIndexCont, endIndexCont);
      for (std::pair< itk::Index<3>, double > segment : segments)
      {
        if (!double_out->GetLargestPossibleRegion().IsInside(segment.first))
          continue;

        itk::RGBAPixel<double> pix = double_out->GetPixel(segment.first);
        pix[0] += dir[0] * scale;
        pix[1] += dir[1] * scale;
        pix[2] += dir[2] * scale;
        pix[3] += segment.second * scale;

        double_out->SetPixel(segment.first, pix);
      }
    }

    float maxRgb = 0.000000001;
    float maxInt = 0.000000001;
    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];
    int numPix = w*h*d*4;

    double* buffer = (double*)double_out->GetBufferPointer();
    // calc maxima
    for(int i=0; i<numPix; i++)
    {
      if((i-3)%4 != 0)
      {
        if(buffer[i] > maxRgb)
          maxRgb = buffer[i];
      }
      else
      {
        if(buffer[i] > maxInt)
          maxInt = buffer[i];
      }
    }

    // write output, normalized uchar 0..255
    unsigned char* outImageBufferPointer = (unsigned char*)outImage->GetBufferPointer();
    for(int i=0; i<numPix; i++)
    {
      if((i-3)%4 != 0)
        outImageBufferPointer[i] = (unsigned char) (255.0 * buffer[i] / maxRgb);
      else
        outImageBufferPointer[i] = (unsigned char) (255.0 * buffer[i] / maxInt);
    }
  }
}
}
