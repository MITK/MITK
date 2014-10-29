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

namespace itk{

  template< class OutputImageType >
  TractsToRgbaImageFilter< OutputImageType >::TractsToRgbaImageFilter()
    : m_UpsamplingFactor(1)
    , m_InputImage(NULL)
    , m_UseImageGeometry(false)
  {

  }

  template< class OutputImageType >
  TractsToRgbaImageFilter< OutputImageType >::~TractsToRgbaImageFilter()
  {
  }

  template< class OutputImageType >
  itk::Point<float, 3> TractsToRgbaImageFilter< OutputImageType >::GetItkPoint(double point[3])
  {
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
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

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];

    // set/initialize output
    unsigned char* outImageBufferPointer = (unsigned char*)outImage->GetBufferPointer();
    float* buffer = new float[w*h*d*4];
    for (int i=0; i<w*h*d*4; i++)
      buffer[i] = 0;

    // resample fiber bundle
    float minSpacing = 1;
    if(newSpacing[0]<newSpacing[1] && newSpacing[0]<newSpacing[2])
        minSpacing = newSpacing[0];
    else if (newSpacing[1] < newSpacing[2])
        minSpacing = newSpacing[1];
    else
        minSpacing = newSpacing[2];

    m_FiberBundle = m_FiberBundle->GetDeepCopy();
    m_FiberBundle->ResampleSpline(minSpacing);

    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    int numFibers = m_FiberBundle->GetNumFibers();
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
      ++disp;
      vtkIdType   numPoints(0);
      vtkIdType*  points(NULL);
      vLines->GetNextCell ( numPoints, points );

      // calc directions (which are used as weights)
      std::list< itk::Point<float, 3> > rgbweights;
      std::list<float> intensities;

      for( int j=0; j<numPoints-1; j++)
      {

        itk::Point<float, 3> vertex = GetItkPoint(fiberPolyData->GetPoint(points[j]));
        itk::Point<float, 3> vertexPost = GetItkPoint(fiberPolyData->GetPoint(points[j+1]));

        itk::Point<float, 3> dir;
        dir[0] = fabs((vertexPost[0] - vertex[0]) * outImage->GetSpacing()[0]);
        dir[1] = fabs((vertexPost[1] - vertex[1]) * outImage->GetSpacing()[1]);
        dir[2] = fabs((vertexPost[2] - vertex[2]) * outImage->GetSpacing()[2]);

        rgbweights.push_back(dir);

        float intensity = sqrt(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);
        intensities.push_back(intensity);

        // last point gets same as previous one
        if(j==numPoints-2)
        {
          rgbweights.push_back(dir);
          intensities.push_back(intensity);
        }
      }

      // fill output image
      for( int j=0; j<numPoints; j++)
      {
        itk::Point<float, 3> vertex = GetItkPoint(fiberPolyData->GetPoint(points[j]));
        itk::Index<3> index;
        itk::ContinuousIndex<float, 3> contIndex;
        outImage->TransformPhysicalPointToIndex(vertex, index);
        outImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

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

        // int coordinates inside image?
        if (px < 0 || px >= w-1)
          continue;
        if (py < 0 || py >= h-1)
          continue;
        if (pz < 0 || pz >= d-1)
          continue;

        float scale = 100 * pow((float)m_UpsamplingFactor,3);
        itk::Point<float, 3> rgbweight = rgbweights.front();
        rgbweights.pop_front();
        float intweight = intensities.front();
        intensities.pop_front();

        // add to r-channel in output image
        buffer[0+4*( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * rgbweight[0] * scale;
        buffer[0+4*( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * rgbweight[0] * scale;

        // add to g-channel in output image
        buffer[1+4*( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * rgbweight[1] * scale;
        buffer[1+4*( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * rgbweight[1] * scale;

        // add to b-channel in output image
        buffer[2+4*( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * rgbweight[2] * scale;
        buffer[2+4*( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * rgbweight[2] * scale;

        // add to a-channel in output image
        buffer[3+4*( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * intweight * scale;
        buffer[3+4*( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * intweight * scale;
        buffer[3+4*( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * intweight * scale;
        buffer[3+4*( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * intweight * scale;
        buffer[3+4*( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * intweight * scale;
        buffer[3+4*( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * intweight * scale;
        buffer[3+4*( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * intweight * scale;
        buffer[3+4*( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * intweight * scale;
      }
    }
    float maxRgb = 0.000000001;
    float maxInt = 0.000000001;
    int numPix;

    numPix = w*h*d*4;
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
    for(int i=0; i<numPix; i++)
    {
      if((i-3)%4 != 0)
        outImageBufferPointer[i] = (unsigned char) (255.0 * buffer[i] / maxRgb);
      else
        outImageBufferPointer[i] = (unsigned char) (255.0 * buffer[i] / maxInt);
    }
  }
}
