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
#include "itkTractsToFiberEndingsImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

namespace itk{

  template< class OutputImageType >
  TractsToFiberEndingsImageFilter< OutputImageType >::TractsToFiberEndingsImageFilter()
    : m_UpsamplingFactor(1)
    , m_InvertImage(false)
    , m_UseImageGeometry(false)
    , m_BinaryOutput(false)
    , m_InputImage(nullptr)
  {

  }

  template< class OutputImageType >
  TractsToFiberEndingsImageFilter< OutputImageType >::~TractsToFiberEndingsImageFilter()
  {
  }

  template< class OutputImageType >
  void TractsToFiberEndingsImageFilter< OutputImageType >::GenerateData()
  {
    // generate upsampled image
    mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();
    typename OutputImageType::Pointer outImage = this->GetOutput();

    // calculate new image parameters
    itk::Vector<double, 3> newSpacing;
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

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];

    // set/initialize output
    OutPixelType* outImageBufferPointer = (OutPixelType*)outImage->GetBufferPointer();
    for (int i=0; i<w*h*d; i++)
      outImageBufferPointer[i] = 0;

    // resample fiber bundle
    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();

    int numFibers = m_FiberBundle->GetNumFibers();
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        ++disp;
      vtkCell* cell = fiberPolyData->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      // fill output image
      if (numPoints>0)
      {
        itk::Point<float, 3> vertex = mitk::imv::GetItkPoint(points->GetPoint(0));
        itk::Index<3> index;
        outImage->TransformPhysicalPointToIndex(vertex, index);
        if (upsampledRegion.IsInside(index))
        {
          if (m_BinaryOutput)
          {
            outImage->SetPixel(index, 1);
          }
          else
          {
            outImage->SetPixel(index, outImage->GetPixel(index)+1);
          }
        }
      }

      if (numPoints>=2)
      {
        itk::Point<float, 3> vertex = mitk::imv::GetItkPoint(points->GetPoint(numPoints-1));
        itk::Index<3> index;
        outImage->TransformPhysicalPointToIndex(vertex, index);
        if (upsampledRegion.IsInside(index))
        {
          if (m_BinaryOutput)
          {
              outImage->SetPixel(index, 1);
          }
          else
          {
              outImage->SetPixel(index, outImage->GetPixel(index)+1);
          }
        }
      }
    }

    if (m_InvertImage)
      for (int i=0; i<w*h*d; i++)
        outImageBufferPointer[i] = 1-outImageBufferPointer[i];
  }
}
