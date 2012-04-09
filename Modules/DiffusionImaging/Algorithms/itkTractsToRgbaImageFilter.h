#ifndef __itkTractsToRgbaImageFilter_h__
#define __itkTractsToRgbaImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

template< class OutputImageType >
class TractsToRgbaImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractsToRgbaImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;
  typedef itk::Image<unsigned char, 3> InputImageType;

  itkNewMacro(Self);
  itkTypeMacro( TractsToRgbaImageFilter, ImageSource );

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, unsigned int);
  itkGetMacro( UpsamplingFactor, unsigned int);

  /** Invert Image **/
  itkSetMacro( InvertImage, bool);
  itkGetMacro( InvertImage, bool);

  /** Binary Output **/
  itkSetMacro( BinaryOutput, bool);
  itkGetMacro( BinaryOutput, bool);

  itkSetMacro( FiberBundle, mitk::FiberBundleX::Pointer);
  itkSetMacro( InputImage, typename InputImageType::Pointer);

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool);
  itkGetMacro( UseImageGeometry, bool);


  void GenerateData();

protected:

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractsToRgbaImageFilter();
  virtual ~TractsToRgbaImageFilter();

  mitk::FiberBundleX::Pointer m_FiberBundle;
  unsigned int m_UpsamplingFactor;
  bool m_InvertImage;
  bool m_BinaryOutput;
  bool m_UseImageGeometry;
  typename InputImageType::Pointer m_InputImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToRgbaImageFilter.cpp"
#endif

#endif // __itkTractsToRgbaImageFilter_h__
