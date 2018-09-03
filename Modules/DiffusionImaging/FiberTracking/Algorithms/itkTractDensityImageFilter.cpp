/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Coindex[1]right (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "itkTractDensityImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCell.h>

// misc
#include <cmath>
#include <boost/progress.hpp>
#include <vtkBox.h>
#include <mitkDiffusionFunctionCollection.h>

namespace itk{

template< class OutputImageType >
TractDensityImageFilter< OutputImageType >::TractDensityImageFilter()
  : m_UpsamplingFactor(1)
  , m_InvertImage(false)
  , m_BinaryOutput(false)
  , m_UseImageGeometry(false)
  , m_OutputAbsoluteValues(false)
  , m_MaxDensity(0)
  , m_NumCoveredVoxels(0)
{
}

template< class OutputImageType >
TractDensityImageFilter< OutputImageType >::~TractDensityImageFilter()
{
}

template< class OutputImageType >
void TractDensityImageFilter< OutputImageType >::GenerateData()
{
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
    MITK_INFO << "TractDensityImageFilter: using image geometry";
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
    MITK_INFO << "TractDensityImageFilter: using fiber bundle geometry";
    newSpacing = geometry->GetSpacing()/m_UpsamplingFactor;
    newOrigin = geometry->GetOrigin();
    mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();

    // we retrieve the origin from a vtk-polydata (corner-based) and hance have to translate it to an image geometry
    // i.e. center-based
    newOrigin[0] += bounds.GetElement(0) + 0.5 * newSpacing[0];
    newOrigin[1] += bounds.GetElement(2) + 0.5 * newSpacing[1];
    newOrigin[2] += bounds.GetElement(4) + 0.5 * newSpacing[2];

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        newDirection[j][i] = geometry->GetMatrixColumn(i)[j];
    upsampledRegion.SetSize(0, ceil( geometry->GetExtent(0)*m_UpsamplingFactor ) );
    upsampledRegion.SetSize(1, ceil( geometry->GetExtent(1)*m_UpsamplingFactor ) );
    upsampledRegion.SetSize(2, ceil( geometry->GetExtent(2)*m_UpsamplingFactor ) );
  }
  typename OutputImageType::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();

  // apply new image parameters
  outImage->SetSpacing( newSpacing );
  outImage->SetOrigin( newOrigin );
  outImage->SetDirection( newDirection );
  outImage->SetLargestPossibleRegion( upsampledRegion );
  outImage->SetBufferedRegion( upsampledRegion );
  outImage->SetRequestedRegion( upsampledRegion );
  outImage->Allocate();
  outImage->FillBuffer(0.0);

  int w = upsampledSize[0];
  int h = upsampledSize[1];
  int d = upsampledSize[2];

  // set/initialize output
  OutPixelType* outImageBufferPointer = (OutPixelType*)outImage->GetBufferPointer();

  MITK_INFO << "TractDensityImageFilter: starting image generation";
  vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();

  int numFibers = m_FiberBundle->GetNumFibers();
  boost::progress_display disp(numFibers);
  for( int i=0; i<numFibers; i++ )
  {
    ++disp;
    vtkCell* cell = fiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    float weight = m_FiberBundle->GetFiberWeight(i);

    // fill output image
    for( int j=0; j<numPoints-1; j++)
    {
      itk::Point<float, 3> startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
      itk::Index<3> startIndex;
      itk::ContinuousIndex<float, 3> startIndexCont;
      outImage->TransformPhysicalPointToIndex(startVertex, startIndex);
      outImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

      itk::Point<float, 3> endVertex = mitk::imv::GetItkPoint(points->GetPoint(j + 1));
      itk::Index<3> endIndex;
      itk::ContinuousIndex<float, 3> endIndexCont;
      outImage->TransformPhysicalPointToIndex(endVertex, endIndex);
      outImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

      std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(newSpacing, startIndex, endIndex, startIndexCont, endIndexCont);
      for (std::pair< itk::Index<3>, double > segment : segments)
      {
        if (!outImage->GetLargestPossibleRegion().IsInside(segment.first))
          continue;
        if (outImage->GetPixel(segment.first)==0)
          m_NumCoveredVoxels++;

        if (m_BinaryOutput)
          outImage->SetPixel(segment.first, 1);
        else
          outImage->SetPixel(segment.first, outImage->GetPixel(segment.first)+segment.second * weight);
      }
    }
  }

  m_MaxDensity = 0;
  for (int i=0; i<w*h*d; i++)
    if (m_MaxDensity < outImageBufferPointer[i])
      m_MaxDensity = outImageBufferPointer[i];
  if (!m_OutputAbsoluteValues && !m_BinaryOutput)
  {
    MITK_INFO << "TractDensityImageFilter: max-normalizing output image";
    if (m_MaxDensity>0)
      for (int i=0; i<w*h*d; i++)
      {
        outImageBufferPointer[i] /= m_MaxDensity;
      }
  }
  if (m_InvertImage)
  {
    MITK_INFO << "TractDensityImageFilter: inverting image";
    for (int i=0; i<w*h*d; i++)
      outImageBufferPointer[i] = 1-outImageBufferPointer[i];
  }
  MITK_INFO << "TractDensityImageFilter: finished processing";
}
}
