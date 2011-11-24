#include "itkTractDensityImageFilter.h"

#include <itkBSplineUpsampleImageFilter.h>

#define __CEIL_UCHAR__(val) (val) =       \
( (val) < 0 ) ? ( 0 ) : ( ( (val)>255 ) ? ( 255 ) : ( (val) ) );

namespace itk{

  template< class TInputImage, class TOutputPixelType >
  TractDensityImageFilter< TInputImage, TOutputPixelType >
      ::TractDensityImageFilter():
    m_BinaryEnvelope(false)
  {
    this->SetNumberOfRequiredInputs(0);
  }

  template< class TInputImage, class TOutputPixelType >
  TractDensityImageFilter< TInputImage, TOutputPixelType >
      ::~TractDensityImageFilter()
  {
  }


  template< class TInputImage, class TOutputPixelType >
  itk::Point<float, 3> TractDensityImageFilter< TInputImage, TOutputPixelType >
      ::GetItkPoint(double point[3])
  {
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
  }

  template< class TInputImage, class TOutputPixelType >
  void TractDensityImageFilter< TInputImage, TOutputPixelType >
      ::GenerateData()
  {
    // check for correct pixel type
    if(&typeid(TOutputPixelType) != &typeid(unsigned char))
    {
      MITK_INFO << "Only 'unsigned char' supported as OutputPixelType";
      return;
    }

    // generate upsampled image
    mitk::Geometry3D::Pointer geometry = m_FiberBundle->GetGeometry();
    typename OutputImageType::Pointer outImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    // calculate new image parameters
    mitk::Vector3D newSpacing = geometry->GetSpacing()/m_UpsamplingFactor;
    mitk::Point3D newOrigin = geometry->GetOrigin();
    itk::Matrix<double, 3, 3> newDirection;
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        newDirection[j][i] = geometry->GetMatrixColumn(i)[j]/newSpacing[i];
    ImageRegion<3> upsampledRegion;
    upsampledRegion.SetSize(0, geometry->GetExtent(0)*m_UpsamplingFactor);
    upsampledRegion.SetSize(1, geometry->GetExtent(1)*m_UpsamplingFactor);
    upsampledRegion.SetSize(2, geometry->GetExtent(2)*m_UpsamplingFactor);
    typename InputImageType::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();

    // aply new image parameters
    outImage->SetSpacing( newSpacing );
    outImage->SetOrigin( newOrigin );
    outImage->SetDirection( newDirection );
    outImage->SetRegions( upsampledRegion );
    outImage->Allocate();

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];

    // set/initialize output
    float* accu;
    unsigned char* outImageBufferPointer = reinterpret_cast<unsigned char*>(outImage->GetBufferPointer());
    if(isRgba)
    {
      accu = new float[w*h*d*4];
      for (int i=0; i<w*h*d*4; i++)
        accu[i] = 0;
    }
    else
    {
      accu = new float[w*h*d];
      for (int i=0; i<w*h*d; i++)
        accu[i] = 0;
    }

    // resample fiber bundle
    float minSpacing = 1;
    if(newSpacing[0]<newSpacing[1] && newSpacing[0]<newSpacing[2])
        minSpacing = newSpacing[0];
    else if (newSpacing[1] < newSpacing[2])
        minSpacing = newSpacing[1];
    else
        minSpacing = newSpacing[2];

    m_FiberBundle = m_FiberBundle->GetDeepCopy();
    m_FiberBundle->ResampleFibers(minSpacing);

    // for each tract
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
        itk::Index index;

        if ( outImage->TransformPhysicalPointToIndex(vertex, index) )
        {
          // float fraction of coordinates
          float frac_x = vertex[0] - px;
          float frac_y = vertex[1] - py;
          float frac_z = vertex[2] - pz;

        }

        float scale = 100 * pow((float)m_UpsamplingFactor,3);

        if (m_BinaryEnvelope)
        {
          accu[( px   + w*(py  + h*pz  ))] = 1;
          accu[( px   + w*(py+1+ h*pz  ))] = 1;
          accu[( px   + w*(py  + h*pz+h))] = 1;
          accu[( px   + w*(py+1+ h*pz+h))] = 1;
          accu[( px+1 + w*(py  + h*pz  ))] = 1;
          accu[( px+1 + w*(py  + h*pz+h))] = 1;
          accu[( px+1 + w*(py+1+ h*pz  ))] = 1;
          accu[( px+1 + w*(py+1+ h*pz+h))] = 1;
        }
        else
        {
          accu[( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * intweight * scale;
          accu[( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * intweight * scale;
          accu[( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * intweight * scale;
          accu[( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * intweight * scale;
          accu[( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * intweight * scale;
          accu[( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * intweight * scale;
          accu[( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * intweight * scale;
          accu[( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * intweight * scale;
        }

      }
    }

    float maxRgb = 0.000000001;
    float maxInt = 0.000000001;
    int numPix;

    if(isRgba)
    {
      numPix = w*h*d*4;

      // calc maxima
      for(int i=0; i<numPix; i++)
      {
        if((i-3)%4 != 0)
        {
          if(accu[i] > maxRgb)
          {
            maxRgb = accu[i];
          }
        }
        else
        {
          if(accu[i] > maxInt)
          {
            maxInt = accu[i];
          }
        }
      }

      // write output, normalized uchar 0..255
      for(int i=0; i<numPix; i++)
      {
        if((i-3)%4 != 0)
        {
          outImageBufferPointer[i] = (unsigned char) (255.0 * accu[i] / maxRgb);
        }
        else
        {
          outImageBufferPointer[i] = (unsigned char) (255.0 * accu[i] / maxInt);
        }
      }
    }
    else if (m_BinaryEnvelope)
    {
      numPix = w*h*d;

      // write output, normalized uchar 0..255
      for(int i=0; i<numPix; i++)
      {
        if(m_InvertImage)
        {
          outImageBufferPointer[i] = (unsigned char) ((int)(accu[i]+1)%2);
        }
        else
        {
          outImageBufferPointer[i] = (unsigned char) accu[i];
        }
      }
    }
    else
    {
      numPix = w*h*d;

      // calc maxima
      for(int i=0; i<numPix; i++)
      {
        if(accu[i] > maxInt)
        {
          maxInt = accu[i];
        }
      }

      // write output, normalized uchar 0..255
      for(int i=0; i<numPix; i++)
      {
          outImageBufferPointer[i] = (unsigned char) (255.0 * accu[i] / maxInt);
      }
    }

    delete[] accu;
  }

}
