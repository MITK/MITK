#include "itkTractDensityImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

// misc
#include <math.h>

namespace itk{

  template< class OutputImageType >
  TractDensityImageFilter< OutputImageType >::TractDensityImageFilter()
    : m_BinaryOutput(false)
    , m_InvertImage(false)
    , m_UpsamplingFactor(1)
    , m_InputImage(NULL)
    , m_UseImageGeometry(false)
  {

  }

  template< class OutputImageType >
  TractDensityImageFilter< OutputImageType >::~TractDensityImageFilter()
  {
  }

  template< class OutputImageType >
  itk::Point<float, 3> TractDensityImageFilter< OutputImageType >::GetItkPoint(double point[3])
  {
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
  }

  template< class OutputImageType >
  void TractDensityImageFilter< OutputImageType >::GenerateData()
  {
    // generate upsampled image
    mitk::Geometry3D::Pointer geometry = m_FiberBundle->GetGeometry();
    typename OutputImageType::Pointer outImage = this->GetOutput();

    // calculate new image parameters
    mitk::Vector3D newSpacing;
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
    float minSpacing = 1;
    if(newSpacing[0]<newSpacing[1] && newSpacing[0]<newSpacing[2])
        minSpacing = newSpacing[0];
    else if (newSpacing[1] < newSpacing[2])
        minSpacing = newSpacing[1];
    else
        minSpacing = newSpacing[2];

    m_FiberBundle = m_FiberBundle->GetDeepCopy();
    m_FiberBundle->ResampleFibers(minSpacing/10);

    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    int numFibers = m_FiberBundle->GetNumFibers();
    for( int i=0; i<numFibers; i++ )
    {
      vtkIdType   numPoints(0);
      vtkIdType*  points(NULL);
      vLines->GetNextCell ( numPoints, points );

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

        if (m_BinaryOutput)
        {
          outImageBufferPointer[( px   + w*(py  + h*pz  ))] = 1;
          outImageBufferPointer[( px   + w*(py+1+ h*pz  ))] = 1;
          outImageBufferPointer[( px   + w*(py  + h*pz+h))] = 1;
          outImageBufferPointer[( px   + w*(py+1+ h*pz+h))] = 1;
          outImageBufferPointer[( px+1 + w*(py  + h*pz  ))] = 1;
          outImageBufferPointer[( px+1 + w*(py  + h*pz+h))] = 1;
          outImageBufferPointer[( px+1 + w*(py+1+ h*pz  ))] = 1;
          outImageBufferPointer[( px+1 + w*(py+1+ h*pz+h))] = 1;
        }
        else
        {
          outImageBufferPointer[( px   + w*(py  + h*pz  ))] += (  frac_x)*(  frac_y)*(  frac_z);
          outImageBufferPointer[( px   + w*(py+1+ h*pz  ))] += (  frac_x)*(1-frac_y)*(  frac_z);
          outImageBufferPointer[( px   + w*(py  + h*pz+h))] += (  frac_x)*(  frac_y)*(1-frac_z);
          outImageBufferPointer[( px   + w*(py+1+ h*pz+h))] += (  frac_x)*(1-frac_y)*(1-frac_z);
          outImageBufferPointer[( px+1 + w*(py  + h*pz  ))] += (1-frac_x)*(  frac_y)*(  frac_z);
          outImageBufferPointer[( px+1 + w*(py  + h*pz+h))] += (1-frac_x)*(  frac_y)*(1-frac_z);
          outImageBufferPointer[( px+1 + w*(py+1+ h*pz  ))] += (1-frac_x)*(1-frac_y)*(  frac_z);
          outImageBufferPointer[( px+1 + w*(py+1+ h*pz+h))] += (1-frac_x)*(1-frac_y)*(1-frac_z);
        }
      }
    }
    if (!m_BinaryOutput)
    {
      OutPixelType max = 0;
      for (int i=0; i<w*h*d; i++)
        if (max < outImageBufferPointer[i])
          max = outImageBufferPointer[i];
      if (max>0)
        for (int i=0; i<w*h*d; i++)
          outImageBufferPointer[i] /= max;
    }
    if (m_InvertImage)
      for (int i=0; i<w*h*d; i++)
        outImageBufferPointer[i] = 1-outImageBufferPointer[i];
  }
}
