#ifndef __itkTractsToFiberEndingsImageFilter_h__
#define __itkTractsToFiberEndingsImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

template< class OutputImageType >
class TractsToFiberEndingsImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractsToFiberEndingsImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;

  itkNewMacro(Self);
  itkTypeMacro( TractsToFiberEndingsImageFilter, ImageSource );

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, unsigned int);
  itkGetMacro( UpsamplingFactor, unsigned int);

  /** Invert Image **/
  itkSetMacro( InvertImage, bool);
  itkGetMacro( InvertImage, bool);

  itkSetMacro( FiberBundle, mitk::FiberBundleX::Pointer);
  itkSetMacro( InputImage, typename OutputImageType::Pointer);

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool);
  itkGetMacro( UseImageGeometry, bool);

  void GenerateData();

protected:

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractsToFiberEndingsImageFilter();
  virtual ~TractsToFiberEndingsImageFilter();

  mitk::FiberBundleX::Pointer m_FiberBundle;
  unsigned int m_UpsamplingFactor;
  bool m_InvertImage;
  bool m_UseImageGeometry;
  typename OutputImageType::Pointer m_InputImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToFiberEndingsImageFilter.cpp"
#endif

#endif // __itkTractsToFiberEndingsImageFilter_h__
