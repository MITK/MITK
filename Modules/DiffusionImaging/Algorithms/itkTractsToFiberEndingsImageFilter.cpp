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
    if(&typeid(TOutputPixelType) != &typeid(unsigned char))
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

    int w = upsampledSize[0];
    int h = upsampledSize[1];
    int d = upsampledSize[2];


    unsigned char* accuout;
    accuout = reinterpret_cast<unsigned char*>(outImage->GetBufferPointer());
    for (int i=0; i<w*h*d; i++) accuout[i] = 0;

    typedef mitk::FiberBundle::ContainerTractType   ContainerTractType;
    typedef mitk::FiberBundle::ContainerType        ContainerType;
    typedef mitk::FiberBundle::ContainerPointType   ContainerPointType;
    ContainerType::Pointer tractContainer = m_FiberBundle->GetTractContainer();

    for (int i=0; i<tractContainer->Size(); i++)
    {
      ContainerTractType::Pointer tract = tractContainer->GetElement(i);
      int tractsize = tract->Size();

      if (tractsize>1)
      {
        ContainerPointType start = tract->GetElement(0);
        ContainerPointType end = tract->GetElement(tractsize-1);

        start[0] = (start[0]+0.5) * m_UpsamplingFactor;
        start[1] = (start[1]+0.5) * m_UpsamplingFactor;
        start[2] = (start[2]+0.5) * m_UpsamplingFactor;

        // int coordinates inside image?
        int px = (int) (start[0]);
        if (px < 0 || px >= w)
          continue;
        int py = (int) (start[1]);
        if (py < 0 || py >= h)
          continue;
        int pz = (int) (start[2]);
        if (pz < 0 || pz >= d)
          continue;

        accuout[( px   + w*(py  + h*pz  ))] += 1;


        end[0] = (end[0]+0.5) * m_UpsamplingFactor;
        end[1] = (end[1]+0.5) * m_UpsamplingFactor;
        end[2] = (end[2]+0.5) * m_UpsamplingFactor;

        // int coordinates inside image?
        px = (int) (end[0]);
        if (px < 0 || px >= w)
          continue;
        py = (int) (end[1]);
        if (py < 0 || py >= h)
          continue;
        pz = (int) (end[2]);
        if (pz < 0 || pz >= d)
          continue;

        accuout[( px   + w*(py  + h*pz  ))] += 1;
      }
    }

    MITK_INFO << "2D fiber endings image generated";
  }

}
