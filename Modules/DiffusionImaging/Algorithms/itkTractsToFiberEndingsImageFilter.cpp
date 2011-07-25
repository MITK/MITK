#include "itkTractsToFiberEndingsImageFilter.h"

#include "itkBSplineUpsampleImageFilter.h"

#define __CEIL_UCHAR__(val) (val) =       \
( (val) < 0 ) ? ( 0 ) : ( ( (val)>255 ) ? ( 255 ) : ( (val) ) );

namespace itk{

  template< class TInputImage, class TOutputPixelType >
  TractsToFiberEndingsImageFilter< TInputImage, TOutputPixelType >
      ::TractsToFiberEndingsImageFilter()
  {
    this->SetNumberOfRequiredInputs(0);
  }

  template< class TInputImage, class TOutputPixelType >
  TractsToFiberEndingsImageFilter< TInputImage, TOutputPixelType >
      ::~TractsToFiberEndingsImageFilter()
  {
  }

  template< class TInputImage, class TOutputPixelType >
  void TractsToFiberEndingsImageFilter< TInputImage, TOutputPixelType >
      ::GenerateData()
  {
    MITK_INFO << "Generating 2D fiber endings image";
    bool isRgba = false;
    if(&typeid(TOutputPixelType) == &typeid(itk::RGBAPixel<unsigned char>))
    {
      isRgba = true;
    }
    else if(&typeid(TOutputPixelType) != &typeid(unsigned char))
    {
      MITK_INFO << "Only 'unsigned char' and 'itk::RGBAPixel<unsigned char> supported as OutputPixelType";
      return;
    }

    mitk::Geometry3D::Pointer geometry = m_FiberBundle->GetGeometry();

    typename OutputImageType::Pointer outImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    outImage->SetSpacing( geometry->GetSpacing()/m_UpsamplingFactor );   // Set the image spacing

    mitk::Point3D origin = geometry->GetOrigin();
    mitk::Point3D indexOrigin;
    geometry->WorldToIndex(origin, indexOrigin);
    indexOrigin[0] = indexOrigin[0] - .5 * (1.0-1.0/m_UpsamplingFactor);
    indexOrigin[1] = indexOrigin[1] - .5 * (1.0-1.0/m_UpsamplingFactor);
    indexOrigin[2] = indexOrigin[2] - .5 * (1.0-1.0/m_UpsamplingFactor);
    mitk::Point3D newOrigin;
    geometry->IndexToWorld(indexOrigin, newOrigin);

    outImage->SetOrigin( newOrigin );     // Set the image origin
    itk::Matrix<double, 3, 3> matrix;
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        matrix[j][i] = geometry->GetMatrixColumn(i)[j];
    outImage->SetDirection( matrix );  // Set the image direction

    float* bounds = m_FiberBundle->GetBounds();
    ImageRegion<3> upsampledRegion;
    upsampledRegion.SetSize(0, bounds[0]);
    upsampledRegion.SetSize(1, bounds[1]);
    upsampledRegion.SetSize(2, bounds[2]);

    typename InputImageType::RegionType::SizeType upsampledSize = upsampledRegion.GetSize();
    for (unsigned int n = 0; n < 3; n++)
    {
      upsampledSize[n] = upsampledSize[n] * m_UpsamplingFactor;
    }
    upsampledRegion.SetSize( upsampledSize );
    outImage->SetRegions( upsampledRegion );

    outImage->Allocate();
    //    itk::RGBAPixel<unsigned char> pix;
    //    pix.Set(0,0,0,0);
    //    outImage->FillBuffer(pix);

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];


    unsigned char* accuout;
    float* accu;

    accuout = reinterpret_cast<unsigned char*>(outImage->GetBufferPointer());

    if(isRgba)
    {
      accu = new float[w*h*d*4];
      for (int i=0; i<w*h*d*4; i++) accu[i] = 0;
    }
    else
    {
      accu = new float[w*h*d];
      for (int i=0; i<w*h*d; i++) accu[i] = 0;
    }

    // for each tract
    int numTracts = m_FiberBundle->GetNumTracts();
    for( int i=0; i<numTracts; i++ )
    {
      std::vector< itk::Point<float, 3> > vertices;

      // get fiber start point
      int numVertices = m_FiberBundle->GetNumPoints(i);
      itk::Point<float, 3> start = m_FiberBundle->GetPoint(i,0);
      vertices.push_back(start);

      // get fiber end point
      if(numVertices>1)
      {
        itk::Point<float, 3> end = m_FiberBundle->GetPoint(i,numVertices-1);
        vertices.push_back(end);
      }

      ////////////////////
      // fill output image

      // for each vertex
      for( int j=0; j<vertices.size(); j++)
      {
        itk::Point<float, 3> vertex = vertices.at(j);

        // scaling coordinates (index coords scale with upsampling)
        vertex[0] = (vertex[0]+0.5) * m_UpsamplingFactor;
        vertex[1] = (vertex[1]+0.5) * m_UpsamplingFactor;
        vertex[2] = (vertex[2]+0.5) * m_UpsamplingFactor;

        // int coordinates inside image?
        int px = (int) (vertex[0]);
        if (px < 0 || px >= w)
          continue;
        int py = (int) (vertex[1]);
        if (py < 0 || py >= h)
          continue;
        int pz = (int) (vertex[2]);
        if (pz < 0 || pz >= d)
          continue;

        // float fraction of coordinates
        float frac_x = vertex[0] - px;
        float frac_y = vertex[1] - py;
        float frac_z = vertex[2] - pz;

        float scale = 100 * pow((float)m_UpsamplingFactor,3);

        if(isRgba)
        {
          // add to r-channel in output image
          accu[0+4*( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * scale;
          accu[0+4*( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * scale;
          accu[0+4*( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * scale;
          accu[0+4*( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * scale;
          accu[0+4*( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * scale;
          accu[0+4*( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * scale;
          accu[0+4*( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * scale;
          accu[0+4*( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * scale;

          // add to a-channel in output image
          accu[3+4*( px   + w*(py  + h*pz  ))] = 1;
          accu[3+4*( px   + w*(py+1+ h*pz  ))] = 1;
          accu[3+4*( px   + w*(py  + h*pz+h))] = 1;
          accu[3+4*( px   + w*(py+1+ h*pz+h))] = 1;
          accu[3+4*( px+1 + w*(py  + h*pz  ))] = 1;
          accu[3+4*( px+1 + w*(py  + h*pz+h))] = 1;
          accu[3+4*( px+1 + w*(py+1+ h*pz  ))] = 1;
          accu[3+4*( px+1 + w*(py+1+ h*pz+h))] = 1;
        }
        else
        {
          accu[( px   + w*(py  + h*pz  ))] += (1-frac_x)*(1-frac_y)*(1-frac_z) * scale;
          accu[( px   + w*(py+1+ h*pz  ))] += (1-frac_x)*(  frac_y)*(1-frac_z) * scale;
          accu[( px   + w*(py  + h*pz+h))] += (1-frac_x)*(1-frac_y)*(  frac_z) * scale;
          accu[( px   + w*(py+1+ h*pz+h))] += (1-frac_x)*(  frac_y)*(  frac_z) * scale;
          accu[( px+1 + w*(py  + h*pz  ))] += (  frac_x)*(1-frac_y)*(1-frac_z) * scale;
          accu[( px+1 + w*(py  + h*pz+h))] += (  frac_x)*(1-frac_y)*(  frac_z) * scale;
          accu[( px+1 + w*(py+1+ h*pz  ))] += (  frac_x)*(  frac_y)*(1-frac_z) * scale;
          accu[( px+1 + w*(py+1+ h*pz+h))] += (  frac_x)*(  frac_y)*(  frac_z) * scale;
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
          accuout[i] = (unsigned char) (255.0 * accu[i] / maxRgb);
        }
        else
        {
          accuout[i] = (unsigned char) (255.0 * accu[i] / maxInt);
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
          accuout[i] = (unsigned char) (255.0 * accu[i] / maxInt);

      }
    }

    delete[] accu;
    MITK_INFO << "2D fiber endings image generated";
  }

}
