#ifndef __itkTractDensityImageFilter_h__
#define __itkTractDensityImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

template< class OutputImageType >
class TractDensityImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractDensityImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;

  itkNewMacro(Self);
  itkTypeMacro( TractDensityImageFilter, ImageSource );

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, unsigned int);
  itkGetMacro( UpsamplingFactor, unsigned int);

  /** Invert Image **/
  itkSetMacro( InvertImage, bool);
  itkGetMacro( InvertImage, bool);

  /** Binary Output **/
  itkSetMacro( BinaryOutput, bool);
  itkGetMacro( BinaryOutput, bool);

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool);
  itkGetMacro( UseImageGeometry, bool);

  itkSetMacro( FiberBundle, mitk::FiberBundleX::Pointer);
  itkSetMacro( InputImage, typename OutputImageType::Pointer);

  void GenerateData();

protected:

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractDensityImageFilter();
  virtual ~TractDensityImageFilter();

  typename OutputImageType::Pointer m_InputImage;
  mitk::FiberBundleX::Pointer m_FiberBundle;
  unsigned int m_UpsamplingFactor;
  bool m_InvertImage;
  bool m_BinaryOutput;
  bool m_UseImageGeometry;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractDensityImageFilter.cpp"
#endif

#endif // __itkTractDensityImageFilter_h__
